#!/usr/bin/env python3
"""Benchmark the step function of any set of versions across board sizes.

Each version is a git ref (or WORKTREE for the uncommitted main.c). Its
main.c is pulled out of git as-is and built against bench/adapter.c, so old
versions are measured exactly as they were written.

  bench/run.py                          # every version in bench/versions.txt
  bench/run.py --only WORKTREE --append # re-run one version, keep the rest
  bench/run.py --summarize              # rebuild summary.csv from raw.csv

Outputs (bench/results/):
  raw.csv      one row per timed sample
  summary.csv  per version x size: min/p50/p95/p99/max/mean/stddev, ns per
               cell, Mcells/s, speedup vs the first version, correctness
"""
import argparse
import csv
import math
import os
import shutil
import subprocess
import sys
from pathlib import Path

BENCH = Path(__file__).resolve().parent
REPO = BENCH.parent
RESULTS = BENCH / "results"
BUILD = BENCH / "build"

RAW_FIELDS = ["label", "ref", "date", "size", "cells", "batch", "sample", "ns_per_step"]
RUN_FIELDS = ["label", "ref", "date", "size", "verify", "first_bad_gen", "status"]
SUMMARY_FIELDS = [
    "order", "label", "ref", "date", "size", "cells", "samples", "batch", "correct",
    "min_ns", "p50_ns", "p95_ns", "p99_ns", "max_ns", "mean_ns", "stddev_ns",
    "ns_per_cell_p50", "mcells_per_s_p50", "speedup_p50_vs_first", "speedup_mean_vs_first",
]


def git(*args):
    return subprocess.run(["git", "-C", str(REPO), *args], check=True,
                          capture_output=True, text=True).stdout


def load_versions(path):
    versions = []
    for line in Path(path).read_text().splitlines():
        line = line.split("#", 1)[0].strip()
        if line:
            ref, label = (part.strip() for part in line.split("|", 1))
            versions.append((ref, label))
    return versions


def version_source(ref):
    if ref == "WORKTREE":
        return (REPO / "main.c").read_text(), "worktree"
    return git("show", f"{ref}:main.c"), git("log", "-1", "--format=%cs", ref).strip()


def build(ref, label, src, size, cflags):
    out_dir = BUILD / label.replace(" ", "_").replace("/", "_")
    out_dir.mkdir(parents=True, exist_ok=True)
    (out_dir / "main.c").write_text(src)
    for name in ("ruleset.c", "ruleset.h"):
        shutil.copy(BENCH / "rules" / name, out_dir / name)

    if size % 8:
        raise ValueError("sizes must be multiples of 8 (one block)")
    if "RESOLUTION" in src:            # May 2024: square boards, one macro
        size_macros = [f"-DRESOLUTION={size // 8}"]
    else:                              # WIDTH in 8-cell blocks, HEIGHT in 8-row bands
        size_macros = [f"-DWIDTH={size // 8}", f"-DHEIGHT={size // 8}"]
    if "uint64_t" in src:
        size_macros.append("-DBENCH_WORDS")

    exe = out_dir / f"driver_{size}"
    cmd = ["gcc", *cflags.split(), "-fgnu89-inline", "-fopenmp", "-w",
           f"-DBENCH_N={size}", f"-DTHREADS={os.cpu_count()}", *size_macros,
           f'-DVERSION_MAIN="{out_dir / "main.c"}"', f"-I{BENCH}", f"-I{out_dir}",
           str(BENCH / "adapter.c"), str(BENCH / "driver.c"), str(out_dir / "ruleset.c"),
           "-o", str(exe)]
    subprocess.run(cmd, check=True, capture_output=True, text=True)
    return exe


def run_one(exe, size, budget_ms, verify_gens, seed, timeout):
    proc = subprocess.run([str(exe), str(size), str(budget_ms), str(verify_gens), str(seed)],
                          capture_output=True, text=True, timeout=timeout)
    verify, first_bad, batch, samples = "skip", -1, 1, []
    for line in proc.stdout.splitlines():
        if line.startswith("#verify"):
            _, verify, first_bad = line.split()
        elif line.startswith("#batch"):
            batch = int(line.split()[1])
        else:
            parts = line.split(",")
            if len(parts) == 3:  # anything else is the version's own output
                samples.append((int(parts[0]), float(parts[2])))
    status = "ok" if proc.returncode == 0 else f"exit {proc.returncode}"
    if verify == "skip" and proc.returncode != 0:
        verify = "crash"
    return verify, int(first_bad), batch, samples, status


def read_csv(path):
    if not path.exists():
        return []
    with path.open() as f:
        return list(csv.DictReader(f))


def write_csv(path, fields, rows):
    with path.open("w", newline="") as f:
        w = csv.DictWriter(f, fieldnames=fields)
        w.writeheader()
        w.writerows(rows)


def percentile(sorted_vals, p):
    """Linear interpolation between closest ranks (numpy's default)."""
    if len(sorted_vals) == 1:
        return sorted_vals[0]
    k = (len(sorted_vals) - 1) * p / 100
    lo, hi = math.floor(k), math.ceil(k)
    return sorted_vals[lo] + (sorted_vals[hi] - sorted_vals[lo]) * (k - lo)


def summarize(order):
    raw = read_csv(RESULTS / "raw.csv")
    runs = {(r["label"], int(r["size"])): r for r in read_csv(RESULTS / "runs.csv")}
    groups = {}
    for r in raw:
        groups.setdefault((r["label"], int(r["size"])), []).append(r)

    labels = [l for l in order if any(k[0] == l for k in groups)]
    labels += sorted({k[0] for k in groups} - set(labels))
    rows = []
    for (label, size), rs in groups.items():
        vals = sorted(float(r["ns_per_step"]) for r in rs)
        mean = sum(vals) / len(vals)
        sd = math.sqrt(sum((v - mean) ** 2 for v in vals) / (len(vals) - 1)) if len(vals) > 1 else 0.0
        p50 = percentile(vals, 50)
        cells = size * size
        run = runs.get((label, size), {})
        rows.append({
            "order": labels.index(label), "label": label, "ref": rs[0]["ref"],
            "date": rs[0]["date"], "size": size, "cells": cells, "samples": len(vals),
            "batch": rs[0]["batch"], "correct": run.get("verify", "?"),
            "min_ns": vals[0], "p50_ns": p50, "p95_ns": percentile(vals, 95),
            "p99_ns": percentile(vals, 99), "max_ns": vals[-1], "mean_ns": mean,
            "stddev_ns": sd, "ns_per_cell_p50": p50 / cells, "mcells_per_s_p50": cells / p50 * 1e3,
        })

    first = {r["size"]: r for r in rows if r["order"] == 0}
    for r in rows:
        base = first.get(r["size"])
        r["speedup_p50_vs_first"] = base["p50_ns"] / r["p50_ns"] if base else ""
        r["speedup_mean_vs_first"] = base["mean_ns"] / r["mean_ns"] if base else ""
    rows.sort(key=lambda r: (r["order"], r["size"]))
    for r in rows:
        for k, v in r.items():
            if isinstance(v, float):
                r[k] = f"{v:.6g}"
    write_csv(RESULTS / "summary.csv", SUMMARY_FIELDS, rows)
    return rows


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--versions", default=str(BENCH / "versions.txt"))
    ap.add_argument("--only", nargs="*", help="refs or labels to run (default: all)")
    ap.add_argument("--sizes", default="8,16,32,64,128,256,512,1024,2048")
    ap.add_argument("--budget-ms", type=int, default=1500, help="timing budget per version x size")
    ap.add_argument("--verify-gens", type=int, default=16, help="generations checked vs reference")
    ap.add_argument("--cflags", default="-O3")
    ap.add_argument("--seed", type=int, default=12345)
    ap.add_argument("--timeout", type=int, default=600)
    ap.add_argument("--append", action="store_true", help="keep results of versions not re-run")
    ap.add_argument("--summarize", action="store_true", help="only rebuild summary.csv")
    args = ap.parse_args()

    versions = load_versions(args.versions)
    order = [label for _, label in versions]
    RESULTS.mkdir(exist_ok=True)
    if args.summarize:
        summarize(order)
        return

    todo = [v for v in versions if not args.only or v[0] in args.only or v[1] in args.only]
    sizes = [int(s) for s in args.sizes.split(",")]
    redo = {label for _, label in todo}
    raw = [r for r in read_csv(RESULTS / "raw.csv") if args.append and r["label"] not in redo]
    runs = [r for r in read_csv(RESULTS / "runs.csv") if args.append and r["label"] not in redo]

    for ref, label in todo:
        src, date = version_source(ref)
        for size in sizes:
            print(f"{label:<34} {size:>5}x{size:<5}", end=" ", flush=True)
            try:
                exe = build(ref, label, src, size, args.cflags)
                verify, bad, batch, samples, status = run_one(
                    exe, size, args.budget_ms, args.verify_gens, args.seed, args.timeout)
            except subprocess.CalledProcessError as e:
                verify, bad, batch, samples, status = "skip", -1, 0, [], "build failed"
                print(e.stderr[-400:], file=sys.stderr)
            except subprocess.TimeoutExpired:
                verify, bad, batch, samples, status = "skip", -1, 0, [], "timeout"
            runs.append({"label": label, "ref": ref, "date": date, "size": size,
                         "verify": verify, "first_bad_gen": bad, "status": status})
            for idx, ns in samples:
                raw.append({"label": label, "ref": ref, "date": date, "size": size,
                            "cells": size * size, "batch": batch, "sample": idx, "ns_per_step": ns})
            if samples:
                med = sorted(ns for _, ns in samples)[len(samples) // 2]
                print(f"{status:<6} verify={verify:<4} n={len(samples):<5} p50={med / 1e3:,.3f} us")
            else:
                print(status)
        write_csv(RESULTS / "raw.csv", RAW_FIELDS, raw)
        write_csv(RESULTS / "runs.csv", RUN_FIELDS, runs)

    summarize(order)


if __name__ == "__main__":
    main()

# Step benchmarks

Times the step function (`Iterate`) of any version of `main.c`, straight from
git history, on square boards from 8×8 to 2048×2048 cells.

```sh
python3 bench/run.py                           # every version in versions.txt
python3 bench/run.py --only WORKTREE --append  # your uncommitted main.c, keep the rest
python3 bench/run.py --sizes 64,256 --budget-ms 500
python3 bench/run.py --summarize               # rebuild summary.csv from raw.csv
```

Add a version by putting `<ref> | <label>` in `versions.txt`. The first entry
is the baseline for the speedup columns. `WORKTREE` means the current `main.c`.

## How it works

- `adapter.c` includes a version's `main.c` unmodified (its `main` renamed)
  and drives its `Iterate(G, Copy, Master)`. It handles both board layouts:
  8-cell `union block` rows and 64-bit `uint64_t` rows.
- `driver.c` first runs 16 generations against a naive B3/S23 reference and
  reports `ok` / `fail`, then times steps until the budget runs out. Small
  boards time a batch of steps per sample, so each sample takes at least 20 µs.
- `rules/` holds the B3/S23 `ruleset.c` every version is built with.
- Every version gets the same flags (`-O3` by default; change with `--cflags`).

## Output (`results/`)

| File | One row per | Columns |
|---|---|---|
| `raw.csv` | timed sample | label, ref, date, size, cells, batch, sample, ns_per_step |
| `runs.csv` | version × size | verify (`ok`/`fail`/`crash`), first_bad_gen, exit status |
| `summary.csv` | version × size | min, p50, p95, p99, max, mean, stddev (ns), ns/cell, Mcells/s, speedup vs first |

`raw.csv` is large, so it isn't committed. Re-run to regenerate it.

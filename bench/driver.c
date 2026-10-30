/* Times one version at one board size and checks it against a naive
 * B3/S23 torus reference.
 *
 * usage: driver <n> <budget_ms> <verify_gens> <seed>
 *
 * stdout:
 *   #verify ok|fail|skip <first bad generation or -1>
 *   #batch <steps per sample>
 *   <sample>,<steps>,<ns_per_step>       one line per sample
 *
 * Tiny boards step in nanoseconds, below what one clock read can resolve,
 * so a sample times a batch of steps sized to take >= ~20us; big boards
 * get batch 1, so every sample is a single real step. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "api.h"

#define MIN_SAMPLE_NS 20000.0
#define MIN_SAMPLES 30
#define MAX_SAMPLES 5000

static uint64_t rng;
static uint64_t next_rand(void) {
  rng ^= rng << 13;
  rng ^= rng >> 7;
  rng ^= rng << 17;
  return rng;
}

static double now_ns(void) {
  struct timespec t;
  clock_gettime(CLOCK_MONOTONIC_RAW, &t);
  return t.tv_sec * 1e9 + t.tv_nsec;
}

static void fill(void* b, unsigned char* ref, int n) {
  for (int r = 0; r < n; r++)
    for (int c = 0; c < n; c++) {
      int alive = (next_rand() >> 11) % 100 < 40;
      bench_set(b, r, c, alive);
      if (ref) ref[r * n + c] = alive;
    }
}

static void ref_step(const unsigned char* src, unsigned char* dst, int n) {
  for (int r = 0; r < n; r++)
    for (int c = 0; c < n; c++) {
      int count = 0;
      for (int dr = -1; dr <= 1; dr++)
        for (int dc = -1; dc <= 1; dc++)
          if (dr || dc) count += src[((r + dr + n) % n) * n + (c + dc + n) % n];
      int alive = src[r * n + c];
      dst[r * n + c] = count == 3 || (count == 2 && alive);
    }
}

/* Returns the first generation that differs from the reference, or -1. */
static int verify(int n, int gens) {
  void* b = bench_create(n);
  unsigned char* ref = malloc((size_t)n * n);
  unsigned char* tmp = malloc((size_t)n * n);
  fill(b, ref, n);
  for (int g = 1; g <= gens; g++) {
    bench_step(b);
    ref_step(ref, tmp, n);
    memcpy(ref, tmp, (size_t)n * n);
    for (int r = 0; r < n; r++)
      for (int c = 0; c < n; c++)
        if (bench_get(b, r, c) != ref[r * n + c]) return g;
  }
  return -1;
}

int main(int argc, char** argv) {
  if (argc < 5) {
    fprintf(stderr, "usage: %s <n> <budget_ms> <verify_gens> <seed>\n", argv[0]);
    return 2;
  }
  int n = atoi(argv[1]);
  double budget_ns = atof(argv[2]) * 1e6;
  int verify_gens = atoi(argv[3]);
  rng = strtoull(argv[4], NULL, 10) | 1;

  if (verify_gens > 0) {
    int bad = verify(n, verify_gens);
    printf("#verify %s %d\n", bad < 0 ? "ok" : "fail", bad);
  } else {
    printf("#verify skip -1\n");
  }
  fflush(stdout);

  void* b = bench_create(n);
  fill(b, NULL, n);

  /* Warm up and pick a batch size so one sample is >= MIN_SAMPLE_NS. */
  int batch = 1;
  for (;;) {
    double t0 = now_ns();
    for (int i = 0; i < batch; i++) bench_step(b);
    double dt = now_ns() - t0;
    if (dt >= MIN_SAMPLE_NS || batch >= (1 << 24)) break;
    batch *= 2;
  }
  printf("#batch %d\n", batch);

  double start = now_ns();
  for (int s = 0; s < MAX_SAMPLES; s++) {
    double t0 = now_ns();
    for (int i = 0; i < batch; i++) bench_step(b);
    double t1 = now_ns();
    printf("%d,%d,%.2f\n", s, batch, (t1 - t0) / batch);
    if (s + 1 >= MIN_SAMPLES && t1 - start >= budget_ns) break;
  }
  return 0;
}

/* Wraps any version of main.c without editing it: its main() is renamed and
 * the file is included whole, then its Iterate() is driven directly.
 *
 * Every version so far exposes Iterate(G, Copy, Master). Two board layouts:
 *   default      union block rows, 8 cells per byte (all historical versions)
 *   BENCH_WORDS  uint64_t rows, 64 cells per word (bitboard versions)
 *
 * VERSION_MAIN is the path of the main.c under test; BENCH_N is the board
 * side in cells. run.py sets the size macros each version expects. */
#include "api.h"

#define main bench_version_main
#include VERSION_MAIN
#undef main

#include <stdint.h>
#include <stdlib.h>

#ifdef BENCH_WORDS
typedef uint64_t bench_cell_t;
#define BENCH_PER 64
#else
typedef union block bench_cell_t;
#define BENCH_PER 8
#endif

#define BENCH_COLS ((BENCH_N + BENCH_PER - 1) / BENCH_PER)

typedef struct {
  bench_cell_t** g;
  bench_cell_t** copy;
  bench_cell_t*** master;
} bench_board;

static bench_cell_t** bench_alloc(void) {
  bench_cell_t** rows = calloc(BENCH_N, sizeof(bench_cell_t*));
  for (int i = 0; i < BENCH_N; i++) rows[i] = calloc(BENCH_COLS, sizeof(bench_cell_t));
  return rows;
}

void* bench_create(int n) {
  (void)n;
  bench_board* b = calloc(1, sizeof(bench_board));
  b->g = bench_alloc();
  b->copy = bench_alloc();
  b->master = calloc(1, sizeof(bench_cell_t**));
  *b->master = b->g;
  return b;
}

static unsigned char* bench_byte(void* p, int row, int col) {
  bench_board* b = p;
  return (unsigned char*)&(*b->master)[row][0] + col / 8;  /* little-endian */
}

void bench_set(void* p, int row, int col, int alive) {
  unsigned char* byte = bench_byte(p, row, col);
  *byte = (*byte & ~(1u << (col % 8))) | ((alive ? 1u : 0u) << (col % 8));
}

int bench_get(void* p, int row, int col) {
  return (*bench_byte(p, row, col) >> (col % 8)) & 1;
}

void bench_step(void* p) {
  bench_board* b = p;
  Iterate(b->g, b->copy, b->master);
}

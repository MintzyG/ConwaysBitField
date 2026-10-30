#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <locale.h>
#include <stdint.h>

#include "ruleset.h"

#ifndef THREADS
#define THREADS 1
#endif

#ifndef DELAY
#define DELAY 100
#endif

#ifndef WIDTH
#define WIDTH 8
#endif

#ifndef HEIGHT
#define HEIGHT 8
#endif

#define _WIDTH WIDTH
#define _HEIGHT (8 * HEIGHT)

#define BLOCK_SIZE 8

#define SEPARATOR 9618

#define CELLS_W (_WIDTH * BLOCK_SIZE)
#define WORDS ((CELLS_W + 63) / 64)
#define TAIL_BITS (CELLS_W - (WORDS - 1) * 64)
#define TAIL_MASK (TAIL_BITS == 64 ? ~0ULL : (1ULL << TAIL_BITS) - 1)

/* A board is one block of _HEIGHT rows x WORDS uint64_t; bit k of word j in
 * a row = column 64*j + k. Bits past the last column are kept at zero. */
typedef uint64_t row_word;

#define ROW(board, linha) ((board) + (size_t)(linha) * WORDS)

static row_word* NewBoard(void) {
  return (row_word*)calloc((size_t)_HEIGHT * WORDS, sizeof(row_word));
}

static inline char GetCell(const row_word* board, int linha, int coluna) {
  return (ROW(board, linha)[coluna / 64] >> (coluna % 64)) & 1;
}

static inline void SetCell(row_word* board, int linha, int coluna, char value) {
  row_word bit = 1ULL << (coluna % 64);
  row_word* w = &ROW(board, linha)[coluna / 64];
  *w = (*w & ~bit) | (value ? bit : 0);
}

/* Row shifted so each bit holds its west / east neighbour, wrapping around. */
static inline row_word West(const row_word* r, int j) {
  row_word carry = j ? r[j - 1] >> 63 : (r[WORDS - 1] >> (TAIL_BITS - 1)) & 1;
  return (r[j] << 1) | carry;
}

static inline row_word East(const row_word* r, int j) {
  if (j == WORDS - 1) return (r[j] >> 1) | ((r[0] & 1) << (TAIL_BITS - 1));
  return (r[j] >> 1) | (r[j + 1] << 63);
}

/* Full adder on 64 lanes: s = x + y + z (low bit), c = carry. */
#define FULL_ADD(x, y, z, s, c) do { row_word t_ = (x) ^ (y); (s) = t_ ^ (z); (c) = ((x) & (y)) | (t_ & (z)); } while (0)

/* Steps 64 cells at once: the 8 neighbours of every cell are shifted copies
 * of the rows around it, summed into a 4-bit counter held as bitplanes
 * s0..s3 (bit k of sN = bit N of cell k's neighbour count).
 * Sum each row of neighbours first (top and bottom 0..3, middle 0..2),
 * then add the three row sums digit by digit. */
static inline row_word StepWord(const row_word* up, const row_word* mid, const row_word* down, int j) {
  row_word a0, a1, c0, c1;
  FULL_ADD(West(up, j), up[j], East(up, j), a0, a1);
  FULL_ADD(West(down, j), down[j], East(down, j), c0, c1);
  row_word mw = West(mid, j), me = East(mid, j);
  row_word b0 = mw ^ me, b1 = mw & me;

  row_word s0, k0, t, k1;
  FULL_ADD(a0, b0, c0, s0, k0);  /* 1s */
  FULL_ADD(a1, b1, c1, t, k1);   /* 2s, plus the carry from the 1s below */
  row_word s1 = t ^ k0, k1b = t & k0;
  row_word s2 = k1 ^ k1b, s3 = k1 & k1b;

  row_word next = RuleBits(s0, s1, s2, s3, mid[j]);  /* generated into ruleset.h */
  return j == WORDS - 1 ? next & TAIL_MASK : next;
}

#ifndef TTY
void Graph(const row_word* board) {
  wchar_t line[_WIDTH * BLOCK_SIZE / 2 + 3];
  char output_line[(_WIDTH * BLOCK_SIZE / 2 + 3) * sizeof(wchar_t)];
  int state = 0;

  for (int i = 0; i < (_WIDTH * BLOCK_SIZE / 2) + 2; i++) { line[i] = SEPARATOR; }
  line[(_WIDTH * BLOCK_SIZE / 2) + 2] = L'\0';
  size_t len = wcstombs(output_line, line, sizeof(output_line));
  fwrite(output_line, sizeof(char), len, stdout);
  fwrite("\n", sizeof(char), 1, stdout);

  for (int i = 0; i < _HEIGHT; i += 4) {
    line[0] = SEPARATOR;

    for (int j = 0; j < _WIDTH; j++) {
      for (int k = 0; k < BLOCK_SIZE; k += 2) {
        state = GetCell(board, i, j * BLOCK_SIZE + k);
        state += 2 * GetCell(board, i + 1, j * BLOCK_SIZE + k);
        state += 4 * GetCell(board, i + 2, j * BLOCK_SIZE + k);
        state += 8 * GetCell(board, i, j * BLOCK_SIZE + k + 1);
        state += 16 * GetCell(board, i + 1, j * BLOCK_SIZE + k + 1);
        state += 32 * GetCell(board, i + 2, j * BLOCK_SIZE + k + 1);
        state += 64 * GetCell(board, i + 3, j * BLOCK_SIZE + k);
        state += 128 * GetCell(board, i + 3, j * BLOCK_SIZE + k + 1);

        line[(j * BLOCK_SIZE) / 2 + k / 2 + 1] = state + 10240;
      }
    }

    line[_WIDTH * BLOCK_SIZE / 2 + 1] = SEPARATOR;
    line[(_WIDTH * BLOCK_SIZE / 2) + 2] = L'\0';

    len = wcstombs(output_line, line, sizeof(output_line));
    fwrite(output_line, sizeof(char), len, stdout);
    fwrite("\n", sizeof(char), 1, stdout);
  }

  for (int i = 0; i < (_WIDTH * BLOCK_SIZE / 2) + 2; i++) { line[i] = SEPARATOR; }
  line[(_WIDTH * BLOCK_SIZE / 2) + 2] = L'\0';
  len = wcstombs(output_line, line, sizeof(output_line));
  fwrite(output_line, sizeof(char), len, stdout);
  fwrite("\n", sizeof(char), 1, stdout);
}
#else
void Graph(const row_word* board) {
  wchar_t chars[4] = { 32,9600,9604,9608 };
  wchar_t line[_WIDTH * BLOCK_SIZE + 3];
  char output_line[(_WIDTH * BLOCK_SIZE + 3) * sizeof(wchar_t)];
  int state = 0;

  for (int i = 0; i < (_WIDTH * BLOCK_SIZE) + 2; i++) { line[i] = SEPARATOR; }
  line[(_WIDTH * BLOCK_SIZE) + 2] = L'\0';
  size_t len = wcstombs(output_line, line, sizeof(output_line));
  fwrite(output_line, sizeof(char), len, stdout);
  fwrite("\n", sizeof(char), 1, stdout);

  for (int i = 0; i < _HEIGHT; i += 2) {
    line[0] = SEPARATOR;

    for (int j = 0; j < _WIDTH; j++) {
      for (int k = 0; k < BLOCK_SIZE; k++) {
        state = GetCell(board, i, j * BLOCK_SIZE + k);
        state += 2 * GetCell(board, i + 1, j * BLOCK_SIZE + k);

        line[(j * BLOCK_SIZE) + k + 1] = chars[state];
      }
    }

    line[_WIDTH * BLOCK_SIZE + 1] = SEPARATOR;
    line[(_WIDTH * BLOCK_SIZE) + 2] = L'\0';

    len = wcstombs(output_line, line, sizeof(output_line));
    fwrite(output_line, sizeof(char), len, stdout);
    fwrite("\n", sizeof(char), 1, stdout);
  }

  for (int i = 0; i < (_WIDTH * BLOCK_SIZE) + 2; i++) { line[i] = SEPARATOR; }
  line[(_WIDTH * BLOCK_SIZE) + 2] = L'\0';
  len = wcstombs(output_line, line, sizeof(output_line));
  fwrite(output_line, sizeof(char), len, stdout);
  fwrite("\n", sizeof(char), 1, stdout);
}
#endif

/* Writes the next generation of *board into *scratch, then swaps them. */
void Iterate(row_word** board, row_word** scratch) {
  const row_word* src = *board;
  row_word* dst = *scratch;
  for (int i = 0; i < _HEIGHT; i++) {
    const row_word* up = ROW(src, i ? i - 1 : _HEIGHT - 1);
    const row_word* down = ROW(src, i + 1 < _HEIGHT ? i + 1 : 0);
    for (int j = 0; j < WORDS; j++) {
      ROW(dst, i)[j] = StepWord(up, ROW(src, i), down, j);
    }
  }
  *scratch = *board;
  *board = dst;
}

int main() {
  setlocale(LC_ALL, "");
  printf("\e[?25l");

  srand(clock());
  row_word* G = NewBoard();
  row_word* Copy = NewBoard();

#ifdef RAND
  for (int i = 0; i < _HEIGHT; i++) {
    for (int j = 0; j < CELLS_W; j++) {
      SetCell(G, i, j, rand() & 1);
    }
  }
#else
  // Glider
  int gx = (_WIDTH / 2) * BLOCK_SIZE;
  SetCell(G, _HEIGHT / 2, gx + 1, 1);
  SetCell(G, (_HEIGHT / 2) + 1, gx, 1);
  SetCell(G, (_HEIGHT / 2) + 2, gx, 1);
  SetCell(G, (_HEIGHT / 2) + 2, gx + 1, 1);
  SetCell(G, (_HEIGHT / 2) + 2, gx + 2, 1);
#endif

  // How to draw:
  // SetCell(G, LINE, COLUMN, 1);

#ifdef SPEEDTEST
  int iterations = 1000;
  struct timespec start, end;
  clock_gettime(CLOCK_MONOTONIC, &start);
  for (int i = 0; i < iterations; i++) {
#else
    while (1) {
#endif
      Graph(G);
      // getchar();
      Iterate(&G, &Copy);
      usleep(DELAY * 1000);
      printf("\e[1;1H\e[2J");
    }
#ifdef SPEEDTEST
    clock_gettime(CLOCK_MONOTONIC, &end);
    double time_spent = (end.tv_sec - start.tv_sec) * 1e6 + (end.tv_nsec - start.tv_nsec) / 1e3;
    printf("Time taken for %d iterations: %f microseconds\n", iterations, time_spent);
#endif
    return 0;
  }

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <locale.h>
#include <stdint.h>

#include "conways.h"



void Graph(block** board) {
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
        state = GetBlockCell(board[i][j], k);
        state += 2 * GetBlockCell(board[i + 1][j], k);
        state += 4 * GetBlockCell(board[i + 2][j], k);
        state += 8 * GetBlockCell(board[i][j], k + 1);
        state += 16 * GetBlockCell(board[i + 1][j], k + 1);
        state += 32 * GetBlockCell(board[i + 2][j], k + 1);
        state += 64 * GetBlockCell(board[i + 3][j], k);
        state += 128 * GetBlockCell(board[i + 3][j], k + 1);

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


int main(){
  setlocale(LC_ALL, "");
  srand(clock());
  block** G = (block**)calloc(_HEIGHT, sizeof(block*));
  for (int i = 0; i < _HEIGHT; i++) {
    G[i] = (block*)calloc(_WIDTH, sizeof(block));
  }

  for (int i = 0; i < _HEIGHT; i++) {
    for (int j = 0; j < _WIDTH; j++) {
      G[i][j].cells = rand() % 256;
    }
  }
  // Graph(G);

  int iterations = 50000;
  struct timespec start, end;
  clock_gettime(CLOCK_MONOTONIC, &start);
  volatile int resultado = 0;
  for (int i = 0; i < iterations; i++) {
    int x = rand()%64;
    int y = rand()%8;
    int index = rand()%2;
    SetBlockCell(G, x, y, index);
  }
  clock_gettime(CLOCK_MONOTONIC, &end);
  double time_spent = (end.tv_sec - start.tv_sec) * 1e6 + (end.tv_nsec - start.tv_nsec) / 1e3;
  printf("%d\n", (int)time_spent);
  // double time_spent = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e3;
  // printf("%d\n", (int)time_spent);
  // printf("Time taken for %d iterations: %f seconds\n resultado: %d\n", iterations, time_spent, resultado);
}

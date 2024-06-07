#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <locale.h>

#include "ruleset.h"

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
#define _HEIGHT 8 * HEIGHT

#define BLOCK_SIZE 8

#define SEPARATOR 9618

union block {
  struct {
    unsigned char a : 1;
    unsigned char b : 1;
    unsigned char c : 1;
    unsigned char d : 1;
    unsigned char e : 1;
    unsigned char f : 1;
    unsigned char g : 1;
    unsigned char h : 1;
  } cell_group;
  unsigned char cells;
};

void SetBlocks(union block* B, short cell, char value) {
  switch (cell) {
    case 0:
      B->cell_group.a = value;
      break;
    case 1:
      B->cell_group.b = value;
      break;
    case 2:
      B->cell_group.c = value;
      break;
    case 3:
      B->cell_group.d = value;
      break;
    case 4:
      B->cell_group.e = value;
      break;
    case 5:
      B->cell_group.f = value;
      break;
    case 6:
      B->cell_group.g = value;
      break;
    case 7:
      B->cell_group.h = value;
      break;
    default:
      printf("Tried to SET out of struct bounds\n");
      exit(1);
  };
}

void SetBlockCell(union block** G, short linha, short coluna, char value) {
  union block* B = &(G[linha][coluna/BLOCK_SIZE]);
  switch (coluna%BLOCK_SIZE) {
    case 0:
      B->cell_group.a = value;
      break;
    case 1:
      B->cell_group.b = value;
      break;
    case 2:
      B->cell_group.c = value;
      break;
    case 3:
      B->cell_group.d = value;
      break;
    case 4:
      B->cell_group.e = value;
      break;
    case 5:
      B->cell_group.f = value;
      break;
    case 6:
      B->cell_group.g = value;
      break;
    case 7:
      B->cell_group.h = value;
      break;
    default:
      printf("Tried to SET out of struct bounds\n");
      exit(1);
  };
}

char GetBlockCell(union block G, short index) {
  switch (index) {
    case 0:
      return G.cell_group.a;
    case 1:
      return G.cell_group.b;
    case 2:
      return G.cell_group.c;
    case 3:
      return G.cell_group.d;
    case 4:
      return G.cell_group.e;
    case 5:
      return G.cell_group.f;
    case 6:
      return G.cell_group.g;
    case 7:
      return G.cell_group.h;
    default:
      printf("Tried to GET out of struct bounds\n");
      exit(1);
  };
}

char GetCellHelper(union block** board, short linha, short coluna){
  linha = (linha+(_HEIGHT))%(_HEIGHT);
  coluna = (coluna+(_WIDTH * BLOCK_SIZE))%(_WIDTH * BLOCK_SIZE);
  return GetBlockCell(board[linha][coluna/BLOCK_SIZE], coluna%BLOCK_SIZE);
}

char GetNeighbours(union block** board, short linha, short coluna) {
  union block neighbours;

  SetBlocks(&neighbours, 0, GetCellHelper(board, linha - 1,coluna - 1));
  SetBlocks(&neighbours, 1, GetCellHelper(board, linha - 1,coluna));
  SetBlocks(&neighbours, 2, GetCellHelper(board, linha - 1,coluna + 1));

  SetBlocks(&neighbours, 3, GetCellHelper(board, linha,coluna - 1));
  SetBlocks(&neighbours, 4, GetCellHelper(board, linha,coluna + 1));

  SetBlocks(&neighbours, 5, GetCellHelper(board, linha + 1,coluna - 1));
  SetBlocks(&neighbours, 6, GetCellHelper(board, linha + 1,coluna));
  SetBlocks(&neighbours, 7, GetCellHelper(board, linha + 1,coluna + 1));

  char count = 0;
  for (int i = 0; i < BLOCK_SIZE; i++) {
    count += GetBlockCell(neighbours, i);
  }

  return RuleStateManager(count, GetBlockCell(board[linha][coluna/BLOCK_SIZE], coluna%BLOCK_SIZE));
}

#ifndef TTY
void Graph(union block** board) {
  wchar_t line[_WIDTH * BLOCK_SIZE / 2 + 3];
  char output_line[(_WIDTH * BLOCK_SIZE / 2 + 3) * sizeof(wchar_t)];
  int state = 0;

  for(int i = 0; i < (_WIDTH * BLOCK_SIZE / 2) + 2; i++) {line[i] = SEPARATOR;}
  line[(_WIDTH * BLOCK_SIZE/2) + 2] = L'\0';
  size_t len = wcstombs(output_line, line, sizeof(output_line));
  fwrite(output_line, sizeof(char), len, stdout);
  fwrite("\n", sizeof(char), 1, stdout);

  for (int i = 0; i < _HEIGHT; i += 4) {
    line[0] = SEPARATOR;

    for (int j = 0; j < _WIDTH; j++) {
      for (int k = 0; k < BLOCK_SIZE; k += 2) {
        state = GetBlockCell(board[i][j], k);
        state += 2 * GetBlockCell(board[i+1][j], k);
        state += 4 * GetBlockCell(board[i+2][j], k);
        state += 8 * GetBlockCell(board[i][j], k+1);
        state += 16 * GetBlockCell(board[i+1][j], k+1);
        state += 32 * GetBlockCell(board[i+2][j], k+1);
        state += 64 * GetBlockCell(board[i+3][j], k);
        state += 128 * GetBlockCell(board[i+3][j], k+1);

        line[(j * BLOCK_SIZE) / 2 + k / 2 + 1] = state + 10240;
      }
    }

    line[_WIDTH * BLOCK_SIZE / 2 + 1] = SEPARATOR;
    line[(_WIDTH * BLOCK_SIZE/2) + 2] = L'\0';

    len = wcstombs(output_line, line, sizeof(output_line));
    fwrite(output_line, sizeof(char), len, stdout);
    fwrite("\n", sizeof(char), 1, stdout);
  }

  for(int i = 0; i < (_WIDTH * BLOCK_SIZE / 2) + 2; i++) {line[i] = SEPARATOR;}
  line[(_WIDTH * BLOCK_SIZE/2) + 2] = L'\0';
  len = wcstombs(output_line, line, sizeof(output_line));
  fwrite(output_line, sizeof(char), len, stdout);
  fwrite("\n", sizeof(char), 1, stdout);
}
#else
void Graph(union block** board) {
  wchar_t chars[4] = {32,9600,9604,9608};
  wchar_t line[_WIDTH * BLOCK_SIZE + 3];
  char output_line[(_WIDTH * BLOCK_SIZE + 3) * sizeof(wchar_t)];
  int state = 0;

  for(int i = 0; i < (_WIDTH * BLOCK_SIZE) + 2; i++) {line[i] = SEPARATOR;}
  line[(_WIDTH * BLOCK_SIZE) + 2] = L'\0';
  size_t len = wcstombs(output_line, line, sizeof(output_line));
  fwrite(output_line, sizeof(char), len, stdout);
  fwrite("\n", sizeof(char), 1, stdout);

  for (int i = 0; i < _HEIGHT; i += 2) {
    line[0] = SEPARATOR;

    for (int j = 0; j < _WIDTH; j++) {
      for (int k = 0; k < BLOCK_SIZE; k++) {
        state = GetBlockCell(board[i][j], k);
        state += 2 * GetBlockCell(board[i + 1][j], k);

        line[(j * BLOCK_SIZE) + k + 1] = chars[state];
      }
    }

    line[_WIDTH * BLOCK_SIZE + 1] = SEPARATOR;
    line[(_WIDTH * BLOCK_SIZE) + 2] = L'\0';

    len = wcstombs(output_line, line, sizeof(output_line));
    fwrite(output_line, sizeof(char), len, stdout);
    fwrite("\n", sizeof(char), 1, stdout);
  }

  for(int i = 0; i < (_WIDTH * BLOCK_SIZE) + 2; i++) {line[i] = SEPARATOR;}
  line[(_WIDTH * BLOCK_SIZE) + 2] = L'\0';
  len = wcstombs(output_line, line, sizeof(output_line));
  fwrite(output_line, sizeof(char), len, stdout);
  fwrite("\n", sizeof(char), 1, stdout);
}
#endif

void Iterate(union block** G, union block** Copy, union block*** Master) {
  if (*Master == G)
  { *Master = Copy; }
  else
  { *Master = G; }

  char state = 0;
  for(int i = 0; i < _HEIGHT; i++){
    for(int j = 0; j < _WIDTH * BLOCK_SIZE; j++){
      if (*Master == Copy) {
        state = GetNeighbours(G, i, j);
        SetBlockCell(Copy, i, j, state);
      } else {
        state = GetNeighbours(Copy, i, j);
        SetBlockCell(G, i, j, state);
      }
    }
  }
}

int main() {
setlocale(LC_ALL, "");
printf("\e[?25l");

  srand(clock());
  union block** G = calloc(_HEIGHT, sizeof *G);
  for (int i = 0; i < _HEIGHT; i++){
    G[i] = calloc(_WIDTH, sizeof(union block));
    for (int j = 0; j < _WIDTH; j++) {
      G[i][j].cells = 0;
    }
  }

  union block** Copy = calloc(_HEIGHT, sizeof *Copy);
  for (int i = 0; i < _HEIGHT; i++){
    Copy[i] = calloc(_WIDTH, sizeof(union block));
    for (int j = 0; j < _WIDTH; j++) {
      Copy[i][j].cells = 0;
    }
  }

  union block*** Master = calloc(1, sizeof *Master);
  *Master = G;

  for(int i = 0; i < _HEIGHT; i++){
    for(int j = 0; j < _WIDTH; j++){
#ifdef RAND
      G[i][j].cells = rand() % 256;
#else
      G[i][j].cells = 0;
#endif
    }
  }

#ifndef RAND
  G[_HEIGHT/2][_WIDTH/2].cells = 2;
  G[(_HEIGHT/2) + 1][_WIDTH/2].cells = 1;
  G[(_HEIGHT/2) + 2][_WIDTH/2].cells = 7;
  // TODO: Add a micro-random input (8x1 block)
#endif

  // How to draw:
  // G[LINE][BLOCK].cells = ENCODING;

  while (1){
    Graph(*Master);
    // getchar();
    Iterate(G, Copy, Master);
    usleep(DELAY*1000);
    system("clear");
  }
  return 0;
}

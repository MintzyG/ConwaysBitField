#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#ifndef RESOLUTION
  #define RESOLUTION 8
#endif

#define WIDTH RESOLUTION
#define HEIGHT 8 * RESOLUTION
#define BLOCK_SIZE 8
#define ALIVE "██"
#define DEAD "  "

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

void Graph(union block**);
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

void SwitchColuna(union block** board, unsigned char linha, char coluna, char* C, char* D) {
  switch (coluna) {
    case 0:
      *C = GetBlockCell(board[linha][WIDTH - 1], (WIDTH - 1)%BLOCK_SIZE);
      *D = GetBlockCell(board[linha][0], 1);
      break;
    case HEIGHT - 1:
      *C = GetBlockCell(board[linha][WIDTH - 1], BLOCK_SIZE - 2);
      *D = GetBlockCell(board[linha][0], 0);
      break;
    default:
      *C = GetBlockCell(board[linha][(coluna - 1)/BLOCK_SIZE], (coluna - 1)%BLOCK_SIZE);
      *D = GetBlockCell(board[linha][(coluna + 1)/BLOCK_SIZE], (coluna + 1)%BLOCK_SIZE);
      break;
  };
}

char GetNeighbours(union block** board, unsigned char linha, unsigned char coluna) {
  char A = 0, B = 0, C = 0, D = 0;
  char DA = 0, DB = 0, DC = 0, DD = 0; 

  switch (linha) {
    case 0:
      A = GetBlockCell(board[HEIGHT - 1][coluna/BLOCK_SIZE], coluna%BLOCK_SIZE);
      B = GetBlockCell(board[1][coluna/BLOCK_SIZE], coluna%BLOCK_SIZE);
      SwitchColuna(board, linha, coluna, &C, &D);
      if (coluna == 0) {
        DA = GetBlockCell(board[HEIGHT - 1][WIDTH - 1], BLOCK_SIZE - 1);
        DB = GetBlockCell(board[HEIGHT - 1][0], 1);
        DC = GetBlockCell(board[1][WIDTH - 1], BLOCK_SIZE - 1);
        DD = GetBlockCell(board[1][0], 1);
      } else if (coluna == HEIGHT - 1) {
        DA = GetBlockCell(board[HEIGHT - 1][WIDTH - 1], BLOCK_SIZE - 2);
        DB = GetBlockCell(board[HEIGHT - 1][0], 0);
        DC = GetBlockCell(board[1][WIDTH - 1], BLOCK_SIZE - 2);
        DD = GetBlockCell(board[1][0], 0);
      } else {
        DA = GetBlockCell(board[HEIGHT - 1][(coluna - 1)/BLOCK_SIZE], (coluna - 1)%BLOCK_SIZE);
        DB = GetBlockCell(board[HEIGHT - 1][(coluna + 1)/BLOCK_SIZE], (coluna + 1)%BLOCK_SIZE);
        DC = GetBlockCell(board[1][(coluna - 1)/BLOCK_SIZE], (coluna - 1)%BLOCK_SIZE);
        DD = GetBlockCell(board[1][(coluna + 1)/BLOCK_SIZE], (coluna + 1)%BLOCK_SIZE);
      }
      break;
    case HEIGHT - 1:
      A = GetBlockCell(board[HEIGHT - 2][coluna/BLOCK_SIZE], coluna%BLOCK_SIZE);
      B = GetBlockCell(board[0][coluna/BLOCK_SIZE], coluna%BLOCK_SIZE);
      SwitchColuna(board, linha, coluna, &C, &D);
      if (coluna == 0) {
        DA = GetBlockCell(board[HEIGHT - 2][WIDTH - 1], BLOCK_SIZE - 1);
        DB = GetBlockCell(board[HEIGHT - 2][0], 1);
        DC = GetBlockCell(board[0][WIDTH - 1], BLOCK_SIZE - 1);
        DD = GetBlockCell(board[0][0], 1);
      } else if (coluna == HEIGHT - 1) {
        DA = GetBlockCell(board[HEIGHT - 2][WIDTH - 1], BLOCK_SIZE - 2);
        DB = GetBlockCell(board[HEIGHT - 2][0], 0);
        DC = GetBlockCell(board[0][WIDTH - 1], BLOCK_SIZE - 2);
        DD = GetBlockCell(board[0][0], 0);
      } else {
        DA = GetBlockCell(board[HEIGHT - 2][(coluna - 1)/BLOCK_SIZE], (coluna - 1)%BLOCK_SIZE);
        DB = GetBlockCell(board[HEIGHT - 2][(coluna + 1)/BLOCK_SIZE], (coluna + 1)%BLOCK_SIZE);
        DC = GetBlockCell(board[0][(coluna - 1)/BLOCK_SIZE], (coluna - 1)%BLOCK_SIZE);
        DD = GetBlockCell(board[0][(coluna + 1)/BLOCK_SIZE], (coluna + 1)%BLOCK_SIZE);
      }
      break;
    default:
      A = GetBlockCell(board[linha + 1][coluna/BLOCK_SIZE], coluna%BLOCK_SIZE);
      B = GetBlockCell(board[linha - 1][coluna/BLOCK_SIZE], coluna%BLOCK_SIZE);
      SwitchColuna(board, linha, coluna, &C, &D);
      if (coluna == 0) {
        DA = GetBlockCell(board[linha + 1][WIDTH - 1], BLOCK_SIZE - 1);
        DB = GetBlockCell(board[linha + 1][0], 1);
        DC = GetBlockCell(board[linha - 1][WIDTH - 1], BLOCK_SIZE - 1);
        DD = GetBlockCell(board[linha - 1][0], 1);
      } else if (coluna == HEIGHT - 1) {
        DA = GetBlockCell(board[linha + 1][WIDTH - 1], BLOCK_SIZE - 2);
        DB = GetBlockCell(board[linha + 1][0], 0);
        DC = GetBlockCell(board[linha - 1][WIDTH - 1], BLOCK_SIZE - 2);
        DD = GetBlockCell(board[linha - 1][0], 0);
      } else {
        DA = GetBlockCell(board[linha + 1][(coluna - 1)/BLOCK_SIZE], (coluna - 1)%BLOCK_SIZE);
        DB = GetBlockCell(board[linha + 1][(coluna + 1)/BLOCK_SIZE], (coluna + 1)%BLOCK_SIZE);
        DC = GetBlockCell(board[linha - 1][(coluna - 1)/BLOCK_SIZE], (coluna - 1)%BLOCK_SIZE);
        DD = GetBlockCell(board[linha - 1][(coluna + 1)/BLOCK_SIZE], (coluna + 1)%BLOCK_SIZE);
      }
      break;
  };

  char count = A + B + C + D + DA + DB + DC + DD;
  if (count >= 4) {
    return 0;
  } else if (count == 2) {
    if (GetBlockCell(board[linha][coluna/BLOCK_SIZE], coluna%BLOCK_SIZE)){
      return 1;
    }
    return 0;
  } else if (count == 3) {
    return 1;
  } else {
    return 0;
  }
}

void Graph(union block** board) {
  char state = 0;
  for (int i = 0; i < HEIGHT; i++) {
    for (int j = 0; j < WIDTH; j++) {
      for (int k = 0; k < BLOCK_SIZE; k++){
        state = GetBlockCell(board[i][j], k);
        if (state == 1) {
          printf(ALIVE);
        } else {
          printf(DEAD);
        }
      }
    }
    printf("\n");
  }
}

void Iterate(union block** G, union block** Copy, union block*** Master) {
  if (*Master == G) 
  { *Master = Copy; } 
  else 
  { *Master = G; }
  
  char state = 0;
  for(int i = 0; i < HEIGHT; i++){
    for(int j = 0; j < HEIGHT; j++){
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
  srand(clock());
  union block** G = calloc(HEIGHT, sizeof *G);
  for (int i = 0; i < HEIGHT; i++){
    G[i] = calloc(WIDTH, sizeof(union block));
    for (int j = 0; j < WIDTH; j++) {
      G[i][j].cells = 0;
    }
  }

  union block** Copy = calloc(HEIGHT, sizeof *G);
  for (int i = 0; i < HEIGHT; i++){
    Copy[i] = calloc(WIDTH, sizeof(union block));
    for (int j = 0; j < WIDTH; j++) {
      Copy[i][j].cells = 0;
    }
  }

  union block*** Master = calloc(1, sizeof *Master);
  *Master = G;

  for(int i = 0; i < HEIGHT; i++){
    for(int j = 0; j < WIDTH; j++){
      // G[i][j].cells = rand() % 256;
      G[i][j].cells = 0;
    }
  }

  // How to draw:
  // G[LINE][BLOCK].cells = ENCODING;

  G[16-10][4].cells = 64;
  G[17-10][4].cells = 80;
  G[18-10][4].cells = 12;
  G[19-10][4].cells = 12;
  G[20-10][4].cells = 12;
  G[21-10][4].cells = 80;
  G[22-10][4].cells = 64;

  G[18-10][6].cells = 3;
  G[19-10][6].cells = 3;

  G[18-10][3].cells = 12;
  G[19-10][3].cells = 34;
  G[20-10][3].cells = 65;
  G[21-10][3].cells = 209;
  G[22-10][3].cells = 65;
  G[23-10][3].cells = 34;
  G[24-10][3].cells = 12;

  G[20-10][1].cells = 192;
  G[21-10][1].cells = 192;

  G[49+7][4].cells = 64;
  G[50+7][4].cells = 80;
  G[51+7][4].cells = 12;
  G[52+7][4].cells = 12;
  G[53+7][4].cells = 12;
  G[54+7][4].cells = 80;
  G[55+7][4].cells = 64;

  G[52+7][6].cells = 3;
  G[53+7][6].cells = 3;

  G[47+7][3].cells = 12;
  G[48+7][3].cells = 34;
  G[49+7][3].cells = 65;
  G[50+7][3].cells = 209;
  G[51+7][3].cells = 65;
  G[52+7][3].cells = 34;
  G[53+7][3].cells = 12;

  G[50+7][1].cells = 192;
  G[51+7][1].cells = 192;

  while (1){
    Graph(*Master);
    // getchar();
    Iterate(G, Copy, Master);
    // printf("\n");
    Graph(*Master);
    usleep(100000);
    system("clear");
  }
  return 0;
}
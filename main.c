#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <locale.h>
#include <ncurses.h>

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

inline void SetBlockCell(union block** G, short linha, short coluna, char value) {
  G[linha][coluna / BLOCK_SIZE].cells = (G[linha][coluna / BLOCK_SIZE].cells & ~(1 << (coluna % BLOCK_SIZE))) | (value << (coluna % BLOCK_SIZE));
}

inline char GetBlockCell(union block G, short index) {
  return (G.cells >> index) & 1;
}

inline char GetCellHelper(union block** board, short linha, short coluna) {
  linha = (linha + _HEIGHT) % _HEIGHT;
  coluna = (coluna + _WIDTH * BLOCK_SIZE) % (_WIDTH * BLOCK_SIZE);
  return GetBlockCell(board[linha][coluna / BLOCK_SIZE], coluna % BLOCK_SIZE);
}

inline char GetNeighbours(union block** board, short linha, short coluna) {
  char count = 0;
  count += GetCellHelper(board, linha - 1, coluna - 1);
  count += GetCellHelper(board, linha - 1, coluna);
  count += GetCellHelper(board, linha - 1, coluna + 1);
  count += GetCellHelper(board, linha, coluna - 1);
  count += GetCellHelper(board, linha, coluna + 1);
  count += GetCellHelper(board, linha + 1, coluna - 1);
  count += GetCellHelper(board, linha + 1, coluna);
  count += GetCellHelper(board, linha + 1, coluna + 1);
  return RuleStateManager(count, GetCellHelper(board, linha, coluna));
}

void GetBlockNeighbours(union block** board, union block** inverse, short linha, short coluna) {
  for (int i = 0; i < BLOCK_SIZE; i++) {
    SetBlockCell(inverse, linha, coluna * BLOCK_SIZE + i, GetNeighbours(board, linha, coluna * BLOCK_SIZE + i));
  }
}

#ifndef TTY
void Graph(union block** board) {
  wchar_t line[_WIDTH * BLOCK_SIZE / 2 + 3];
  char output_line[(_WIDTH * BLOCK_SIZE / 2 + 3) * sizeof(wchar_t)];
  int state = 0;

  move(0, 0);

  for (int i = 0; i < (_WIDTH * BLOCK_SIZE / 2) + 2; i++) { line[i] = SEPARATOR; }
  line[(_WIDTH * BLOCK_SIZE / 2) + 2] = L'\0';
  size_t len = wcstombs(output_line, line, sizeof(output_line));
  printw("%s\n", output_line);

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
    printw("%s\n", output_line);
  }

  for (int i = 0; i < (_WIDTH * BLOCK_SIZE / 2) + 2; i++) { line[i] = SEPARATOR; }
  line[(_WIDTH * BLOCK_SIZE / 2) + 2] = L'\0';
  len = wcstombs(output_line, line, sizeof(output_line));
  printw("%s\n", output_line);
  refresh();
}
#else
void Graph(union block** board) {
  wchar_t chars[4] = { 32,9600,9604,9608 };
  wchar_t line[_WIDTH * BLOCK_SIZE + 3];
  char output_line[(_WIDTH * BLOCK_SIZE + 3) * sizeof(wchar_t)];
  int state = 0;

  move(0, 0);

  for (int i = 0; i < (_WIDTH * BLOCK_SIZE) + 2; i++) { line[i] = SEPARATOR; }
  line[(_WIDTH * BLOCK_SIZE) + 2] = L'\0';
  size_t len = wcstombs(output_line, line, sizeof(output_line));
  printw("%s\n", output_line);

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
    printw("%s\n", output_line);
  }

  for (int i = 0; i < (_WIDTH * BLOCK_SIZE) + 2; i++) { line[i] = SEPARATOR; }
  line[(_WIDTH * BLOCK_SIZE) + 2] = L'\0';
  len = wcstombs(output_line, line, sizeof(output_line));
  printw("%s\n", output_line);
  refresh();
}
#endif
void Iterate(union block** G, union block** Copy, union block*** Master) {
  for (int i = 0; i < _HEIGHT; i++) {
    for (int j = 0; j < _WIDTH; j++) {
      GetBlockNeighbours(*Master, (*Master == G) ? Copy : G, i, j);
    }
  }
  *Master = (*Master == G) ? Copy : G;
}

void setupExtendedMouse() {
    printf("\033[?1003h"); // Enable mouse movement tracking
    printf("\033[?1015h"); // Enable urxvt-style mouse mode
    printf("\033[?1006h"); // Enable SGR extended mouse coordinates
}

bool getSubCellPosition(MEVENT *event, float *subX, float *subY) {
    if (event->bstate & REPORT_MOUSE_POSITION) {
        *subX = ((event->bstate >> 16) & 0xFF) / 255.0;
        *subY = ((event->bstate >> 24) & 0xFF) / 255.0;
        return true;
    }
    *subX = 0.5;
    *subY = 0.5;
    return false;
}

void handleMouseClick(int y, int x, union block** board) {
    if (y <= 0 || x <= 0) return;

    MEVENT event;
    getmouse(&event);

    float subX, subY;
    getSubCellPosition(&event, &subX, &subY);
#ifndef TTY
    if (y >= _HEIGHT/4 + 1 || x >= (_WIDTH * BLOCK_SIZE)/2 + 1) return;

    y--;
    x--;
    int baseX = x * 2;
    int baseY = y * 4;
    int columnOffset = (subX >= 0.5) ? 1 : 0;
    int rowOffset;

    if (subY < 0.25) rowOffset = 0;
    else if (subY < 0.5) rowOffset = 1;
    else if (subY < 0.75) rowOffset = 2;
    else rowOffset = 3;

    int cellX = baseX + columnOffset;
    int cellY = baseY + rowOffset;
#else
    if (y >= _HEIGHT/2 + 1 || x >= _WIDTH * BLOCK_SIZE + 1) return;

    y--;
    x--;
    int baseX = x;
    int baseY = y * 2;
    int cellX = baseX;
    int cellY = baseY + ((subY >= 0.5) ? 1 : 0);
#endif
    if (cellX < _WIDTH * BLOCK_SIZE && cellY < _HEIGHT) {
        char currentValue = GetCellHelper(board, cellY, cellX);
        SetBlockCell(board, cellY, cellX, !currentValue);
    }
}

int main() {
  setlocale(LC_ALL, "");
  initscr();
  cbreak();
  noecho();
  curs_set(0);
  keypad(stdscr, TRUE);
  timeout(0);
  mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);
  setupExtendedMouse();

  bool paused = false;
  MEVENT event;
  int ch;

  srand(clock());
  union block** G = (union block**)calloc(_HEIGHT, sizeof(union block*));
  for (int i = 0; i < _HEIGHT; i++) {
    G[i] = (union block*)calloc(_WIDTH, sizeof(union block));
  }

  union block** Copy = (union block**)calloc(_HEIGHT, sizeof(union block*));
  for (int i = 0; i < _HEIGHT; i++) {
    Copy[i] = (union block*)calloc(_WIDTH, sizeof(union block));
  }

  union block*** Master = (union block***)calloc(1, sizeof(union block**));
  *Master = G;

  for (int i = 0; i < _HEIGHT; i++) {
    for (int j = 0; j < _WIDTH; j++) {
#ifdef RAND
      G[i][j].cells = rand() % 256;
#else
      G[i][j].cells = 0;
#endif
    }
  }

#ifndef RAND
  G[_HEIGHT / 2][_WIDTH / 2].cells = 2;
  G[(_HEIGHT / 2) + 1][_WIDTH / 2].cells = 1;
  G[(_HEIGHT / 2) + 2][_WIDTH / 2].cells = 7;
#endif

  // How to draw:
  // G[LINE][BLOCK].cells = ENCODING;

#ifdef SPEEDTEST
  int iterations = 1000;
  struct timespec start, end;
  clock_gettime(CLOCK_MONOTONIC, &start);
  for (int i = 0; i < iterations; i++) {
#else
    while (1) {
#endif
      ch = getch();
      if (ch == 'q' || ch == 'Q') { break; }
      if (ch == ' ') { paused = !paused; }
      if (ch == KEY_MOUSE && paused) {
        if (getmouse(&event) == OK) {
          handleMouseClick(event.y, event.x, *Master);
        }
      }
      Graph(*Master);
      // getchar();
      if (!paused) {
        Iterate(G, Copy, Master);
      }
      usleep(DELAY * 1000);
      move(0, 0);
    }
#ifdef SPEEDTEST
    clock_gettime(CLOCK_MONOTONIC, &end);
    double time_spent = (end.tv_sec - start.tv_sec) * 1e6 + (end.tv_nsec - start.tv_nsec) / 1e3;
    printf("Time taken for %d iterations: %f microseconds\n", iterations, time_spent);
#endif
    endwin();
    return 0;
  }

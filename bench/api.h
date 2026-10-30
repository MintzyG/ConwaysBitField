#pragma once

/* Interface every adapter implements. The driver only talks to a board
 * through these, so any version of main.c can be timed the same way. */
void* bench_create(int n);                      /* n x n cells, all dead */
void bench_set(void* b, int row, int col, int alive);
int bench_get(void* b, int row, int col);
void bench_step(void* b);                       /* one generation */

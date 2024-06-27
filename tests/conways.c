#include "conways.h"
#include <stdio.h>
#include <stdlib.h>

void SetBlockCell(block** G, short linha, short coluna, char value) {
  G[linha][coluna / BLOCK_SIZE].cells = (G[linha][coluna / BLOCK_SIZE].cells & ~(1 << (coluna % BLOCK_SIZE))) | (value << (coluna % BLOCK_SIZE));
}

char GetBlockCell(block G, short index) {
  return (G.cells >> index) & 1;
}

char GetCellHelper(union block** board, short linha, short coluna) {
  linha = (linha + _HEIGHT) % _HEIGHT;
  coluna = (coluna + _WIDTH * BLOCK_SIZE) % (_WIDTH * BLOCK_SIZE);
  return GetBlockCell(board[linha][coluna / BLOCK_SIZE], coluna % BLOCK_SIZE);
}

#include "conways.h"
#include <stdio.h>
#include <stdlib.h>

void SetBlockCell(block** G, short linha, short coluna, char value) {
  G[linha][coluna / BLOCK_SIZE].cells = (G[linha][coluna / BLOCK_SIZE].cells & ~(1 << (coluna % BLOCK_SIZE))) | (value << (coluna % BLOCK_SIZE));
}

char GetBlockCell(block G, short index) {
  return (G.cells >> index) & 1;
}


// void SetBlockCell(block** G, short linha, short coluna, char value) {
//   union block* B = &(G[linha][coluna/BLOCK_SIZE]);
//   switch (coluna%BLOCK_SIZE) {
//     case 0:
//       B->cell_group.a = value;
//       break;
//     case 1:
//       B->cell_group.b = value;
//       break;
//     case 2:
//       B->cell_group.c = value;
//       break;
//     case 3:
//       B->cell_group.d = value;
//       break;
//     case 4:
//       B->cell_group.e = value;
//       break;
//     case 5:
//       B->cell_group.f = value;
//       break;
//     case 6:
//       B->cell_group.g = value;
//       break;
//     case 7:
//       B->cell_group.h = value;
//       break;
//     default:
//       printf("Tried to SET out of struct bounds\n");
//       exit(1);
//   };
// }
//
// char GetBlockCell(block G, short index) {
//   switch (index) {
//     case 0:
//       return G.cell_group.a;
//     case 1:
//       return G.cell_group.b;
//     case 2:
//       return G.cell_group.c;
//     case 3:
//       return G.cell_group.d;
//     case 4:
//       return G.cell_group.e;
//     case 5:
//       return G.cell_group.f;
//     case 6:
//       return G.cell_group.g;
//     case 7:
//       return G.cell_group.h;
//     default:
//       printf("Tried to GET out of struct bounds\n");
//       exit(1);
//   };
// }

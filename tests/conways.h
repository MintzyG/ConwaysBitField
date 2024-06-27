#pragma once 

#define _HEIGHT 64
#define _WIDTH 8
#define BLOCK_SIZE 8
#define SEPARATOR 9618

typedef union block block;
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

void SetBlockCell(block** G, short linha, short coluna, char value);
char GetBlockCell(block G, short index);

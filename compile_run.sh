#!/usr/bin/env bash 

echo "How tall do you want the matrix from 1-16:"
read Height

echo "How wide do you want the matrix from 1-16:"
read Width

echo "Do you want a random board? (y/Y | n/N)"
read Rand

echo "What delay do you want 1ms - 1000ms:"
read Delay

if [[ "$Rand" == "Y" || "$Rand" == "y" ]]; then
  gcc main.c -o game.out -DDELAY=$Delay -DRAND -DWIDTH=$Width -DHEIGHT=$Height -O3 -Wall -Wextra
else
  gcc main.c -o game.out -DDELAY=$Delay -DWIDTH=$Width -DHEIGHT=$Height -O3 -Wall -Wextra
fi
./game.out

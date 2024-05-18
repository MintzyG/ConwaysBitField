#!/usr/bin/env bash

echo "How tall do you want the matrix from 1-16:"
read Height
Height=${Height:-10}

echo "How wide do you want the matrix from 1-16:"
read Width
Width=${Width:-10}

echo "Do you want a random board? (y/Y | n/N)"
read Rand
Rand=${Rand:-Y}

echo "What delay do you want 1ms - 1000ms:"
read Delay
Delay=${Delay:-125}

echo "Are you going to run it through tty? (y/Y | n/N)"
read Tty
Tty=${Tty:-N}

gcc_cmd="gcc main.c -o game.out -DDELAY=$Delay -DWIDTH=$Width -DHEIGHT=$Height -Wall -Wextra -O3"

if [[ "$Rand" == "Y" || "$Rand" == "y" ]]; then
  gcc_cmd+=" -DRAND"
fi

if [[ "$Tty" == "Y" || "$Tty" == "y" ]]; then
  gcc_cmd+=" -DTTY"
fi

eval $gcc_cmd

./game.out

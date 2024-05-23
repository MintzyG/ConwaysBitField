#!/usr/bin/env bash

pushd ./modules
./out.out
popd

echo "Do you want a max size board? (y/Y | n/N)"
read Max
Max=${Max:-N}

echo "Are you going to run it through tty? (y/Y | n/N)"
read Tty
Tty=${Tty:-N}

if [[ "$Max" == "N" || "$Max" == "n" ]]; then
  echo "How tall do you want the matrix from 1-16:"
  read Height
  Height=${Height:-16}

  echo "How wide do you want the matrix from 1-16:"
  read Width
  Width=${Width:-16}
else
  Lines=$(tput lines)
  Columns=$(tput cols)
  if [[ "$Tty" == "Y" || "$Tty" == "y" ]]; then
    Height=$(((Lines -3)/4))
    Width=$(((Columns -2)/8))
  else
     Height=$(((Lines -3)/2))
    Width=$(((Columns -2)/4))
  fi
fi

echo "Do you want a random board? (y/Y | n/N)"
read Rand
Rand=${Rand:-Y}

echo "What delay do you want 1ms - 1000ms:"
read Delay
Delay=${Delay:-100}

gcc_cmd="gcc main.c modules/ruleset.c -o game.out -DDELAY=$Delay -DWIDTH=$Width -DHEIGHT=$Height -Wall -Wextra -O3 -Imodules"

if [[ "$Rand" == "Y" || "$Rand" == "y" ]]; then
  gcc_cmd+=" -DRAND"
fi

if [[ "$Tty" == "Y" || "$Tty" == "y" ]]; then
  gcc_cmd+=" -DTTY"
fi

eval $gcc_cmd && ./game.out

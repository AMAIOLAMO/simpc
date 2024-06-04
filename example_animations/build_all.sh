#!/bin/bash

for animation in ./*.c
do
  echo "building $animation"
  cc -Wall -Wextra -fPIC -shared -o "$(echo "$animation" | sed "s/\.c//").so" "$animation"
done


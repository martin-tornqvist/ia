#!/bin/bash

if [ "x$1" = "x-h" ]; then
    echo "Usage: valgrind.sh [-s]"
    echo "  -s        generate suppressions"
    exit 0
fi    

if [ "x$1" = "x-s" ]; then
    generate_suppressions="--gen-suppressions=all"
fi

cd target
valgrind \
  --track-origins=yes \
  --leak-check=full \
  --show-possibly-lost=no \
  --show-reachable=no \
  $generate_suppressions \
  --suppressions=./../tools/valgrind.supp \
  ./ia

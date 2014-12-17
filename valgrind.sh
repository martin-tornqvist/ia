#!/bin/bash

if [ "x$1" = "x-h" ]; then
    echo "Usage: run-with-valgrind.sh [-s]"
    echo "  -s        generate suppressions"
    exit 0
fi    

if [ "x$1" = "x-s" ]; then
    supp_flag="--gen-suppressions=all"
fi

cd target
valgrind --track-origins=yes --leak-check=full --show-reachable=yes $supp_flag \
    --suppressions=./../tools/valgrind.supp ./ia

#!/bin/bash

set -xue

root_dir=$PWD

mkdir -p build
cd build
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 ../
make -j 4 ia-debug

cd $root_dir

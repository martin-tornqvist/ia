#!/bin/bash

set -xue

root_dir=$PWD

mkdir -p build
cd build
cmake -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS=1 ../
ninja ia-debug

cd $root_dir

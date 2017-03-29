#!/bin/bash

#
# NOTE: This assumes that CMake has already been run
#

root_dir=$PWD

cd build && make -j 4 ia-debug && ./ia-debug

cd $root_dir

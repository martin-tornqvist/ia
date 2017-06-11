#!/bin/bash

set -xue

root_dir=$PWD

./build-debug.sh

cd build

./ia-debug

cd $root_dir

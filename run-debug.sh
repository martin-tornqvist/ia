#!/bin/bash

root_dir=$PWD

make debug -j 2 && cd target/debug && ./ia

cd $root_dir

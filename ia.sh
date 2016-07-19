#!/bin/bash

root_dir=$PWD

make debug -j 4 && cd target && ./ia

cd $root_dir

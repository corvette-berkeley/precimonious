#!/bin/bash

# ###################################
# Shell script to print coverage trace
# Use: ./coverage.sh file .
# ####################################

sourcePath=$CORVETTE_PATH"/src"
sharedLib=$CORVETTE_PATH"/src/Passes.so"

# compile counters into driver
clang -emit-llvm -c $sourcePath/branches.c -o $2/branches.bc
llvm-link -o $2/temp_$1.bc $2/$1.bc $2/branches.bc

# run pass to instrument code
opt -load $sharedLib -coverage --exclude $sourcePath/exclude_coverage.txt $2/temp_$1.bc > $2/c_$1.bc
echo "Done instrumenting program with coverage information" c_$1.bc

# cleaning up
rm -f $2/temp_$1.bc





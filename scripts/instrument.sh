#!/bin/bash

# ###################################
# Shell script to remove output files
# Use: ./instrument.sh file .
# ####################################

sourcePath=$CORVETTE_PATH"/src"
sharedLib=$sourcePath"/Passes.so"

# compile counters and link with bitcode to be instrumented
clang -emit-llvm -c $sourcePath/counters.c -o $2/counters.bc
llvm-link -o $2/temp_$1.bc $2/$1.bc $2/counters.bc

# run pass to instrument code
opt -load $sharedLib -instrument --exclude $sourcePath/exclude_instrument.txt $2/temp_$1.bc > $2/i_$1.bc
echo "Done instrumenting program" i_$1.bc

# cleaning up
rm -f $2/temp_$1.bc





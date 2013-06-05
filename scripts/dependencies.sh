#!/bin/bash

# ##########################################
# Shell script to run the metrics LLVM pass
# Use: ./dependencies.sh name_of_bc_file function_main
# ##########################################

sharedLib=$CORVETTE_PATH"/precision-tuning/Passes.so"

echo "Finding dependencies file for " $1.bc
opt -load $sharedLib -create-call-dependency $1.bc --call-main $2 > $1.tmp

# cleaning up
rm -f $1.tmp





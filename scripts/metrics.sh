#!/bin/bash

# ##########################################
# Shell script to run the metrics LLVM pass
# Use: ./metrics.sh name_of_bc_file
# ##########################################

sharedLib=$CORVETTE_PATH"/src/Passes.so"

echo "Creating metrics file for " $1.bc
opt -load $sharedLib -measure-metric $1.bc > $1.metrics





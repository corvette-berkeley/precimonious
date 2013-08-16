#!/bin/bash

# #############################################################
# Shell script to create search file for a given program
# Use: ./print_functions.sh name_of_bc_file
# #############################################################

sharedLib=$CORVETTE_PATH"/src/Passes.so"

arch=`uname -a`
name=${arch:0:6}

if [ "$name" = "Darwin" ]; then
    sharedLib=$CORVETTE_PATH"/src/Passes.dylib"    
fi

echo "Printing function names"
opt -load $sharedLib -print-names $1.bc -disable-output

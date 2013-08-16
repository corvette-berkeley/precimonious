#!/bin/bash

# #############################################################
# Shell script to create search file for a given program
# Use: ./print_functions.sh name_of_bc_file
# #############################################################

sharedLib=$CORVETTE_PATH"/src/Passes.so"

# if Darwin, the file extension is different
arch=`uname -a`
name=${arch:0:6}
if [ "$name" = "Darwin" ]; then
    sharedLib=$CORVETTE_PATH"/src/Passes.dylib"    
fi

echo "Replacing function calls"
opt -load $sharedLib -json-config=config_$1.json -function-calls $1.bc > transformed.bc

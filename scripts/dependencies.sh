#!/bin/bash

# ##########################################
# Shell script to run the metrics LLVM pass
# Use: ./dependencies.sh name_of_bc_file function_main
# ##########################################

arch=`uname -a`
name=${arch:0:6}

sharedLib=$CORVETTE_PATH"/src/Passes.so"

if [ "$name" = "Darwin" ]; then
    sharedLib=$CORVETTE_PATH"/src/Passes.dylib"    
fi

echo "Finding dependencies file for " $1.bc
opt -load $sharedLib -create-call-dependency $1.bc --call-main $2 > $1.tmp

# cleaning up
rm -f $1.tmp





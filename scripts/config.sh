#!/bin/bash

# #############################################################
# Shell script to create configuration file with original types
# Use: ./config.sh name_of_bc_file
# #############################################################

arch=`uname -a`
name=${arch:0:6}

sharedLib=$CORVETTE_PATH"/src/Passes.so"

if [ "$name" = "Darwin" ]; then
    sharedLib=$CORVETTE_PATH"/src/Passes.dylib"    
fi

varFlags="--only-arrays --only-scalars --funs"

echo "Creating type configuration file" config_$1.json
opt -load $sharedLib -config-file $varFlags $1.bc --filename config_$1.json > $1.tmp

# cleaning up
rm -f $1.tmp




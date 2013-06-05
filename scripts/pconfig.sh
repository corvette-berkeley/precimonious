#!/bin/bash

# #############################################################
# Shell script to create configuration file with original types
# Use: ./config.sh name_of_bc_file
# #############################################################

sharedLib=$CORVETTE_PATH"/precision-tuning/Passes.so"
varFlags="--only-arrays --only-scalars"

echo "Creating type configuration file" config_$1.json
opt -load $sharedLib -config-file $varFlags --pformat $1.bc --filename config_$1.json > $1.tmp

# cleaning up
rm -f $1.tmp




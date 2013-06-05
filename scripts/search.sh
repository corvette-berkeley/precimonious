#!/bin/bash

# #############################################################
# Shell script to create search file for a given program
# Use: ./search.sh name_of_bc_file
# #############################################################

sharedLib=$CORVETTE_PATH"/precision-tuning/Passes.so"
varFlags="--only-arrays --only-scalars"

echo "Creating search file search file" search_$1.json
opt -load $sharedLib -search-file --original-type $varFlags $1.bc --filename search_$1.json > $1.tmp

# cleaning up
rm -f $1.tmp

#!/bin/bash

# #############################################################
# Shell script to create configuration file with original types
# Use: ./run.sh name_of_bc_file .
# #############################################################

scripts=$CORVETTE_PATH"/scripts"

sh $scripts/clean.sh $2

sh $scripts/compile.sh $1 $2

#sh $scripts/search.sh $1

#sh $scripts/pconfig.sh $1

echo "Running dd algorithm"
$scripts/dd.py $1.bc searchFile.json $1.json





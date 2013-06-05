#!/bin/bash

opt -strip $1 -o tmp01.bc 
opt -strip $2 -o tmp02.bc 
llvm-dis < tmp01.bc > tmp01
llvm-dis < tmp02.bc > tmp02
diff=$(diff tmp01 tmp02 | wc -l)
diff=$(echo $diff)
if [[ "$diff" == "0" ]]
then echo "true"
else echo "false ($diff lines of differences)"
fi

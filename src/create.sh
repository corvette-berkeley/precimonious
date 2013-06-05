#!/bin/bash

path="/Users/nacuong/Documents/Berkeley/research/corvette/precision-tuning"
target="output.txt"

echo "Creating bitcode file"
clang -c -emit-llvm -g -o $1.bc $1.c

echo "Creating text format of bitcode file"
llvm-dis < $1.bc > $1.txt

echo "Running LLVM pass"
opt -load $path/Passes.dylib -search-file $1.bc > $target
#opt -load $path/Passes.so -print-vars $1.bc > $target

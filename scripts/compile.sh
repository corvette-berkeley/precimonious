#!/bin/bash

# ###################################
# Shell script to remove output files
# Use: ./compile.sh file .
# ####################################

loggingPath=$CORVETTE_PATH"/logging"

clang -emit-llvm -c $2/$1.c -o $2/temp_$1.bc
clang -emit-llvm -c $loggingPath/cov_checker.c -o $2/cov_checker.bc
clang -emit-llvm -c $loggingPath/cov_serializer.c -o $2/cov_serializer.bc
clang -emit-llvm -c $loggingPath/cov_log.c -o $2/cov_log.bc
llvm-link -o $2/$1.bc $2/temp_$1.bc $2/cov_checker.bc $2/cov_serializer.bc $2/cov_log.bc


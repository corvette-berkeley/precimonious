# Precimonious v0.1

## Overview
__Precimonious__ employs a _dynamic program analysis_ technique to find a lower
floating-point precision that can be used in any part of a program.
Precimonious performs a search on the program variables trying to lower their
precision subject to accuracy constraints and performance goals. The tool then
recommends a type instantiation for these variables using less precision while
producing an accurate enough answer without causing exceptions.

This work was presented at the International Conference for High Performance
Computing, Networking, Storage and Analysis (SC'13) in November 2013. 

## Installation Instruction
### Requirement
1. Scons build system. 
2. LLVM 3.0. When building LLVM, use --enable-shared flag.
```
../llvm/configure --enable-shared
make
```
3. Set the following environment variable.
```
CORVETTE_PATH=path/to/precimonious
LLVM_COMPILER=clang
LD_LIBRARY_PATH=path/to/llvm/Release/lib
PATH=$PATH:path/to/llvm/Release/bin
```

### Instruction
After setting up the requirement, you can install Precimonious by
> cd src
> scons -Uc
> scons -U
> scons -U test // to run the regression test

## Running the Example

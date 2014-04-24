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
* Scons build system. 
* LLVM 3.0. When building LLVM, use --enable-shared flag.
```
../llvm/configure --enable-shared
make
```
* Set the following environment variable.
```
CORVETTE_PATH=path/to/precimonious
LLVM_COMPILER=clang
LD_LIBRARY_PATH=path/to/llvm/Release/lib
PATH=$PATH:path/to/llvm/Release/bin
```

### Instruction
After setting up the requirement, you can install Precimonious by

```
cd src
scons -Uc
scons -U
scons -U test // to run the regression test
```

## Running the Example
* Go inside _examples_ folder, take a look at the file funarc.c. This is the target program that we will tune precision on.
* Compile the program with the following command
```
./compile funarc.c . (remember there is a dot at the end)
```
This will create a bitcode file called funarc.bc, together with some other temporary files.
```
lli funarc.bc
```
This will create a file called spec.cov. This file contains the output and the error threshold in hex format.
* Open funarc.c, comment out the code at line 100, so that the next time the program runs, it will not create the specification file spec.cov again.
```
// cov_arr_spec_log("spec.cov", threshold, INPUTS, log)
```
* Compile funarc again
```
./compile funarc.c .
```
* Now you can run Precimonious to tune precision of funarc using the following command.
```
../scripts/dd2.py funarc.bc search_funarc.json config_funarc.json
```
This will create two files: _dd2_diff_funarc.bc.json_ and _dd2_valid_funarc.bc.json_. 

The first file (_dd2_diff_funarc.bc.json_) tells you which variables can be converted to double or float. 

The second file (_dd2_valid_funarc.bc.json_) is the type configuration file in json format. Changing the precision according to the type configuration produces a program that uses less precision and runs faster than the original program.

You may also wonder what are the other files search_funarc.json and config_funarc.json?

```
search_funarc.json: specify search space for Precimonious. To generate this automatically, run:
../scripts/search.sh funarc .
config_funarc.json: the type configuration of the original program. To generate this automatically, run:
 ../scripts/pconfig.sh funarc .
```


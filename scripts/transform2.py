#!/usr/bin/env python

import os
import proc
import platform
import sys
from subprocess import CalledProcessError, PIPE, Popen, call


# #############################################################
# Python script that transforms code, runs it and checks result
# See main.py for a sample on how to invoke this script
# #############################################################

def transform(bitcodefile, configfile):

    # setting some variables
    corvettepath = os.getenv("CORVETTE_PATH")

    if platform.system() == 'Darwin':
        sharedlib = corvettepath + "/src/Passes.dylib"
    else:
        sharedlib = corvettepath + "/src/Passes.so"

    # modified bitcode file
    mbitcodefile = "m_" + sys.argv[1]
    mbitcode = open(mbitcodefile, 'w')

    # temp bitcode file
    tempfile = "temp_" + sys.argv[1]
    temp = open(tempfile, 'w')

    output = open("output.txt", 'w')

    # ######################################
    # running the transformation LLVM passes
    # passes are run in the order the appear
    # ######################################
    command = ['opt', '-load', sharedlib, "-json-config=" + configfile, "-adjust-operators", bitcodefile]
    retval = call(command, stdin=None, stdout=mbitcode, stderr=None)

    # return -1 if running LLVM passes fails
    if retval <> 0:
        return -1 
    
    # running modified bitcode
    command = ['opt', '-O2', mbitcodefile, '-o', sys.argv[1] + '_opt.bc']
    call(command, stdin=None, stdout=None, stderr=None)
    command = ['llc', sys.argv[1] + '_opt.bc', '-o', sys.argv[1] + '.s']
    call(command, stdin=None, stdout=None, stderr=None)
    command = ['clang', sys.argv[1] + '.s', '-lm', '-o', sys.argv[1] + '.out']
    call(command, stdin=None, stdout=None, stderr=None)
    command = ['./' + sys.argv[1] + '.out']
    retval = call(command, stdin=None, stdout=None, stderr=None)

    #try:
    #    retval = proc.run(command, timeout=5)
    #except proc.Timeout:
    #  return -4

    # return -3 if crashed when run
    if retval <> 0:
        return -3 

    output = open("sat.cov", 'r')
    firstline = output.readline()
    firstline = firstline.strip()
    if (firstline == "true"):
      return 1
    else:
      return 0


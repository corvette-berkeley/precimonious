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

def transform(bitcodefile, configfile, inx):

    # setting some variables
    corvettepath = os.getenv("CORVETTE_PATH")

    if platform.system() == 'Darwin':
        sharedlib = corvettepath + "/precision-tuning/Passes.dylib"
    else:
        sharedlib = corvettepath + "/precision-tuning/Passes.so"

    # modified bitcode file
    mbitcodefile = "m_" + sys.argv[1] + "_" + str(inx)
    mbitcode = open(mbitcodefile, 'w')

    # modified + optimized bitcode file
    obitcodefile = "o_" + sys.argv[1] + "_" + str(inx)
    obitcode = open(obitcodefile, 'w')

    # temp bitcode file
    tempfile = "temp_" + sys.argv[1] + "_" + str(inx)
    temp = open(tempfile, 'w')

    output = open("output.txt", 'w')

    # ######################################
    # running the transformation LLVM passes
    # passes are run in the order the appear
    # ######################################
    command = ['opt', '-load', sharedlib, "-json-config=" + configfile, "-adjust-operators", "--die", bitcodefile]
    retval = call(command, stdin=None, stdout=mbitcode, stderr=None)

    # return -1 if running LLVM passes fails
    if retval <> 0:
        return -1 
    
    # ###########################
    # removing redundant castings
    # ###########################
    command = ['opt', '-load', sharedlib, "-json-config=" + configfile, "-remove-dead-casting", mbitcodefile]
    retval = call(command, stdin=None, stdout=obitcode, stderr=None)

    # return -3 if removing dead castings fails
    if retval <> 0:
        return -3

    # We aren't using stats, so we don't need to run pass
    #command = ['opt', '-load', sharedlib, "-measure-metric", obitcodefile]
    #retval = call(command, stdin=None, stdout=temp, stderr=None)

    # ######################################
    # running the modified+optimized program
    # ######################################
    command = ['lli', obitcodefile, str(inx)]
    #command = ['lli', mbitcodefile]
    # retval = call(command, stdin=None, stdout=output, stderr=None)    
    try:
      retval = proc.run(command, timeout=30)
    except proc.Timeout:
      return -4 
    output.close()

    # return -2 if running the modified+optimized bitcode fails
    if retval <> 0:
        return -2

    # reading output
    output = open("sat_" + str(inx) + ".cov", 'r')
    firstline = output.readline()
    firstline = firstline.strip()
    if (firstline == "true"):
      # ##########################
      # get the dynamic score
      # #########################
      command = ['llc', obitcodefile, '-o', sys.argv[1] + "_" + str(inx) + '.s']
      call(command, stdin=None, stdout=None, stderr=None)
      command = ['gcc', sys.argv[1] + "_" + str(inx) + '.s', '-lm', '-o', sys.argv[1] + "_" + str(inx) + '.out']
      call(command, stdin=None, stdout=None, stderr=None)
      command = ['./' + sys.argv[1] + "_" + str(inx) + '.out']
      call(command, stdin=None, stdout=None, stderr=None)

      return 1
    else:
      return 0


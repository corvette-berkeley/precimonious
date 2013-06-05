#!/usr/bin/env python
#

import os
import platform
import sys
from subprocess import CalledProcessError, PIPE, Popen, call

coverage = set()
count = 0
fininfile = open('final_inputs', 'w')
for i in range(0, 10000):
  # remove inputs file
  command = ['rm', '-f', 'inputs']
  call(command, stdin=None, stdout=None, stderr=None)
  # remove coverage file
  command = ['rm', "-f", 'coverage.cov']
  call(command, stdin=None, stdout=None, stderr=None)
  # execute
  command = ['lli', 'c_' + sys.argv[1] + '.bc']
  retval = call(command, stdin=None, stdout=None, stderr=None)
  # reading coverage output
  if retval == 0:
    covstr = ""
    with open ('coverage.cov', 'r') as myfile:
      covstr = myfile.read().replace('\n', '')
    if covstr not in coverage:
      coverage.add(covstr)
      count = count+1
      print i
      infile = open('inputs', 'r')
      firstline = infile.readline()
      firstline = infile.readline()
      firstline = firstline.strip()
      infile.close()
      fininfile.write(firstline + '\n')
      if count == 10:
        break
fininfile.close()

#!/usr/bin/env python
#

import sys
from subprocess import CalledProcessError, PIPE, Popen, call

def calculate_speedup(configs, dir):
  # calculating speedup
  infile = open(dir + '/score.cov', 'r')
  original = int(infile.readline())
  infile.close()

  for i in range(1,configs+1):
    infile = open(dir + '/score' + str(i) + '.cov', 'r')
    modified = int(infile.readline())
    diff = original - modified
    speedup = (diff*100.0)/original
    print 'Speedup ' + str(i) + ':' + str(speedup)
    infile.close()


def main():
  dir = sys.argv[3]

  # running original
  command = [dir + '/' + sys.argv[1] + '_timers']
  call(command, stdin=None, stdout=None, stderr=None)
  command = ['cat', dir + '/score.cov']
  call(command, stdin=None, stdout=None, stderr=None)

  # running configurations
  n = int(sys.argv[2])
  for i in range(1,n+1):
    command = [dir + '/config' + str(i) + '_timers']
    call(command, stdin=None, stdout=None, stderr=None)
    command = ['cat', dir + '/score' + str(i) + '.cov']
    call(command, stdin=None, stdout=None, stderr=None)

  calculate_speedup(n, dir)


if __name__ == "__main__":
    main()



#!/usr/bin/env python

import types, sys, os, math, json
import transform_par, utilities
import math
import multiprocessing
from multiprocessing import Process, Queue

#
# global search counter
#
search_counter = 0
CPU_NO = 6

def run_delta(delta_change, delta_type, change_set, type_set, search_config,
    original_config, bitcode, original_score, inx, queue):
  if len(delta_change) > 0:
    # always reset to lowest precision
    utilities.to_2nd_highest_precision(change_set, type_set)
    # apply change for variables in delta
    utilities.to_highest_precision(delta_change, delta_type)
    # run config
    result = utilities.run_config(search_config, original_config, bitcode, inx)
    score = utilities.get_dynamic_score(inx)
    if result == 1 and score < original_score:
      queue.put([inx, score])

def dd_search_config(change_set, type_set, search_config, original_config, bitcode, original_score, div):
  global search_counter
  global CPU_NO
  #
  # partition change_set into deltas and delta inverses
  #
  delta_change_set = []
  delta_type_set = []
  delta_inv_change_set = []
  delta_inv_type_set = []
  div_size = int(math.ceil(float(len(change_set))/float(div)))
  for i in xrange(0, len(change_set), div_size):
    delta_change = []
    delta_type = []
    delta_inv_change = []
    delta_inv_type = []
    for j in xrange(0, len(change_set)):
      if j >= i and j < i+div_size:
        delta_change.append(change_set[j])
        delta_type.append(type_set[j])
      else:
        delta_inv_change.append(change_set[j])
        delta_inv_type.append(type_set[j])
    delta_change_set.append(delta_change)
    delta_type_set.append(delta_type)
    delta_inv_change_set.append(delta_inv_change)
    delta_inv_type_set.append(delta_inv_type)

  min_score = -1
  #
  # iterate through all delta and inverse delta set
  # record delta set that passes
  #
  pass_inx = -1
  inv_is_better = False

  #
  # iterate through all deltas and delta inverses
  # run in parallel
  #
  start_inx = search_counter
  queue = Queue()
  inv_queue = Queue()
  workers = []
  for i in xrange(0, len(delta_change_set)):
    workers.append(Process(target=run_delta, args=(delta_change_set[i],
    delta_type_set[i], change_set, type_set, search_config, original_config, bitcode,
    original_score, start_inx+i, queue))) 

  for i in xrange(0, len(delta_inv_change_set)):
    workers.append(Process(target=run_delta, args=(delta_inv_change_set[i],
    delta_inv_type_set[i], change_set, type_set, search_config, original_config, bitcode,
    original_score, start_inx+len(delta_change_set)+i, inv_queue))) 

  # run in parallel maximum cpu_no processes at a time
  cpu_no = CPU_NO 
  #multiprocessing.cpu_count()
  # cpu_no = 1
  loop = int(len(workers)/cpu_no)
  for i in xrange(0, loop):
    for j in xrange(i*cpu_no, min((i+1)*cpu_no, len(workers))):
      workers[j].start()
    for j in xrange(i*cpu_no, min((i+1)*cpu_no, len(workers))):
      workers[j].join()

  while not queue.empty():
    result = queue.get()
    if min_score == -1 or result[1] < min_score:
      pass_inx = result[0] - start_inx
      inv_is_better = False

  while not inv_queue.empty():
    result = inv_queue.get()
    if min_score == -1 or result[1] < min_score:
      pass_inx = result[0] - start_inx - len(delta_change_set)
      inv_is_better = True

  search_counter += len(delta_change_set) + len(delta_inv_change_set)

  #
  # recursively search in pass delta or pass delta inverse
  # right now keep searching for the first pass delta or
  # pass delta inverse; later on we will integrate cost
  # model here
  #
  if pass_inx != -1:
    pass_change_set = delta_inv_change_set[pass_inx] if inv_is_better else delta_change_set[pass_inx]
    pass_type_set = delta_inv_type_set[pass_inx] if inv_is_better else delta_type_set[pass_inx]

    if len(pass_change_set) > 1:
      # always reset to lowest precision
      utilities.to_2nd_highest_precision(change_set, type_set)
      dd_search_config(pass_change_set, pass_type_set, search_config, original_config, bitcode, original_score, 2)
    else:
      utilities.to_2nd_highest_precision(change_set, type_set)
      utilities.to_highest_precision(pass_change_set, pass_type_set)
    return

  #
  # stop searching when division greater than change set size
  #
  if div >= len(change_set):
    utilities.to_highest_precision(change_set, type_set)
    return
  else:
    dd_search_config(change_set, type_set, search_config, original_config, bitcode, original_score, 2*div)
    return

def search_config(change_set, type_set, search_config, original_config, bitcode, original_score):
  global search_counter
  # search from bottom up
  utilities.to_2nd_highest_precision(change_set, type_set)
  if utilities.run_config(search_config, original_config, bitcode, search_counter) != 1 or utilities.get_dynamic_score(search_counter) > original_score: 
    search_counter = search_counter + 1
    dd_search_config(change_set, type_set, search_config, original_config, bitcode, original_score, 2)
  else:
    search_counter = search_counter + 1
  # remove types that cannot be changed
  for i in xrange(0, len(change_set)):
    if len(type_set[i]) > 0 and change_set[i]["type"] == type_set[i][-1]:
      del(type_set[i][:])

  # remove highest precision from each type vector
  for type_vector in type_set:
    if len(type_vector) > 0:
      type_vector.pop()

#
# main function receives
#   - argv[1] : bitcode file location
#   - argv[2] : search file location
#   - argv[3] : original config file location
#
def main():
  global search_counter
  bitcode = sys.argv[1]
  search_conf_file = sys.argv[2]
  original_conf_file = sys.argv[3]

  #
  # delete log file if exists
  #
  try:
    os.remove("log.dd")
  except OSError:
    pass

  #
  # parsing config files
  #
  search_conf = json.loads(open(search_conf_file, 'r').read())
  original_conf = json.loads(open(original_conf_file, 'r').read())
  search_changes = search_conf["config"]
  change_set = []
  type_set = []

  #
  # record the change set
  #
  for search_change in search_changes:
    type_vector = search_change.values()[0]["type"]
    if isinstance(type_vector, list):
      type_set.append(type_vector)
      change_set.append(search_change.values()[0])

  #
  # search for valid configuration
  #
  print "Searching for valid configuration using delta-debugging algorithm ..."

  # get original score
  utilities.to_highest_precision(change_set, type_set)
  utilities.run_config(search_conf, original_conf, bitcode, search_counter)
  original_score = utilities.get_dynamic_score(search_counter) * 1.05
  search_counter = search_counter + 1

  # keep searching while the type set is not searched throughout
  while not utilities.is_empty(type_set):
    search_config(change_set, type_set, search_conf, original_conf, bitcode, original_score)

  # get the score of modified program
  utilities.run_config(search_conf, original_conf, bitcode, search_counter)
  modified_score = utilities.get_dynamic_score(search_counter)
  search_counter = search_counter + 1

  if modified_score <= original_score:
    print "Check valid_" + bitcode + ".json for the valid configuration file"
    # print valid configuration file and diff file
    utilities.print_config(search_conf, "dd2_valid_" + bitcode + ".json")
    utilities.print_diff(search_conf, original_conf, "dd2_diff_" + bitcode + ".json")
  else:
    print "No configuration is found!"

if __name__ == "__main__":
  main()

#!/usr/bin/env python

import types, sys, os, math, json
import transform_notime, utilities

#
# global search counter
#
search_counter = 0

#
# run bitcode file with the current
# search configuration
# return  1 bitcode file is valid
#         0 bitcode file is invalid
#         -1 some internal transformation error happens
#         -2 some internal transformation error happens
#         -3 some internal transformation error happens
#
def run_config(search_config, original_config, bitcode):
  global search_counter
  utilities.print_config(search_config, "config_temp.json")
  result = transform_notime.transform(bitcode, "config_temp.json")
  if result == 1:
    utilities.print_config(search_config, "VALID_config_" + bitcode + "_" + str(search_counter) + ".json")
    utilities.log_config(search_config, "VALID", "log.dd", search_counter)
    utilities.print_diff(search_config, original_config, "dd2_diff_" + bitcode + "_" + str(search_counter) + ".json")
  elif result == 0:
    utilities.print_config(search_config, "INVALID_config_" + bitcode + "_" + str(search_counter) + ".json")
    utilities.log_config(search_config, "INVALID", "log.dd", search_counter)
  elif result == -1:
    utilities.print_config(search_config, "FAIL1_config_" + bitcode + "_" + str(search_counter) + ".json")
    utilities.log_config(search_config, "FAIL1", "log.dd", search_counter)
  elif result == -2:
    utilities.print_config(search_config, "FAIL2_config_" + bitcode + "_" + str(search_counter) + ".json")
    utilities.log_config(search_config, "FAIL2", "log.dd", search_counter)
  elif result == -3:
    utilities.print_config(search_config, "FAIL3_config_" + bitcode + "_" + str(search_counter) + ".json")
    utilities.log_config(search_config, "FAIL3", "log.dd", search_counter)
  else:
    utilities.print_config(search_config, "FAIL4_config_" + bitcode + "_" + str(search_counter) + ".json")
    utilities.log_config(search_config, "FAIL4", "log.dd", search_counter)

  search_counter += 1
  return result

#
# modify change set so that each variable
# maps to its highest type
#
def to_highest_precision(change_set, type_set):
  for i in range(0, len(change_set)):
    c = change_set[i]
    t = type_set[i]
    if len(t) > 0:
      c["type"] = t[-1]

#
# modify change set so that each variable
# maps to its 2nd highest type
#
def to_2nd_highest_precision(change_set, type_set):
  for i in range(0, len(change_set)):
    c = change_set[i]
    t = type_set[i]
    if len(t) > 1:
      c["type"] = t[-2]

#
# check if we have search through all 
# types in type_set
#
def is_empty(type_set):
  for t in type_set:
    if len(t) > 1:
      return False
  return True

def dd_search_config(change_set, type_set, search_config, original_config, bitcode, div, original_score):
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

  #
  # iterate through all delta and inverse delta set
  # record delta set that passes
  #
  pass_inx = -1
  inv_is_better = False
  #min_score = -1

  for i in xrange(0, len(delta_change_set)):
    delta_change = delta_change_set[i]
    delta_type = delta_type_set[i]
    if len(delta_change) > 0:
      # always reset to lowest precision
      to_2nd_highest_precision(change_set, type_set)
      # apply change for variables in delta
      to_highest_precision(delta_change, delta_type)
      # record i if config passes
      if run_config(search_config, original_config, bitcode) == 1: 
        pass_inx = i
        inv_is_better = False
        break

    delta_inv_change = delta_inv_change_set[i]
    delta_inv_type = delta_inv_type_set[i]
    if len(delta_inv_change) > 0 and div > 2:
      # always reset to lowest precision
      to_2nd_highest_precision(change_set, type_set)
      # apply change for variables in delta inverse
      to_highest_precision(delta_inv_change, delta_inv_type)
      # record i if config passes
      if run_config(search_config, original_config, bitcode) == 1: 
        pass_inx = i
        inv_is_better = True
        break
        #  min_score = score

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
      to_2nd_highest_precision(change_set, type_set)
      dd_search_config(pass_change_set, pass_type_set, search_config, original_config, bitcode, 2, original_score)
    else:
      to_2nd_highest_precision(change_set, type_set)
      to_highest_precision(pass_change_set, pass_type_set)
    return

  #
  # stop searching when division greater than change set size
  #
  if div >= len(change_set):
    to_highest_precision(change_set, type_set)
    return
  else:
    dd_search_config(change_set, type_set, search_config, original_config, bitcode, 2*div, original_score)
    return

def search_config(change_set, type_set, search_config, original_config, bitcode, original_score):
  # search from bottom up
  to_2nd_highest_precision(change_set, type_set)
  if run_config(search_config, original_config, bitcode) != 1: 
    dd_search_config(change_set, type_set, search_config, original_config, bitcode, 2, original_score)
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
  original_score = -1

  # keep searching while the type set is not searched throughout
  while not is_empty(type_set):
    search_config(change_set, type_set, search_conf, original_conf, bitcode, original_score)

  #if modified_score <= original_score:
  print "Check valid_" + bitcode + ".json for the valid configuration file"
  # print valid configuration file and diff file
  utilities.print_config(search_conf, "dd2_valid_" + bitcode + ".json")
  utilities.print_diff(search_conf, original_conf, "dd2_diff_" + bitcode + ".json")

if __name__ == "__main__":
  main()

#!/usr/bin/env python

import json
import types
import sys
import os
import transform
import utilities

search = 0

def run_config(config, bitcodeFile, searchConfig, originalConfig):
  global search
  utilities.print_config(config, "config_temp.json")
  result = transform.transform(bitcodeFile, "config_temp.json")
  if result == 1:
    utilities.print_config(config, "VALID_config_" + bitcodeFile + "_" + str(search) + ".json")
    utilities.log_config(config, "VALID", "log.dd", search)
    utilities.print_diff(searchConfig, originalConfig, "diff_" + bitcodeFile + "_" + str(search) + ".json")
  elif result == 0:
    utilities.print_config(config, "INVALID_config_" + bitcodeFile + "_" + str(search) + ".json")
    utilities.log_config(config, "INVALID", "log.dd", search)
  elif result == -1:
    utilities.print_config(config, "FAIL1_config_" + bitcodeFile + "_" + str(search) + ".json")
    utilities.log_config(config, "FAIL1", "log.dd", search)
  elif result == -2:
    utilities.print_config(config, "FAIL2_config_" + bitcodeFile + "_" + str(search) + ".json")
    utilities.log_config(config, "FAIL2", "log.dd", search)
  elif result == -3:
    utilities.print_config(config, "FAIL3_config_" + bitcodeFile + "_" + str(search) + ".json")
    utilities.log_config(config, "FAIL3", "log.dd", search)

  search += 1
  return result

def to_highest_precision(changeSet, typeSet):
  for i in range(0, len(changeSet)):
    c = changeSet[i]
    t = typeSet[i]
    if len(t) > 0:
      c["type"] = t[-1]

def to_2nd_highest_precision(changeSet, typeSet):
  for i in range(0, len(changeSet)):
    c = changeSet[i]
    t = typeSet[i]
    if len(t) > 1:
      c["type"] = t[-2]

def is_empty(typeSet):
  for t in typeSet:
    if len(t) > 1:
      return False
  return True

def db_search_config(changeSet, typeSet, config, bitcodeFile, searchConfig, originalConfig):
  failureChange = []
  if len(changeSet) == 1:
    failureChange.append([changeSet[0], typeSet[0]])
    return failureChange

  # split changeSet and typeSet into two halves
  half = int(len(changeSet)/2)
  changeSet01 = []
  changeSet02 = []
  typeSet01 = []
  typeSet02 = []
  for i in range(0, len(changeSet)):
    if i < half:
      changeSet01.append(changeSet[i])
      typeSet01.append(typeSet[i])
    else:
      changeSet02.append(changeSet[i])
      typeSet02.append(typeSet[i])
  # first half test
  to_highest_precision(changeSet, typeSet)
  to_2nd_highest_precision(changeSet01, typeSet01)
  result01 = run_config(config, bitcodeFile, searchConfig, originalConfig)
  if result01 != 1:
    failureChange.extend(db_search_config(changeSet01, typeSet01, config, bitcodeFile, searchConfig, originalConfig))
  # second half test
  to_highest_precision(changeSet, typeSet)
  to_2nd_highest_precision(changeSet02, typeSet02)
  result02 = run_config(config, bitcodeFile, searchConfig, originalConfig)
  if result02 != 1:
    failureChange.extend(db_search_config(changeSet02, typeSet02, config, bitcodeFile, searchConfig, originalConfig))
  # third test
  if result01 == 1 and result02 == 1:
    failureChange01 = db_search_config(changeSet01, typeSet01, config, bitcodeFile, searchConfig, originalConfig)
    for failure in failureChange01:
      to_highest_precision(changeSet, typeSet)
      if isinstance(failure[0], list):
        for failChange in failure:
          failChange[0]["type"] = failChange[1][-2]
      else:
        failure[0]["type"] = failure[1][-2]
      failureChange02 = db_search_config(changeSet02, typeSet02, config, bitcodeFile, searchConfig, originalConfig)
      for failure2 in failureChange02:
        if isinstance(failure[0], list) and isinstance(failure2[0], list):
          failure.extend(failure2)
          failureChange.append(failure)
        elif isinstance(failure[0], list):
          failure.append(failure2)
          failureChange.append(failure)
        elif isinstance(failure2[0], list):
          failure2.append(failure)
          failureChange.append(failure2)
        else:
          failurePair = []
          failurePair.append(failure)
          failurePair.append(failure2)
          failureChange.append(failurePair)

  return failureChange

def search_config(changeSet, typeSet, config, bitcodeFile, searchConfig, originalConfig):
  # to 2nd highest precision
  to_2nd_highest_precision(changeSet, typeSet)
  # run config
  if run_config(config, bitcodeFile, searchConfig, originalConfig) != 1:
    # run delta-debugging algorithm
    failureChange = db_search_config(changeSet, typeSet, config, bitcodeFile, searchConfig, originalConfig) 
    # create valid config
    to_2nd_highest_precision(changeSet, typeSet)
    # failure change need to be in highest precision
    for failure in failureChange:
      if isinstance(failure[0], list):
        failureChange = failure[0]
        failureChange[0]["type"] = failureChange[1][-1]
        # remove all type choices for these changes
        del(failureChange[1][:])
      else:
        failure[0]["type"] = failure[1][-1]
        # remove all type choices for these changes
        del(failure[1][:])

  # remove highest precision from type list
  for t in typeSet:
    if len(t) > 0:
      t.pop()

def main():
  bitcodeFile = sys.argv[1]
  searchConfigFile = sys.argv[2]
  originalConfigFile = sys.argv[3]

  # delete log file if exists
  try:
    os.remove("log.dd")
  except OSError:
    pass

  searchConfig = json.loads(open(searchConfigFile, 'r').read())
  originalConfig = json.loads(open(originalConfigFile, 'r').read())
  changes = searchConfig["config"]
  changeSet = []
  typeSet = []

  for change in changes:
    typeList = change.values()[0]["type"]
    if isinstance(typeList, list):
      typeSet.append(typeList)
      changeSet.append(change.values()[0])

  print "Searching for valid config using delta-debugging algorithm..."
  to_highest_precision(changeSet, typeSet)
  while not is_empty(typeSet):
    search_config(changeSet, typeSet, searchConfig, bitcodeFile, searchConfig, originalConfig)
  print "Check valid_" + bitcodeFile + ".json for the valid config file"
  utilities.print_config(searchConfig, "valid_" + bitcodeFile + ".json")
  utilities.print_diff(searchConfig, originalConfig, "diff_" + bitcodeFile + ".json")

if __name__ == "__main__":
  main()




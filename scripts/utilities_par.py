#!/usr/bin/env python

import json
import types
import sys
import transform_par

def get_dynamic_score(inx):
  scorefile = open("score_" + str(inx) + ".cov")
  score = scorefile.readline()
  score = score.strip()
  return int(score)

def log_fast_config(logfile, counter, score):
  f = open(logfile, "a")
  f.write(str(counter) + ": " + str(score))
  f.write("\n")

def log_config(config, msg, logfile, counter):
  f = open(logfile, "a")
  f.write(str(counter) + ". ")
  changeSet = config["config"]
  for change in changeSet:
    changeKey = change.keys()[0]
    t = change.values()[0]["type"]
    log = False
    if t == "float":
      t = "f"
      log = True
    elif t == "double":
      t = "d"
      log = True
    elif t == "longdouble":
      t = "ld"
      log = True
    if log:	
      if changeKey == "localVar" or changeKey == "globalVar":
        name = change.values()[0]["name"]
        f.write(name + ":" + t + " ")
      elif changeKey == "op":
        name = change.values()[0]["id"]
        f.write(name + ":" + t + " ")
  f.write(": " + msg)
  f.write("\n")

def print_config(config, configFile):
  f = open(configFile, 'w+')
  f.write("{\n")
  changeList = config["config"]
  for change in changeList:
    f.write("\t\"" + change.keys()[0] + "\": {\n")
    changeValue = change.values()[0]
    for valueInfo in changeValue.keys():
      f.write("\t\t\"" + valueInfo + "\": \"" + changeValue[valueInfo] + "\",\n")
    f.write("\t},\n")
  f.write("}\n")

def print_diff(changeConfig, originalConfig, diffFile):
  f = open(diffFile, 'w+')
  originalList = originalConfig["config"]
  changeList = changeConfig["config"]
  count = 0
  while count < len(originalList):
    change = changeList[count]
    origin = originalList[count]
    newType = change.values()[0]["type"]
    originType = origin.values()[0]["type"]
    if newType != originType:
      changeType = change.keys()[0]
      if changeType == "localVar":
        function = change.values()[0]["function"]
        if change.values()[0].has_key("file"):
          fileName = change.values()[0]["file"]
          f.write("localVar: " + change.values()[0]["name"] + "  at " + function + " at " + fileName + " " + originType + " -> " + newType + "\n")
        else:
          f.write("localVar: " + change.values()[0]["name"] + "  at " + function + " " + originType + " -> " + newType + "\n")
      elif changeType == "op": 
        fileName = change.values()[0]["file"]
        function = change.values()[0]["function"]
        f.write("op: " + change.values()[0]["id"] + " at " + function + " at " + fileName + " " + originType + " -> " + newType + "\n")
      elif changeType == "globalVar":
        f.write("globalVar: " + change.values()[0]["name"] + " " + originType + " -> " + newType + "\n")
    count += 1

#
# check if we have search through all 
# types in type_set
#
def is_empty(type_set):
  for t in type_set:
    if len(t) > 1:
      return False
  return True


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
# run bitcode file with the current
# search configuration
# return  1 bitcode file is valid
#         0 bitcode file is invalid
#         -1 some internal transformation error happens
#         -2 some internal transformation error happens
#         -3 some internal transformation error happens
#
def run_config(search_config, original_config, bitcode, inx):
  print_config(search_config, "config_temp_" + str(inx) + ".json")
  result = transform_par.transform(bitcode, "config_temp_" + str(inx) + ".json", inx)
  if result == 1:
    print_config(search_config, "VALID_config_" + bitcode + "_" +
        str(inx) + ".json")
    log_config(search_config, "VALID", "log.dd", inx)
    print_diff(search_config, original_config, "dd2_diff_" + bitcode
        + "_" + str(inx) + ".json")
  elif result == 0:
    print_config(search_config, "INVALID_config_" + bitcode + "_" +
        str(inx) + ".json")
    log_config(search_config, "INVALID", "log.dd", inx)
  elif result == -1:
    print_config(search_config, "FAIL1_config_" + bitcode + "_" +
        str(inx) + ".json")
    log_config(search_config, "FAIL1", "log.dd", inx)
  elif result == -2:
    print_config(search_config, "FAIL2_config_" + bitcode + "_" +
        str(inx) + ".json")
    log_config(search_config, "FAIL2", "log.dd", inx)
  elif result == -3:
    print_config(search_config, "FAIL3_config_" + bitcode + "_" +
        str(inx) + ".json")
    log_config(search_config, "FAIL3", "log.dd", inx)
  else:
    print_config(search_config, "FAIL4_config_" + bitcode + "_" +
        str(inx) + ".json")
    log_config(search_config, "FAIL4", "log.dd", inx)

  return result


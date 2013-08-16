#!/usr/bin/env python

import json
import os
import types
import sys
import transform
import utilities

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
				f.write("localVar: " + change.values()[0]["name"] + " " + originType + " -> " + newType)
			elif changeType == "op": 
				f.write("op: " + change.values()[0]["id"] + originType + " -> " + newType)
		count += 1

def run_config(config, search, bitcodefile, originalConfig, limit):
	print_config(config, "config_temp.json")
	result = transform.transform(bitcodefile, "config_temp.json")
	if result == 1:
		print "check VALID_config_" + str(search) + ".json for a valid config file"
		print_config(config, "VALID_config_" + bitcodefile + "_" + str(search) + ".json")
		print_diff(config, originalConfig, "diff_" + str(search) + ".cov")
		utilities.log_config(config, "VALID", "log.bf", search)
	elif result == 0:
		print "\tINVALID CONFIG"
		print_config(config, "INVALID_config_" + bitcodefile + "_" + str(search) + ".json")
		utilities.log_config(config, "INVALID", "log.bf", search)
	elif result == -1:
		print "\tFAIL TYPE 1"
		print_config(config, "FAIL1_config_" + bitcodefile + "_" + str(search) + ".json")
		utilities.log_config(config, "FAIL1", "log.bf", search)
	elif result == -2:
		print "\tFAIL TYPE 2"
		print_config(config, "FAIL2_config_" + bitcodefile + "_" + str(search) + ".json")
		utilities.log_config(config, "FAIL2", "log.bf", search)
	elif result == -3:
		print "\tFAIL TYPE 3"
		print_config(config, "FAIL3_config_" + bitcodefile + "_" + str(search) + ".json")
		utilities.log_config(config, "FAIL3", "log.bf", search)
	search += 1
	if search > limit and limit != -1:
		sys.exit(0)
	return search


def search_config(changeList, changeTypeList, config, search, bitcodefile, originalConfig, limit):
	if len(changeList) == 1:
		for changeType in changeTypeList[0]:
			changeList[0]["type"] = changeType
			search = run_config(config, search, bitcodefile, originalConfig, limit)
	else:
		change = changeList.pop()
		changeTypes = changeTypeList.pop()
		for changeType in changeTypes:
			change["type"] = changeType
			search = search_config(changeList, changeTypeList, config, search, bitcodefile, originalConfig, limit)
		changeList.append(change)
		changeTypeList.append(changeTypes)
	return search

def main():
	bitcodefile = sys.argv[1]
	configSearch = sys.argv[2]
	original = sys.argv[3]
	limit = -1
	if len(sys.argv) >= 5:
		limit = int(sys.argv[4])

	# remove config file
	try:
		os.remove("log.bf")
	except OSError:
		pass

	config = json.loads(open(configSearch, 'r').read())
	originalConfig = json.loads(open(original, 'r').read())
	allChange = config["config"]
	allChangeList = []
	allTypeList = []

	for change in allChange:
		typeList = change.values()[0]["type"]
		if isinstance(typeList, list):
			allTypeList.append(typeList)
			allChangeList.append(change.values()[0])

	print "Searching for valid config ..."
	search_config(allChangeList, allTypeList, config, 0, bitcodefile, originalConfig, limit)

if __name__ == "__main__":
	main()




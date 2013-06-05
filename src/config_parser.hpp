#ifndef CONFIGPARSER_GUARD
#define CONFIGPARSER_GUARD 1

#include "vjson/json.h"
#include "vjson/block_allocator.h"
#include <map>
#include <string>
#include <utility>

using namespace std;

map<string, pair<string, string> > parse_config(const char *filename);

#endif

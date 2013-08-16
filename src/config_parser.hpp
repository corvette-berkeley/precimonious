#ifndef CONFIGPARSER_GUARD
#define CONFIGPARSER_GUARD 1

#include "vjson/json.h"
#include "vjson/block_allocator.h"
#include <map>
#include <string>
#include <utility>
#include "StrChange.hpp"
#include "FuncStrChange.hpp"

using namespace std;

map<string, StrChange*> parse_config(const char *filename);

#endif

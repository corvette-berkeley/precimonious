#include "config_parser.hpp"
#include "vjson/json.h"
#include "vjson/block_allocator.h"
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/raw_ostream.h>
#include <map>
#include <utility>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

using namespace std;
using namespace llvm;

void parse_array(char* type, json_value *arr, const char* del) {
	int cnt;
	cnt = 0;
	for (json_value *el = arr->first_child; el; el = el->next_sibling) {
		if (cnt > 0) strcat(type, del);
		if (el->type == JSON_ARRAY) {
			parse_array(type, el, " ");
		} else {
			strcat(type, el->string_value);
		}
		cnt++;
	}
}

// parse call 
void parse_call(json_value *call, map<string, StrChange*> &changes) {
	char type[1000] = {'\0'};
	char name[100] = {'\0'}; 
	char function[100] = {'\0'};
	char id[100] = {'\0'};
	char file[100] = {'\0'};
	char line[100] = {'\0'};
  char swit[100] = {'\0'};

	for (json_value *child = call->first_child; child; child = child->next_sibling) {
		if (strcmp(child->name, "id") == 0) {
			strcpy(id, child->string_value);
		} else if (strcmp(child->name, "file") == 0) {
			strcpy(file, child->string_value);
		} else if (strcmp(child->name, "function") == 0) {
			strcpy(function, child->string_value);
		} else if (strcmp(child->name, "line") == 0) {
			strcpy(line, child->string_value);
		} else if (strcmp(child->name, "name") == 0) {
			strcpy(name, child->string_value);
		} else if (strcmp(child->name, "switch") == 0) {
			strcpy(swit, child->string_value);
		} else if (strcmp(child->name, "type") == 0) {
			if (child->first_child->type == JSON_ARRAY) {
				parse_array(type, child, ", ");
			} else {
				parse_array(type, child, " ");
			}
		}
	}

	string idStr(id);
	FuncStrChange *change = new FuncStrChange("call", string(type), -1, string(swit));
	changes[idStr] = change;
}

// parse op
void parse_op(json_value *op, map<string, StrChange*> &changes) {
	char type[100] = {'\0'};
	char name[100] = {'\0'}; 
	char function[100] = {'\0'};
	char id[100] = {'\0'};
	char file[100] = {'\0'};
	char line[100] = {'\0'};

	for (json_value *child = op->first_child; child; child = child->next_sibling) {
		if (strcmp(child->name, "id") == 0) {
			strcpy(id, child->string_value);
		} else if (strcmp(child->name, "file") == 0) {
			strcpy(file, child->string_value);
		} else if (strcmp(child->name, "function") == 0) {
			strcpy(function, child->string_value);
		} else if (strcmp(child->name, "line") == 0) {
			strcpy(line, child->string_value);
		} else if (strcmp(child->name, "name") == 0) {
			strcpy(name, child->string_value);
		} else if (strcmp(child->name, "type") == 0) {
			if (child->type == JSON_ARRAY) {
				parse_array(type, child, ", ");
			} else {
				strcpy(type, child->string_value);
			}
		}
	}

	string idStr(id);
	StrChange *change = new StrChange("op", string(type), -1);
	changes[idStr] = change;
}

// parse localVar
void parse_local_var(json_value *localVar, map<string, StrChange*> &changes) {
	char type[100] = {'\0'};
	char id[100] = {'\0'};
	char name[100] = {'\0'}; 
	char function[100] = {'\0'};
	char field[2] = {'\0'};
	int iField = -1;

	for (json_value *child = localVar->first_child; child; child = child->next_sibling) {
		if (strcmp(child->name, "name") == 0) {
			strcpy(name, child->string_value);
		} else if (strcmp(child->name, "function") == 0) {
			strcpy(function, child->string_value);
		} else if (strcmp(child->name, "field") == 0) {
		        strcpy(field, child->string_value);
			iField = atoi(field);
		} else if (strcmp(child->name, "type") == 0) {
			if (child->type == JSON_ARRAY) {
				parse_array(type, child, ", ");
			} else {
				strcpy(type, child->string_value);
			}
		} 
	}

	strcpy(id, name);
	strcat(id, "@");
	strcat(id, function);
	string idStr(id);

	StrChange *change = new StrChange("localVar", string(type), iField);
	changes[idStr] = change;
}

// parse globalVar
void parse_global_var(json_value *globalVar, map<string, StrChange*> &changes) {
	char type[100] = {'\0'};
	char name[100] = {'\0'}; 

	for (json_value *child = globalVar->first_child; child; child = child->next_sibling) {
		if (strcmp(child->name, "name") == 0) {
			strcpy(name, child->string_value);
		} else if (strcmp(child->name, "type") == 0) {
			if (child->type == JSON_ARRAY) {
				parse_array(type, child, ", ");
			} else {
				strcpy(type, child->string_value);
			}
		} 
	}

	string idStr(name);
	StrChange *change = new StrChange("globalVar", string(type), -1);
	changes[idStr] = change;
}

// parse json
void parse_json(json_value *root, map<string, StrChange*> &changes) {
	for (json_value *child = root->first_child; child; child = child->next_sibling) {
		if (strcmp(child->name, "globalVar") == 0) {
			parse_global_var(child, changes);
		} else if (strcmp(child->name, "localVar") == 0) {
			parse_local_var(child, changes);
		} else if (strcmp(child->name, "op") == 0) {
			parse_op(child, changes);
		} else if (strcmp(child->name, "call") == 0) {
			parse_call(child, changes);
		}
	}
}

// parse json
map<string, StrChange*> parse_config(const char* filename) {
	// initialize objects used by vjson
	map<string, StrChange*> changes;
	block_allocator mAllocator(1 << 10); 
	json_value *root;

	// open config file
	FILE *fp = fopen(filename, "rb");

	// get config file source
	fseek(fp, 0, SEEK_END);
	int size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	char *source = (char *)mAllocator.malloc(size + 1);
	size_t r = fread(source, 1, size, fp);
	printf("%ld\n", r);
	source[size] = 0;

	fclose(fp);

	// parse the source
	char *errorPos = 0;
	int errorLine = 0;
	root = json_parse(source, &errorPos, &errorLine, &mAllocator);

	if (!root)
	{
		errs() << "Error parsing config file";
	} else {
		parse_json(root, changes);
	}

	return changes;
} 

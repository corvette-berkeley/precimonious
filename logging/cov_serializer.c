#include <stdio.h>
#include "cov_serializer.h"

void cov_serialize(long double f, unsigned char* buf, int length) {
	int i;
	union {
		unsigned char bytes[10];
		long double val;
	} bfconvert;

	bfconvert.val = f;

	for (i = 0; i < length; i++) {
		buf[i] = bfconvert.bytes[i];
	}
}

long double cov_deserialize(char* buf, int length) {
	int i;
	unsigned int u;
	union {
		unsigned char bytes[10];
		long double val;
	} bfconvert;

	for (i = 0; i < length; i++) {
		sscanf(buf, "%02X", &u);
		bfconvert.bytes[i] = u;
		buf += 2;
	}

	return bfconvert.val;
}


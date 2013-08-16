
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void cov_serialize(long double, unsigned char*, int);
long double cov_deserialize(char*, int);
void cov_log(long double, char*, char*);
void cov_spec_log(long double, long double, char*);
void cov_arr_log(long double[], int, char*, char*);
void cov_spec_arr_log(long double[], int, long double, char*);
void cov_check(char*, char*, char*);
void cov_arr_check(char*, char*, int length);

float *a;

int main(int argc, char *argv[]) {
  int i;

  a = (float*) malloc(sizeof(float)*8);
  a[0] = 2.0;
  a[1] = 3.0;
  for (i = 2; i < 8; i++)
  {
    a[i] = a[i-1] + a[i-2];
  }

  cov_spec_log((long double) a[7], 0, argv[1]);
//  cov_log(a[7], "result", argv[2]); // log.cov
//  cov_check(argv[2], argv[1], argv[3]); // log.cov, spec.cov, result.out

  return 0;
}

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


void cov_log(long double ld, char* msg, char* fn) {
	int ld_size, i;
	unsigned char* buf;
	FILE* file;

	file = fopen(fn, "a");
	ld_size = 10;
	buf = malloc(ld_size);

	if (strlen(msg) > 0) fprintf(file, "#%s\n", msg);
	cov_serialize(ld, buf, ld_size);
	for (i = 0; i < ld_size; i++) {
		fprintf(file, "%02X", buf[i]);
	}
	fprintf(file, "\n");
	fclose(file);
}

void cov_spec_log(long double ideal, long double delta, char* fn) {
	cov_log(ideal, "ideal", fn);
	cov_log(delta, "delta", fn);
}

void cov_arr_log(long double lds[], int size, char* msg, char* fn) {
	int ld_size, i, j;
	unsigned char* buf;
	FILE *file; 

	file = fopen(fn, "a");
	ld_size = 10;
	buf = malloc(ld_size);

	if (strlen(msg) > 0) fprintf(file, "#%s\n", msg);
	for (i = 0; i < size; i++) {
		long double ld = lds[i];
		cov_serialize(ld, buf, ld_size);
		if (i > 0) fprintf(file, " ");
		for (j = 0; j < ld_size; j++) {
			fprintf(file, "%02X", buf[j]);
		}
	}
	fprintf(file, "\n");
	fclose(file);
}

void cov_spec_arr_log(long double ideal[], int size, long double delta, char* fn) {
	cov_arr_log(ideal, size, "ideal", fn);
	cov_log(delta, "delta", fn);
}


void cov_check(char* log, char* spec, char* result) {
	char line[80], estimate_hex[80], ideal_hex[80], delta_hex[80];
	long double estimate, ideal, delta, diff;
	FILE *log_file, *spec_file, *result_file;
	int ld_size;

	ld_size = 10;

	// reading log file
	log_file = fopen(log, "rt");
	while (fgets(line, 80, log_file) != NULL) {
		if (line[0] != '#') strcpy(estimate_hex, line);
	}
	fclose(log_file);

	spec_file = fopen(spec, "rt");
	while (fgets(line, 80, spec_file) != NULL) {
		if (strncmp(line, "#ideal", 6) == 0) {
			fgets(ideal_hex, 80, spec_file);
		} else if (strncmp(line, "#delta", 6) == 0) {
			fgets(delta_hex, 80, spec_file);
		}
	}
	fclose(spec_file);

	// compare log and spec results
	estimate = cov_deserialize(estimate_hex, ld_size); 
	ideal = cov_deserialize(ideal_hex, ld_size);
	delta = cov_deserialize(delta_hex, ld_size);

	if (estimate > ideal) diff = estimate - ideal;
	else diff = ideal - estimate;

	result_file = fopen(result, "w");
	if (delta >= diff) fprintf(result_file, "true\n");
	else fprintf(result_file, "false\n");
	fclose(result_file);
}

void cov_arr_check(char* log, char* spec, int length) {
	char line[1000];
	char *word, *sep, *brkt, *brkb;
	long double estimate[length], ideal[length], delta;
	FILE *log_file, *spec_file;
	int ld_size, cnt, i, predicate;

	sep = " ";
	ld_size = 10;

	// reading log file
	log_file = fopen(log, "rt");
	while (fgets(line, 1000, log_file) != NULL) {
		if (line[0] != '#') {
			cnt = 0;
			for (word = strtok_r(line, sep, &brkt); 
					word; 
					word = strtok_r(NULL, sep, &brkt)){
				estimate[cnt] = cov_deserialize(word, ld_size);
				cnt++;
			}
		}
	}
	fclose(log_file);

	// reading spec file
	spec_file = fopen(spec, "rt");
	while (fgets(line, 1000, log_file) != NULL) {
		if (strncmp(line, "#ideal", 6) == 0) {
			cnt = 0;
			fgets(line, 1000, spec_file);
			for (word = strtok_r(line, sep, &brkb); 
					word; 
					word = strtok_r(NULL, sep, &brkb)){
				ideal[cnt] = cov_deserialize(word, ld_size);
				cnt++;
			}
		} else if (strncmp(line, "#delta", 6) == 0) {
			fgets(line, 80, spec_file);
			delta = cov_deserialize(line, ld_size);
		}
	}
	fclose(spec_file);

	predicate = 1;
	for (i = 0; i < length; i++) {
		long double diff;
		if (estimate[i] > ideal[i]) diff = estimate[i] - ideal[i];
		else diff = ideal[i] - estimate[i];
		if (delta < diff) {
			predicate = 0;
			break;
		}
	}

	if (predicate) printf("true\n");
	else printf("false\n");
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>
#include "cov_checker.h"
#include "cov_serializer.h"

void cov_check(char* log, char* spec, int length) {
  int characters = 1000000;
	char line[characters];
	char *word, *sep, *brkt, *brkb;
	long double estimate[length], ideal[length], delta;
	FILE *log_file, *spec_file;
	int ld_size, cnt, i, predicate;

	sep = " ";
	ld_size = 10;

	// reading log file
	log_file = fopen(log, "rt");
	while (fgets(line, characters, log_file) != NULL) {
		if (line[0] != '#') {
			cnt = 0;
			for (word = strtok_r(line, sep, &brkt); 
					word; 
					word = strtok_r(NULL, sep, &brkt)){
        if (cnt < length) {
          estimate[cnt] = cov_deserialize(word, ld_size);
          cnt++;
        }
      }
    }
	}
	fclose(log_file);

	// reading spec file
	spec_file = fopen(spec, "rt");
	while (fgets(line, characters, spec_file) != NULL) {
		if (strncmp(line, "#ideal", 6) == 0) {
			cnt = 0;
			fgets(line, characters, spec_file);
			for (word = strtok_r(line, sep, &brkb); 
					word; 
					word = strtok_r(NULL, sep, &brkb)){
        if (cnt < length) {
          ideal[cnt] = cov_deserialize(word, ld_size);
          cnt++;
        }
			}
		} else if (strncmp(line, "#delta", 6) == 0) {
			fgets(line, 80, spec_file);
			delta = cov_deserialize(line, ld_size);
		}
	}
	fclose(spec_file);

	predicate = 1;
	for (i = 0; i < length; i++) {
    if (isnan(estimate[i]) || isinf(estimate[i]) )
    {
      predicate = 0;
      break;
    }
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

  // print result to a file
  FILE *result_file;
  result_file = fopen("sat.cov", "w");

  if (predicate) fprintf(result_file, "true\n"); 
  else fprintf(result_file, "false\n");
}

void cov_check_(int* length_pointer) {
  int length = *length_pointer;
  int characters = 1000000;
	char line[characters];
	char *word, *sep, *brkt, *brkb;
	long double estimate[length], ideal[length], delta;
	FILE *log_file, *spec_file;
	int ld_size, cnt, i, predicate;

	sep = " ";
	ld_size = 10;

	// reading log file
	log_file = fopen("log.cov", "rt");
	while (fgets(line, characters, log_file) != NULL) {
		if (line[0] != '#') {
			cnt = 0;
			for (word = strtok_r(line, sep, &brkt); 
					word; 
					word = strtok_r(NULL, sep, &brkt)){
        if (cnt < length) {
          estimate[cnt] = cov_deserialize(word, ld_size);
          cnt++;
        }
      }
    }
	}
	fclose(log_file);

	// reading spec file
	spec_file = fopen("spec.cov", "rt");
	while (fgets(line, characters, spec_file) != NULL) {
		if (strncmp(line, "#ideal", 6) == 0) {
			cnt = 0;
			fgets(line, characters, spec_file);
			for (word = strtok_r(line, sep, &brkb); 
					word; 
					word = strtok_r(NULL, sep, &brkb)){
        if (cnt < length) {
          ideal[cnt] = cov_deserialize(word, ld_size);
          cnt++;
        }
			}
		} else if (strncmp(line, "#delta", 6) == 0) {
			fgets(line, 80, spec_file);
			delta = cov_deserialize(line, ld_size);
		}
	}
	fclose(spec_file);

	predicate = 1;
	for (i = 0; i < length; i++) {
    if (isnan(estimate[i]) || isinf(estimate[i]) )
    {
      predicate = 0;
      break;
    }
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

  // print result to a file
  FILE *result_file;
  result_file = fopen("sat.cov", "w");

  if (predicate) fprintf(result_file, "true\n"); 
  else fprintf(result_file, "false\n");
}

void cov_check_par(char* log, char* spec, int length, char* inx) {
  int characters = 1000000;
	char line[characters];
	char *word, *sep, *brkt, *brkb;
	long double estimate[length], ideal[length], delta;
	FILE *log_file, *spec_file;
	int ld_size, cnt, i, predicate;

	sep = " ";
	ld_size = 10;

	// reading log file
	log_file = fopen(log, "rt");
	while (fgets(line, characters, log_file) != NULL) {
		if (line[0] != '#') {
			cnt = 0;
			for (word = strtok_r(line, sep, &brkt); 
					word; 
					word = strtok_r(NULL, sep, &brkt)){
        if (cnt < length) {
          estimate[cnt] = cov_deserialize(word, ld_size);
          cnt++;
        }
      }
    }
	}
	fclose(log_file);

	// reading spec file
	spec_file = fopen(spec, "rt");
	while (fgets(line, characters, spec_file) != NULL) {
		if (strncmp(line, "#ideal", 6) == 0) {
			cnt = 0;
			fgets(line, characters, spec_file);
			for (word = strtok_r(line, sep, &brkb); 
					word; 
					word = strtok_r(NULL, sep, &brkb)){
        if (cnt < length) {
          ideal[cnt] = cov_deserialize(word, ld_size);
          cnt++;
        }
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

  // print result to a file
  FILE *result_file;
  char *satf = malloc(strlen("sat_.cov") + strlen(inx) + 1);
  satf[0] = '\0';
  strcat(satf, "sat_");
  strcat(satf, inx);
  strcat(satf, ".cov");
  result_file = fopen(satf, "w");

  if (predicate) fprintf(result_file, "true\n"); 
  else fprintf(result_file, "false\n");
}

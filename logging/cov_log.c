#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "cov_log.h"
#include "cov_serializer.h"

void cov_log(char* msg, char* fn, int count, ...) 
{
  FILE* file;
  file = fopen(fn, "a");
  if (strlen(msg) > 0) fprintf(file, "#%s\n", msg);

  va_list ap;
  int i, j;
  va_start(ap, count);
  for (j=0; j < count; j++)
  {
    int ld_size;
    unsigned char* buf;
    long double ld; 
    
    ld = va_arg(ap, long double);
    ld_size = 10;
    buf = malloc(ld_size);
    cov_serialize(ld, buf, ld_size);
    for (i = 0; i < ld_size; i++)
    {
      fprintf(file, "%02X", buf[i]);
    }
    fprintf(file, " ");
  }
  va_end(ap);
  fprintf(file, "\n");
  fclose(file);
}

void cov_log_(int* count_pointer, ...) 
{
  FILE* file;
  file = fopen("log.cov", "a");
  fprintf(file, "#result\n");
  int count = *count_pointer;

  va_list ap;
  int i, j;
  va_start(ap, count);
  for (j=0; j < count; j++)
  {
    int ld_size;
    unsigned char* buf;
    long double* ld; 
    
    ld = va_arg(ap, long double*);
    ld_size = 10;
    buf = malloc(ld_size);
    cov_serialize(*ld, buf, ld_size);
    for (i = 0; i < ld_size; i++)
    {
      fprintf(file, "%02X", buf[i]);
    }
    fprintf(file, " ");
  }
  va_end(ap);
  fprintf(file, "\n");
  fclose(file);
}

void cov_spec_log(char* fn, long double delta, int count, ...) 
{
  // 
  // logging ideal value
  //
  FILE* file;
  file = fopen(fn, "a");
  fprintf(file, "#ideal\n");

  va_list ap;
  int i, j;
  va_start(ap, count);
  for (j=0; j < count; j++)
  {
    int ld_size;
    unsigned char* buf;
    long double ld; 
    
    ld = va_arg(ap, long double);
    ld_size = 10;
    buf = malloc(ld_size);
    cov_serialize(ld, buf, ld_size);
    for (i = 0; i < ld_size; i++)
    {
      fprintf(file, "%02X", buf[i]);
    }
    fprintf(file, " ");
  }
  va_end(ap);
  fprintf(file, "\n");
  fclose(file);

  //
  // logging delta
  //
  cov_log("delta", fn, 1, delta);
}

void cov_spec_log_(long double* delta_pointer, int* count_pointer, ...) 
{
  long double delta = *delta_pointer;
  int count = *count_pointer;

  // 
  // logging ideal value
  //
  FILE* file;
  file = fopen("spec.cov", "a");
  fprintf(file, "#ideal\n");

  va_list ap;
  int i, j;
  va_start(ap, count);
  for (j=0; j < count; j++)
  {
    int ld_size;
    unsigned char* buf;
    long double* ld; 
    
    ld = va_arg(ap, long double*);
    ld_size = 10;
    buf = malloc(ld_size);
    cov_serialize(*ld, buf, ld_size);
    for (i = 0; i < ld_size; i++)
    {
      fprintf(file, "%02X", buf[i]);
    }
    fprintf(file, " ");
  }
  va_end(ap);
  fprintf(file, "\n");
  fclose(file);

  //
  // logging delta
  //
  cov_log("delta", "spec.cov", 1, delta);
}

void cov_arr_log(long double lds[], int size, char* msg, char* fn) 
{
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

void cov_arr_log_(long double** lds_pointer, int* size_pointer) 
{
	int ld_size, i, j;
	unsigned char* buf;
	FILE *file; 

	file = fopen("spec.cov", "a");
	ld_size = 10;
	buf = malloc(ld_size);

  fprintf(file, "#result\n");
  long double* lds = *lds_pointer;
  int size = *size_pointer;
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

void cov_arr_spec_log(char* fn, long double delta, int count, long double* lds)
{
  //
  // logging ideal value
  //
  cov_arr_log(lds, count, "ideal", fn);

  //
  // logging delta value
  //
  cov_log("delta", fn, 1, delta);
}

void cov_arr_spec_log_(long double* delta, int* count, long double** lds)
{
  //
  // logging ideal value
  //
  cov_arr_log(*lds, *count, "ideal", "spec.cov");

  //
  // logging delta value
  //
  cov_log("delta", "spec.cov", 1, *delta);
}

#ifndef LOG_H_INCLUDED
#define LOG_H_INCLUDED

#include <stdarg.h>

void cov_log(char*, char*, int, ...);
void cov_log_(int*, ...);
void cov_spec_log(char*, long double, int, ...);
void cov_spec_log_(long double*, int*, ...);
void cov_arr_log(long double[], int, char*, char*);
void cov_arr_log_(long double**, int*);
void cov_arr_spec_log(char*, long double, int, long double*);
void cov_arr_spec_log_(long double*, int*, long double**);

#endif

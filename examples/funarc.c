#include "../logging/cov_log.h"
#include "../logging/cov_checker.h"
#include "../logging/cov_serializer.h"

#include <time.h>
#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

extern uint64_t current_time_ns(void);
#define INPUTS 10

long double fun( long double x )
{
  int k, n = 5;
  long double t1, d1 = 1.0L;

  t1 = x;

  for( k = 1; k <= n; k++ )
  {
    d1 = 2.0 * d1;
    t1 = t1 + sin (d1 * x) / d1;
  }

  return t1;
}

int main()
{
  int l;
  uint64_t start, end;
  long int diff = 0;

  int i, j, k; 
  long double h, t1, t2, dppi, ans = 5.795776322412856L;
  long double s1; 

  // variables for logging/checking
  long double log[INPUTS];
  long double threshold = 0.0; 
  long double epsilon = -4.0;

  // 0. read input from the file final_inputs
  int finputs[INPUTS];

  FILE* infile = fopen("final_inputs", "r");
  if (!infile) 
  {
    printf("Could not open final_inputs\n");
  }

  char *s = malloc(10);
  for (i = 0; i < INPUTS; i++)
  {
    if (!feof(infile))
    {
      fscanf(infile, "%s", s);
      finputs[i] = (int)cov_deserialize(s, 10);
    }
  }


  // dummy calls
  sqrtf(0);
  acosf(0);
  sinf(0);

  start = current_time_ns();
  for (l = 0; l < INPUTS; l++)
  {
    int n = finputs[l];
    t1 = -1.0;
    dppi = acos(t1);
    s1 = 0.0;
    t1 = 0.0;
    h = dppi / n;

    for( i = 1; i <= n; i++ )
    {
      t2 = fun (i * h);
      s1 = s1 + sqrt (h*h + (t2 - t1)*(t2 - t1));
      t1 = t2;
    }

    // 1. compute threshold and record result
    log[l] = (long double) s1;
    if (s1*pow(10, epsilon) > threshold)
    {
      threshold = s1*pow(10, epsilon);
    }
  }
  end = current_time_ns();

  diff = (end-start);


  // 2. create spec, or checking results
  cov_arr_spec_log("spec.cov", threshold, INPUTS, log);
  cov_arr_log(log, INPUTS, "result", "log.cov");
  cov_check("log.cov", "spec.cov", INPUTS);

  // 3. print score (diff) to a file
  FILE* file;
  file = fopen("score.cov", "w");
  fprintf(file, "%ld\n", diff);
  fclose(file);

  return 0;

}



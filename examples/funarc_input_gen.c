#include "../logging/cov_log.h"

#include <time.h>
#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

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
  // input n is randomly generated
  srand(time(NULL));
  int n = (rand()+1)%100000;

  int i, j, k; 
  long double h, t1, t2, dppi, ans = 5.795776322412856L;
  long double s1; 

  // dummy calls
  sqrtf(0);
  acosf(0);
  sinf(0);

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

  // write randomly generated input to file 'inputs'
  // write in form of binary format using cov_log
  cov_log("result", "inputs", 1, (long double)n);

  return 0;
}



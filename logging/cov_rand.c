#include <stdlib.h>
#include <math.h>
#include "cov_rand.h"

double cov_rand(int min_e, int max_e) 
{
  double mantisa = (double)rand() / ((double)RAND_MAX+1.0);
  double e = rand()%(max_e-min_e) + min_e; // [min_e, max_e-1]
  double rand = (1.0 + mantisa) * pow(2.0, e);

  return rand;
}

double cov_rand_sp(int itv) 
{
  switch (itv) {
    case 0:
      return -1.0* cov_rand(64, 127);
    case 1:
      return -1.0* cov_rand(0, 64);
    case 2:
      return -1.0* cov_rand(-63, 0);
    case 3:
      return -1.0* cov_rand(-126, -63);
    case 4:
      return 1.0*0.0;
    case 5:
      return -1.0*0.0;
    case 6:
      return cov_rand(-126, -63);
    case 7:
      return cov_rand(-63, 0);
    case 8:
      return cov_rand(0, 64);
    case 9:
      return cov_rand(64, 127);
    default:
      return cov_rand_sp(itv%10);
  }
}

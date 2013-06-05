#include <stdio.h>
#include <stdlib.h>


void printBranch(int blockId, int branchId) 
{
  FILE* file = fopen("coverage.cov", "a");
  fprintf(file, "Reaching branch %d ", branchId);
  fclose(file);
}

void printBranchTo(int blockId)
{
  FILE* file = fopen("coverage.cov", "a");
  fprintf(file, "lead to block %d\n", blockId);
  fclose(file);
}

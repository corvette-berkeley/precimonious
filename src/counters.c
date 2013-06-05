
#include <stdio.h>

int counterStoreInstFloat = 0;
int counterStoreInstDouble = 0;
int counterStoreInstLongDouble = 0;

int counterLoadInstFloat = 0;
int counterLoadInstDouble = 0;
int counterLoadInstLongDouble = 0;

int counterArithOpInstFloat = 0;
int counterArithOpInstDouble = 0;
int counterArithOpInstLongDouble = 0;

int counterCmpOpInstFloat = 0;
int counterCmpOpInstDouble = 0;
int counterCmpOpInstLongDouble = 0;

int counterFPTruncInst = 0;
int counterFPExtInst = 0;

long int score = 0;

// Store Instruction
void incrementStoreInstFloat() {
  counterStoreInstFloat++;
//  score += 32;
  score += 8;
}

void incrementStoreInstDouble() {
  counterStoreInstDouble++;
//  score += 64;
  score += 8;
}

void incrementStoreInstLongDouble() {
  counterStoreInstLongDouble++;
//  score += 80;
  score += 8;
}

// Load Instruction
void incrementLoadInstFloat() {
  counterLoadInstFloat++;
//  score += 32;
  score += 8;
}

void incrementLoadInstDouble() {
  counterLoadInstDouble++;
//  score += 64;
  score += 8;
}

void incrementLoadInstLongDouble() {
  counterLoadInstLongDouble++;
//  score += 80;
  score += 8;
}

// ArithOp Instruction
void incrementArithOpInstFloat() {
  counterArithOpInstFloat++;
//  score += 32;
  score += 20;
}

void incrementArithOpInstDouble() {
  counterArithOpInstDouble++;
//  score += 64;
  score += 30;
}

void incrementArithOpInstLongDouble() {
  counterArithOpInstLongDouble++;
//  score += 80;
  score += 40;
}


// CmpOp Instruction
void incrementCmpOpInstFloat() {
  counterCmpOpInstFloat++;
//  score += 32;
  score += 20;
}

void incrementCmpOpInstDouble() {
  counterCmpOpInstDouble++;
//  score += 64;
  score += 30;
}

void incrementCmpOpInstLongDouble() {
  counterCmpOpInstLongDouble++;
//  score += 80;
  score += 40;
}


// Cast Instructions
void incrementFPTruncInst() {
  counterFPTruncInst++;
//  score += 32;
  score += 1;
}

void incrementFPExtInst() {
  counterFPExtInst++;
//  score += 32;
  score += 1;
}



// Printing counters
void printCounters() {
  printf("StoreInst\n");
  printf("\tFloat: %d\n", counterStoreInstFloat);
  printf("\tDouble: %d\n", counterStoreInstDouble);
  printf("\tLongDouble: %d\n", counterStoreInstLongDouble);

  printf("LoadInst\n");
  printf("\tFloat: %d\n", counterLoadInstFloat);
  printf("\tDouble: %d\n", counterLoadInstDouble);
  printf("\tLongDouble: %d\n", counterLoadInstLongDouble);

  printf("ArithOpInst\n");
  printf("\tFloat: %d\n", counterArithOpInstFloat);
  printf("\tDouble: %d\n", counterArithOpInstDouble);
  printf("\tLongDouble: %d\n", counterArithOpInstLongDouble);

  printf("CmpOpInst\n");
  printf("\tFloat: %d\n", counterCmpOpInstFloat);
  printf("\tDouble: %d\n", counterCmpOpInstDouble);
  printf("\tLongDouble: %d\n", counterCmpOpInstLongDouble);

  printf("CastInst\n");
  printf("\tTruncInst: %d\n", counterFPTruncInst);
  printf("\tExtInst: %d\n", counterFPExtInst);

  printf("Score: %ld\n", score);

  FILE* file;
  file = fopen("score.cov", "w");
  fprintf(file, "%ld\n", score);

  return;
}

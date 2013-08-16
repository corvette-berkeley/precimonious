#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <sys/syscall.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <sched.h>
#include <sys/resource.h>



#if defined(__i386__)

uint64_t rdtsc(void)
{
  uint64_t x;
  __asm__ volatile (".byte 0x0f, 0x31" : "=A" (x));
  return x;
}
#elif defined(__x86_64__)

uint64_t rdtsc(void)
{
  unsigned hi, lo;
  __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
  return ( (uint64_t)lo)|( ((uint64_t)hi)<<32 );
}

#elif defined(__powerpc__)

uint64_t rdtsc(void)
{
  uint64_t result=0;
  unsigned long upper, lower,tmp;
  __asm__ volatile(
               "0:                  \n"
               "\tmftbu   %0           \n"
               "\tmftb    %1           \n"
               "\tmftbu   %2           \n"
               "\tcmpw    %2,%0        \n"
               "\tbne     0b         \n"
               : "=r"(upper),"=r"(lower),"=r"(tmp)
		   );
  result = upper;
  result = result<<32;
  result = result|lower;

  return(result);
}
#else

#error "No tick counter is available!"

#endif

#define MONTONIC_TIME 1

static void get_time(struct timespec *ts)
{
#if MONTONIC_TIME
  int clock_type = CLOCK_MONOTONIC;
#else
  int clock_type = CLOCK_REALTIME;
#endif
  if (syscall(__NR_clock_gettime, clock_type, ts)) {
    printf("clock_gettime(MONOTONIC/REALTIME) failed");
    exit(-1);
  }
}


uint64_t current_time_ns(void)
{
  struct timespec ts;
  get_time(&ts);
  return ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}
uint64_t current_time_us(void)
{
  struct timespec ts;
  get_time(&ts);
  return ts.tv_sec * 1000000ULL + ts.tv_nsec/1000;
}
unsigned current_time_sec(void)
{
  struct timespec ts;
  get_time(&ts);
  return ts.tv_sec;
}

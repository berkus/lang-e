#include "sw.h"

static struct rusage start;
static struct rusage stop;

void tc_startwatch (int who) {
#if !defined(__SVR4)
     if (getrusage(who, &start) != 0) {
	  fprintf(stderr, "\ngetrusage error");
	  exit(1);
     }
#endif
}

void tc_stopwatch (int who) {
#if !defined(__SVR4)
     double time;

     if (getrusage(who, &stop) != 0) {
	  fprintf(stderr,"\ngetrusage error");
	  exit(1);
     }

     time = ((double)stop.ru_utime.tv_sec - (double)start.ru_utime.tv_sec)
	  + (((double)stop.ru_utime.tv_usec - (double)start.ru_utime.tv_usec) 
	     / 1000000.0); 
     printf( "user time used %f   ", time);
  
     time = ((double)stop.ru_stime.tv_sec - (double)start.ru_stime.tv_sec) + 
	  (((double)stop.ru_stime.tv_usec - (double)start.ru_stime.tv_usec) 
	   / 1000000.0); 
     printf("system time used %f\n", time);
  
     printf("maximum resident size %ld\n", stop.ru_maxrss - start.ru_maxrss);
     printf("page faults not requiring I/O %ld  ",
	    stop.ru_minflt - start.ru_minflt);
     printf("page faults requiring I/O %ld\n", 
	    stop.ru_majflt - start.ru_majflt);
     printf("voluntary context switches %ld  ", 
	    stop.ru_nvcsw - start.ru_nvcsw);
     printf("involuntary context switches %ld\n", 
	    (long)(stop.ru_nivcsw - start.ru_nivcsw));
#endif
}

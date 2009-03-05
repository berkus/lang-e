#include	<stdio.h>
#include	<assert.h>
#include	<sys/time.h>
#include	<sys/resource.h>

#ifndef _V_SOLARIS_
struct rusage	start_rusage;
struct rusage	stop_rusage;
#endif

void startwatch( who )
        int who;
{
#ifndef _V_SOLARIS_

	if( getrusage( who, &start_rusage ) != 0 ) {
          fprintf(stderr,"\ngetrusage error");
	  exit(1);
          }
#endif
}

void stopwatch( who )
	int	who;
{
#ifndef _V_SOLARIS_
double				time;

if( getrusage( who, &stop_rusage ) != 0 ) {
   fprintf(stderr,"\ngetrusage error");
   exit(1);
   }


time = ((double)stop_rusage.ru_utime.tv_sec - 
        (double)start_rusage.ru_utime.tv_sec) + 
       ((double)stop_rusage.ru_utime.tv_usec -
        (double)start_rusage.ru_utime.tv_usec) / 1000000.0; 
printf( "user time used %f   ", time );

time = ((double)stop_rusage.ru_stime.tv_sec - 
        (double)start_rusage.ru_stime.tv_sec) + 
       ((double)stop_rusage.ru_stime.tv_usec -
        (double)start_rusage.ru_stime.tv_usec) / 1000000.0; 
printf( "system time used %f\n", time );

printf( "maximum resident size %ld\n",stop_rusage.ru_maxrss - 
			              start_rusage.ru_maxrss);

printf( "page faults not requiring I/O %ld  ",stop_rusage.ru_minflt -
					      start_rusage.ru_minflt);

printf( "page faults requiring I/O %ld\n", stop_rusage.ru_majflt - 
				           start_rusage.ru_majflt);

printf( "voluntary context switches %ld  ", stop_rusage.ru_nvcsw - 
					    start_rusage.ru_nvcsw);

printf( "involuntary context switches %ld\n", (long)(stop_rusage.ru_nivcsw - 
					      start_rusage.ru_nivcsw));
#endif
}

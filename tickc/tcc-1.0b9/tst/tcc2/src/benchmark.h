#include	<stdio.h>
#include	<assert.h>
#include	<sys/time.h>
#include	<sys/resource.h>

static int optVerbose = 0, optCount = 0, optDCG = 1, optDump = 0, optTime = 1;
static int ncomp = 0, nruns = 0, ndata = 0;

void doargs(int argc, char **argv) {
     int i;
     for (i=1; i<argc; i++) {
				/* modes */
	  if (!strcmp(argv[i], "-v")) optVerbose = 1;
	  else if (!strcmp(argv[i], "-dump")) { optDump++; optTime = 0; }
#ifdef __TCC__ /* allows us to use benchmark.h with a static compiler without
		  linking in the tcc runtime (libtickc-rts.a) */
	  else if (!strcmp(argv[i], "-count")) { optCount++; optTime = 0; }
	  else if (!strcmp(argv[i], "-s")) optDCG = 0;
#endif
				/* params */
	  else if (!strcmp(argv[i], "-compiles")) ncomp = atoi(argv[++i]);
	  else if (!strcmp(argv[i], "-runs")) nruns = atoi(argv[++i]);
	  else if (!strcmp(argv[i], "-data")) ndata = atoi(argv[++i]);
				/* what to measure */
#ifdef __TCC__
	  else if (!strcmp(argv[i], "-end_closures")) tc_end_closures();
	  else if (!strcmp(argv[i], "-end_IR")) tc_end_IIR();
#endif
	  else if (!strcmp(argv[i], "-end_fg")) i_end_fg();
	  else if (!strcmp(argv[i], "-end_lv")) i_end_lv();
	  else if (!strcmp(argv[i], "-end_ra1")) i_end_ra1();
	  else if (!strcmp(argv[i], "-end_ra2")) i_end_ra2();
	  else if (!strcmp(argv[i], "-rallocFLR")) i_ralloc_ls();
	  else if (!strcmp(argv[i], "-rallocLin")) i_ralloc_lr();
	  else if (!strcmp(argv[i], "-rallocGC")) i_ralloc_gc();
	  else if (!strcmp(argv[i], "-rallocNone")) i_ralloc_ez();
#ifdef __TCC__
	  else if (!strcmp(argv[i], "-bpo")) tc_bpo_on();
#endif
	  else {
	       printf("Unexpected argument: \"%s\"\n", argv[i]);
	       exit(-1);
	  }
     }
#ifdef __TCC__
     if (optDump > 1) tc_debugon();
#endif
}

#ifdef __TCC__
#define dump(fp) do { 							\
     if (optDump == 1) v_dump((void*)fp); 				\
     if (optCount == 1) printf("Number of insns: %d\n", tc_ninsn());	\
} while (0)
#else
#define dump(fp)
#endif


#ifdef __SIMPLE_SCALAR__

#include	<ssdump.h>
				/* Use gettimeofday, which we hacked up to
				   provide cycle counts. */
static struct timeval xtv0, xtv1;
static struct timezone xtz0, xtz1;

void startwatch() { gettimeofday(&xtv0, &xtz0); }

void stopwatch() {
     double cycles;

     assert(sizeof(int) == 4);

     gettimeofday(&xtv1, &xtz1);

     cycles = (double)((xtv1.tv_sec - xtv0.tv_sec)*0xffffffff) +
	  (double)(xtv1.tv_sec - xtv0.tv_sec) +
	  (double)(xtv1.tv_usec - xtv0.tv_usec);

     printf("Number of cycles: %e\n", cycles);
}

swon(char *msg) {
     if (optTime) {
	  if (optDCG) printf("\n=== Doing dynamic: %s\n"
			     "Compiles: %d; Runs: %d; Data: %d\n",
			     msg, ncomp, nruns, ndata);
	  else printf("\n=== Doing static: %s\nRuns: %d; Data: %d\n", 
		      msg, nruns, ndata);
	  startwatch();
     }
}

#define SWON(msg) do { swon(msg); ssd_trace_on(); ssd_stats_on(); } while (0)
#define SWOFF do { ssd_trace_off(); ssd_stats_off(); if (optTime) stopwatch(); } while (0)

#else

struct rusage	start_rusage;
struct rusage	stop_rusage;

void startwatch(int who) {
     if( getrusage( who, &start_rusage ) != 0 ) {
	  fprintf(stderr,"\ngetrusage error");
	  exit(1);
     }
}

void stopwatch(int who) {
     double time;

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
}

swon(char *msg) {
     if (optTime) {
	  if (optDCG) printf("\n=== Doing dynamic: %s\n"
			     "Compiles: %d; Runs: %d; Data: %d\n",
			     msg, ncomp, nruns, ndata);
	  else printf("\n=== Doing static: %s\nRuns: %d; Data: %d\n", 
		      msg, nruns, ndata);
	  startwatch(RUSAGE_SELF);
     }
}

#define SWON(msg)  swon(msg)
#define SWOFF do { if (optTime) stopwatch(RUSAGE_SELF);	} while (0)

#endif

#include	<stdio.h>
#include	<assert.h>
#include	<sys/time.h>
#include	<sys/resource.h>

static int optVerbose = 0, optCount = 0, optDCG = 1, optDump = 0, optTime = 1;
static int ncomp, nruns, ndata;
static int xncomp, xnruns, xndata;

void doargs(int argc, char **argv) {
     int i;
     for (i=1; i<argc; i++) {
				/* modes */
	  if (!strcmp(argv[i], "-v")) optVerbose = 1;
#ifdef __TCC__ /* allows us to use benchmark.h with a static compiler without
		  linking in all the tcc libs */
	  else if (!strcmp(argv[i], "-dump")) { optDump++; optTime = 0; }
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
	  else if (!strcmp(argv[i], "-end_fg")) i_end_fg();
	  else if (!strcmp(argv[i], "-end_lv")) i_end_lv();
	  else if (!strcmp(argv[i], "-end_ra1")) i_end_ra1();
	  else if (!strcmp(argv[i], "-end_ra2")) i_end_ra2();
	  else if (!strcmp(argv[i], "-rallocFLR")) i_ralloc_ls();
	  else if (!strcmp(argv[i], "-rallocLin")) i_ralloc_lr();
	  else if (!strcmp(argv[i], "-rallocGC")) i_ralloc_gc();
	  else if (!strcmp(argv[i], "-rallocNone")) i_ralloc_ez();
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

static double utime, stime;

struct rusage	start_rusage;
struct rusage	stop_rusage;

void startwatch (int who) {
     if( getrusage( who, &start_rusage ) != 0 ) {
	  fprintf(stderr,"\ngetrusage error");
	  exit(1);
     }
}

void stopwatch (int who) {
     double time;

     if( getrusage( who, &stop_rusage ) != 0 ) {
	  fprintf(stderr,"\ngetrusage error");
	  exit(1);
     }

     utime += ((double)stop_rusage.ru_utime.tv_sec - 
	       (double)start_rusage.ru_utime.tv_sec) + 
	  ((double)stop_rusage.ru_utime.tv_usec -
	   (double)start_rusage.ru_utime.tv_usec) / 1000000.0; 
  
     stime += ((double)stop_rusage.ru_stime.tv_sec - 
	       (double)start_rusage.ru_stime.tv_sec) + 
	  ((double)stop_rusage.ru_stime.tv_usec -
	   (double)start_rusage.ru_stime.tv_usec) / 1000000.0; 
}

void swinit (char *msg) {
     if (optTime) {
	  if (optDCG) printf("\n=== Doing dynamic: %s\n"
			     "Compiles: %d; Runs: %d; Data: %d\n",
			     msg, xncomp, xnruns, xndata);
	  else printf("\n=== Doing static: %s\nRuns: %d; Data: %d\n", 
		      msg, xnruns, xndata);
     }
     utime = stime = 0.0;
}

void swdump (void) {
     if (optTime) {
	  printf("user time used %f   ", utime);
	  printf("system time used %f\n", stime);
     }
}

#define SWINIT(msg) swinit(msg)
#define SWDUMP swdump()
#define SWON  do { if (optTime) startwatch(RUSAGE_SELF); } while (0)
#define SWOFF do { if (optTime) stopwatch(RUSAGE_SELF);	} while (0)

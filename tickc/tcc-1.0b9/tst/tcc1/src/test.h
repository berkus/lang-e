#ifndef __TCC_TEST_H__
#define __TCC_TEST_H__

extern int printf();

static int optVerbose = 0, optCount = 0, optDCG = 1, optDump = 0, optTime = 1;
static int ncomp = 0, nruns = 0, ndata = 0;

void doargs(int argc, char **argv) {
     int i;
     for (i=1; i<argc; i++) {
				/* modes */
	  if (!strcmp(argv[i], "-v")) optVerbose = 1;
	  else if (!strcmp(argv[i], "-dump")) { optDump++; optTime = 0; }
	  else if (!strcmp(argv[i], "-count")) { optCount++; optTime = 0; }
	  else if (!strcmp(argv[i], "-s")) optDCG = 0;
				/* params */
	  else if (!strcmp(argv[i], "-compiles")) ncomp = atoi(argv[++i]);
	  else if (!strcmp(argv[i], "-runs")) nruns = atoi(argv[++i]);
	  else if (!strcmp(argv[i], "-data")) ndata = atoi(argv[++i]);
				/* what to measure */
	  else if (!strcmp(argv[i], "-end_closures")) tc_end_closures();
	  else if (!strcmp(argv[i], "-end_IR")) tc_end_IIR();
	  else if (!strcmp(argv[i], "-end_fg")) i_end_fg();
	  else if (!strcmp(argv[i], "-end_lv")) i_end_lv();
	  else if (!strcmp(argv[i], "-end_ra1")) i_end_ra1();
	  else if (!strcmp(argv[i], "-end_ra2")) i_end_ra2();
	  else if (!strcmp(argv[i], "-rallocFLR")) i_ralloc_ls();
	  else if (!strcmp(argv[i], "-rallocLin")) i_ralloc_lr();
	  else if (!strcmp(argv[i], "-rallocGC")) i_ralloc_gc();
#if 0
	  else if (!strcmp(argv[i], "-bpo")) tc_bpoon();
#endif
     }
     if (optDump > 1) tc_debugon();
}

#define dump(fp) do { 							\
     if (optDump == 1) v_dump((void*)fp); 				\
     if (optCount == 1) printf("Number of insns: %d\n", tc_ninsn());	\
} while (0)

#endif /* __TCC_TEST_H__ */

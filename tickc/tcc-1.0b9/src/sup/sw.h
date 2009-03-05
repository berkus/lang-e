#ifndef __STOPWATCH_H__
#define __STOPWATCH_H__

#include <stdio.h>
#include <sys/time.h>
#include <sys/resource.h>


#if defined(__sparc__)
#if !(defined(__LCC__) || defined(__TCC__) || defined(__SVR4))
extern int fprintf (FILE *, char *, ...);
extern int printf (char *, ...);
extern int getrusage (int, struct rusage *);
#endif
#endif

extern void tc_startwatch (int who);
extern void tc_stopwatch (int who);

#define tc_swatch_on  do { tc_startwatch(RUSAGE_SELF); } while (0)
#define tc_swatch_off do { tc_stopwatch(RUSAGE_SELF); } while (0)

#endif

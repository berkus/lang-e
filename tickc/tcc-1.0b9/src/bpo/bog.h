/* $Id: bog.h,v 1.1.1.1 1997/12/05 01:25:43 maxp Exp $ */

#ifndef __BOG_H__
#define __BOG_H__

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "list.h"
#include "mem.h"
#include "set.h"
#include "string.h"
#include "sym.h"
#include "btype.h"

#if defined(__sparc__) && !defined(__LCC__) && !defined(__SVR4)
extern int fprintf (FILE *, char *, ...);
extern int printf(char *, ...);
#endif

/*
 * Rule data structures
 */

typedef struct {		/* Variable info for input words */
     int id;			/* Indices of variables */
     int sh;			/* Position of rightmost bit of each var */
     T msk;			/* Mask for extracting each var in v_ids */
} var_t;

typedef struct {		/* One input word (T) for a rule */
     T f_msk;			/* Mask for fixed part */
     T f_val;			/* Value for fixed part */
     int nv;			/* Number of variables to match */
     var_t **v;			/* Variables */
} pat_t;

typedef struct {
     pat_t **in;
     int nin;
     pat_t **out;
     int nout;
} rule_t;

#define SZ (8*sizeof(T))

/*
 * Inline functions
 */

/* Code to mark (as done, checked, etc) subsets of input sides of rules */
typedef struct {
     int sz;
     int *data;
} *marktype;
static inline marktype markinit (int i) { 
     marktype x;
     NEW(x, 1, ARENA1);
     x->sz = i+1; NEW0(x->data, x->sz, ARENA1);
     return x;
}
typedef enum { CHECKED=1, DONE=2 } markkind;
static inline void mark (marktype x, int i, markkind k) { 
     x->data[i] |= k; 
}
static inline int marked (marktype x, int i, markkind k) { 
     return x->data[i] & k;
}
static inline int markedupto (marktype x, int i, markkind k) {
     int j; for (j = 1; j <= i; j++) if (!(x->data[i] & k)) return 0;
     return 1;
}
static inline int markedabove (marktype x, int i, markkind k) {
     int j; for (j = x->sz-1; j > i; j--) if (x->data[i] & k) return 1;
     return 0;
}
static inline marktype markcopy (marktype orig) {
     int i;
     marktype x;
     NEW(x, 1, ARENA1);
     x->sz = orig->sz; NEW0(x->data, x->sz, ARENA1);
     for (i = 1; i < x->sz; i++) x->data[i] = orig->data[i];
     return x;
}

/*
 * Prototypes and globals needed across modules
 */

extern void exit(int);
extern void error (char *msg);
extern void treematch (List rl);
extern void metamrst (rule_t **r, marktype status, char *ind);
extern void mrstatpos (rule_t **r, int pos, int mil, 
		       marktype status, char *ind);

extern int debugp, treep, nr;
extern Table stab;

#include "cwmisc.h"

#endif __BOG_H__

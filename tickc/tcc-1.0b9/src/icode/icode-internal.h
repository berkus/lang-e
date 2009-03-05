/* $Id: icode-internal.h,v 1.8 1998/05/17 20:38:32 maxp Exp $ */

/* Stuff the icode client does not need to (should not) access */

#ifndef __ICODE_INTERNAL_H__
#define __ICODE_INTERNAL_H__

#include <limits.h>

#if defined(__sparc__) && !defined(__LCC__) && !defined(__SVR4)
extern int fprintf (FILE *, char *, ...);
extern int printf(char *, ...);
#endif

/*
 * Limits and other constants
 */

				/* Default numbers of items: use multiples of
				   x to work with loop unrolling in icode.c */
enum { dn_fpi=64,		/* Floating point immediates (x=16) */
       dn_calls=64 };		/* Calls in 1 function when using FLR ralloc */

				/* Limits for some items */
enum { min_lab=2,		/* Labels */
/* NOTE: vcode (v_lambda) creates a label (by calling v_genlabel) to jump to
   the epilogue code; this is label number 1.  Icode starts numbering labels
   at 2 so that vcode can use icode's label numbers without incurring the cost
   of mapping one set of labels to the other.  The only other place in vcode
   that uses v_genlabel is conditional moves, which are not currently generated
   by icode.  If v_genlabel is used to create anything more than 1 label, we
   should set min_lab=0 and perform icode->vcode label mapping. */
       max_lab=1<<i_width, 
       max_loc=1<<i_width,
       max_param=I_MAXPARAM,
       max_cnt=INT_MAX
};	

enum { NOLRINFO=-1 };
				/* Storage class */
enum { UNDECIDED=0, DATA=1, STACK=2, REGISTER=4, IGNORE=8 };

enum { false=0, true=1 };

enum { WHITE=0, BLACK };

/*
 * Internal opcode stuff
 */

#define isimmed(op) ((op)&0x200)/* Immediate if (op&512) */
#define generic(op) ((op)&0x1FF)/* Return opcode, ignoring immediate bit */
				/* Eliminate the instruction at cp */
static inline void mk_nop (icode_t cp) { *cp = set_op(i_op_nop); }

/*
 * Locals
 */

extern i_local_t i_loc_icur;	/* Current int local id */
extern i_local_t i_loc_fcur;	/* Current fp local id */

enum { 
     i_loc_ilim = (1 << (i_width-1)),
     i_loc_flim = i_lp
};

extern i_local_t i_params_cur;	/* Current (maximum) parameter id */
typedef struct { i_local_t i; int t; } i_pmap_t;
extern i_pmap_t i_params[max_param];

				/* Extract information about locals */
#define SCLASS(local)	(i_locals[(local)].sclass)
#define ADDR(local) 	(i_locals[(local)].addr)
#define TYPE(local)	(i_locals[(local)].type)
#define PARAMP(local)	(i_locals[(local)].paramp)
#define LRANGE(local)	(i_locals[(local)].lr)

enum { INT=0, FP=1 };
#define isfloat(type)	((type) == I_F || (type) == I_D)
extern int i_ty2sc[];

#define sc_var(i)	((i) < i_lp)
#define sc_fp(i)        ((i) >= i_loc_ilim)
#define sc_int(i)       ((i) < i_loc_ilim)
#define fp_id(i)        ((i) - i_loc_ilim) /* fp handle -> bit vector id */
#define fp_mk(i)        ((i) + i_loc_ilim) /* bit vector id -> fp handle */

extern i_cnt_t num_i, num_f, num_bb;

/*
 * Labels
 */

extern i_label_t i_lab_cur;	/* Current label id */

/*
 * Floating point immediates
 */

typedef union {
     float f; 
     double d;
} i_fpi_t;
extern i_fpi_t *i_fpi;
#define i_immd(id)	(i_fpi[(id)].d)
#define i_immf(id)	(i_fpi[(id)].f)

/*
 * Misc flags
 */

				/* Selecting quality of register allocation */
enum { RA_EZ=0, RA_GC=1, RA_LR=2, RA_LS=4 };
extern unsigned int i_ralloctype;
extern unsigned int i_dorefcnt;

enum { END_FG=1, END_LV=2, END_RA1=4, END_RA2=8 };
extern unsigned int i_quit;

/*
 * Unparsing
 */

#include "macros-prt.h"

extern void i_unparseinsn (FILE *fd, i_puint32 cp);

/*
 * Debug stuff
 */

#ifdef DEBUG
#undef DEBUG
#endif
#ifdef NDEBUG
#define DEBUG(x)
#else
#define DEBUG(x) do { if (i_debug) { x; } } while (0)
#endif
extern unsigned int i_debug;

extern void i_fatal (char *msg);

#include <bv.h>			/* Bit vectors */
#include "li.h"			/* Live intervals */
#include "cfg.h"		/* Control flow graph */
#include "reg.h"		/* Register allocation */
#include "util.h"		/* Random stuff */
#include "op2class.h"		/* Map opcodes to insn class */

#endif

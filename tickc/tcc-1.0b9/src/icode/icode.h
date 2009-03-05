/* $Id: icode.h,v 1.8 1998/05/17 20:38:34 maxp Exp $ */

#ifndef __ICODE_H__
#define __ICODE_H__

#include <stdio.h>
#include <vcode.h>

#if defined(__TCC__) && !defined(__CC2__) || defined(__LCC__) || defined(__STRICT_ANSI__)
#define inline
#endif

/*
 * Some useful typedefs
 */

#if !defined(__sparc__) && !defined(__mips__)
#if !defined(__i386__) && !defined(__SIMPLE_SCALAR__)
#error "No icode types for this architecture"
#endif
#endif

typedef unsigned long i_uint32;
typedef i_uint32 * i_puint32;
typedef i_puint32 icode_t;
typedef float i_ref_t;
typedef unsigned char i_flag_t;
typedef unsigned int i_cnt_t;

/*
 * Main data
 */

#include "opcode.h"
				
enum {
     i_isize=2,
     i_width=11, 
     i_zero=(1<<i_width)-1,	/* Special locs: i_zero=0, i_lp="local ptr" */
     i_lp=(1<<i_width)-2,
     i_margin = 64*i_isize	/* i_lim = i_buf+bufsz-i_margin: defines 
				   region in which client should call i_bump */
};

extern icode_t i_buf;		/* Pointer to code buffer */
extern icode_t i_lim;		/* Limit of code buffer */
extern icode_t i_ip;		/* Current instruction  */
extern i_cnt_t i_nbb;		/* Max number of bblocks */

#include "macros-pat.h"
#include "macros-gen.h"

/*
 * Main routines
 */

extern void i_init (unsigned long ninst);
extern void i_end (void);
extern void i_reset (void);

/*
 * Linking
 */
				/* Label type, same as vcode label type */
typedef i_cnt_t i_label_t;
extern i_label_t i_mklabel (void);

/*
 * Locals
 */

enum {				/* Type codes: */
     I_C=V_C,			/* char */
     I_UC=V_UC,			/* unsigned char */
     I_S=V_S,			/* short */
     I_US=V_US,			/* unsigned short */
     I_I=V_I,			/* int */
     I_U=V_U,			/* unsigned */
     I_L=V_L,			/* long */
     I_UL=V_UL,			/* unsigned long */
     I_P=V_P,			/* pointer */
     I_F=V_F,			/* floating */
     I_D=V_D,			/* double */
     I_V=V_V,			/* void */
     I_B=V_B,			/* block structure */
     I_ERR=V_ERR,		/* error condition */
     I_REGISTER=V_REGISTER,
     I_MEMORY=V_MEMORY
};

typedef i_cnt_t i_local_t;	/* Type of locals index */
typedef struct {		/* Rep for locals */
     int addr;			/* Actual address/offset or register number */
     i_cnt_t lr;		/* Live range number */
     i_flag_t sclass;		/* Storage class */
     i_flag_t type;		/* Type info */
     i_flag_t paramp;		/* Non-zero when local is a parameter */
     float refcnt;		/* Reference count */
} i_lrep_t;
extern i_lrep_t i_locals[];	/* List of locals */
				/* Get refcnt info: must be user-visible */
#define REFCNT(local)	(i_locals[(local)].refcnt)

extern i_local_t i_local (unsigned long flags, unsigned int type);
extern i_local_t i_localb (unsigned int size);
extern i_local_t i_param (unsigned int type);
extern i_local_t i_paramn (unsigned int type, unsigned int n);
enum { I_MAXPARAM=10 };		/* depends on vcode MAXPARAM */

/*
 * Floating point immediates
 */

extern i_cnt_t i_fpi_addf (float f);
extern i_cnt_t i_fpi_addd (double d);

/*
 * Unparsing and debugging
 */

extern void i_unparse (void);
extern void i_debug_on (void);
extern void i_debug_off (void);
extern void i_fatal (char *msg);

/*
 * Other stuff
 */

extern unsigned int i_leafp;	/* True when the current function is a leaf */

/* Reference counts */
extern i_ref_t i_refc;
extern void i_refcnt_on (void);
extern void i_refcnt_off (void);

/* Register allocation modes */
extern void i_ralloc_ez (void);
extern void i_ralloc_ls (void);
extern void i_ralloc_lr (void);
extern void i_ralloc_gc (void);

/* Cache support */
extern unsigned i_cachesize;
extern void i_setcachesize (unsigned int s);
#define i_cachealign(p) ((p) & ((i_cachesize-1) & (~0x7)))

/* Benchmarking support */
extern void i_end_fg (void);
extern void i_end_lv (void);
extern void i_end_ra1 (void);
extern void i_end_ra2 (void);

/* Emitting code */
extern union v_fp i_emit (unsigned int *offset, unsigned int *ninsn);

#endif /* __ICODE_H__ */

/* $Id: tickc-rts.h,v 1.5 1998/01/19 20:30:23 maxp Exp $ */

				/* So tcc can compile its run-time system */
#if (!defined(__TCC__) || defined(__CC2__))
#ifndef __TICKC_LIB_H__
#define __TICKC_LIB_H__

#include <icode.h>
#include <mem.h>

#if defined(__sparc__)
enum { _tc_tr0=_g1 };
#elif defined(__mips__) || defined(__sslittle__) || defined(__ssbig__)
enum { _tc_tr0=_t0 };
#endif

/* Tickc types: must match icode and vcode types */
enum {
	TC_C=I_C,		/* char */
	TC_UC=I_UC,		/* unsigned char */
	TC_S=I_S,		/* short */
	TC_US=I_US,		/* unsigned short */
	TC_I=I_I,		/* int */
	TC_U=I_U,		/* unsigned */
	TC_L=I_L,		/* long */
	TC_UL=I_UL,		/* unsigned long */
	TC_P=I_P,		/* pointer */
	TC_F=I_F,		/* float */
	TC_D=I_D,		/* double */
	TC_B=I_B,		/* aggregate (block) */
	TC_ERR=I_ERR,		/* error */
	TC_REGISTER=I_REGISTER,
	TC_MEMORY=I_MEMORY
};

/*
 * Limits and defaults
 */

enum { TC_CODESIZE=131072,	/* Initial dynamic code segment size: 128KB */
       TC_MAXCALL=10,		/* Max. number of outstanding dynamic calls */
       TC_MAXLOCAL=4096,	/* Max. number of dynamic locals */
       TC_MAXTARG=256,		/* Default number of dynamic jump targets */
};

/*
 * Type defs
 */

typedef struct {		/* Base closure type */
     i_local_t (*code)(void*);
     i_label_t lab;
} *_tc_closure_t;
				/* Dynamic call context */
enum { TC_MAXPARAM=I_MAXPARAM };/* Max. number of dynamic arguments */
typedef struct {	       
     int argnum;
     int argtypes[TC_MAXPARAM];
     _tc_closure_t closures[TC_MAXPARAM];
} _tc_dcall_t;
				/* Rep for locals at vcode level */
typedef struct {
     int type;
     int addr;
     int size;
     int reg;
} _tc_loc_t;
extern _tc_loc_t _tc_llocal[TC_MAXLOCAL];

				/* Reps for cspec and vspec objects */
typedef _tc_closure_t _tc_cspec_t;
typedef i_local_t _tc_vspec_t;

typedef v_code * _tc_code_t;	/* Pointer to dynamic code segment */

typedef int (*_tc_ip_t)();	/* Pointer to int function: the default
				   returned by compile */

/*
 * Data and flags
 */

extern _tc_cspec_t _tc_cspec;	/* Generic cspec object */
extern int _tc_leafp;		/* True if current dynamic func is a leaf */
extern _tc_code_t _tc_cp;	/* Pointer to current dynamic code segment */

/*
 * Prototypes
 */

/* Debugging */
extern void tc_debugon (void);	/* Visible to user: no leading '_' */
extern void tc_debugoff (void);

/* Cache */
extern void tc_setcachesize (unsigned int s);

/* Benchmarking */
extern void tc_end_closures (void);
extern void tc_end_IIR (void);

/* Binary peephole optimizations */
extern void tc_bpo_on (void);
extern void tc_bpo_off (void);

/* Dynamic call construction */
extern _tc_dcall_t * _tc_push_init (void);
extern void _tc_push (_tc_dcall_t *tcd, int type, ...);
extern void _tc_arg (int k, _tc_dcall_t *tcd, int type, ...);
extern void _tc_dargs (_tc_dcall_t *tcd);
extern void _tc_vdargs (_tc_dcall_t *tcd, struct v_cstate *vcs);

/* Dynamic labels/jumps, and recursion */
extern void _tc_mktarget (_tc_cspec_t c);
				/* Icode */
extern void _tc_jump (void *c);
extern void _tc_label (void * c);
				/* Vcode */
extern void _tc_jumpf (void *c);
extern void _tc_labelf (void * c);

/* Locals and params */

#define _tc_stackp(loc) (! _tc_llocal[(loc)].reg)
#define _tc_offset(loc) (_tc_llocal[(loc)].addr)
#define _tc_reg(loc) _tc_offset(loc)
				/* Icode support */
extern _tc_vspec_t _tc_local (unsigned long flags, unsigned int type);
extern _tc_vspec_t _tc_localb (unsigned int size);
extern _tc_vspec_t _tc_param (unsigned int type, unsigned int argno);
				/* Vcode support */
extern _tc_vspec_t _tc_localf (unsigned long flags, unsigned int type);
extern _tc_vspec_t _tc_localbf (unsigned int size);
extern _tc_vspec_t _tc_paramf (unsigned int type, unsigned int argno);

/* Dynamic compilation */
				/* Icode */
extern _tc_ip_t _tc_compile (_tc_cspec_t cs, int type);
				/* Vcode */
extern _tc_ip_t _tc_compilef (_tc_cspec_t cs, int type);
extern void _tc_decompile (_tc_code_t cp);

#endif
#endif

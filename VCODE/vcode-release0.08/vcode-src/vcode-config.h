#ifndef __SPARC_H__
#define __SPARC_H__

#define SPARC

/* We assume that this machine has mul/div in hardware. */
/* #define SPARC_NO_SYNTHETIC */

/*
 * Public interface.
 */

/*
 * Register definitions.  Are needed by vcode macros, hence public definition.  
 * g1 is reserved for use in loading large constants, etc.  f2,f3 are reserved 
 * for synthetic FP instructions. 
 */
enum {
	/* integer registers */
	_g0,    _g1,    _g2,    _g3,    _g4,    _g5,    _g6,    _g7, /* globals */
	_o0,    _o1,    _o2,    _o3,    _o4,    _o5,    _o6,    _o7, /* outs */
	_l0,    _l1,    _l2,    _l3,    _l4,    _l5,    _l6,    _l7, /* locals */
	_i0,    _i1,    _i2,    _i3,    _i4,    _i5,    _i6,    _i7, /* ins */

	_fp = _i6,	/* framepointer */
	_ra = _i7,	/* return address */
	_sp = _o6,	/* stack pointer */

	/* floating point */
        _f0=0,  _f1,    _f2,    _f3,    _f4,    _f5,    _f6,    _f7, /* floats */
        _f8,    _f9,    _f10,   _f11,   _f12,   _f13,   _f14,   _f15,
        _f16,   _f17,   _f18,   _f19,   _f20,   _f21,   _f22,   _f23,
        _f24,   _f25,   _f26,   _f27,   _f28,   _f29,   _f30,   _f31
	
};


#ifdef V_MAKE_REG_MAP
/* SPARC-specific map of symbolic register names to physical register names.  */
static struct reg_map {
        char *sym_name;      /* symbolic register name */
        int  phys_name;         /* physical register name */
} sym_to_phys[] = {
	{ "g0", _g0 },
	{ "g1", _g1 },
	{ "g2", _g2 },
	{ "g3", _g3 },
	{ "g4", _g4 },
	{ "g5", _g5 },
	{ "g6", _g6 },
	{ "g7", _g7 },
	{ "o0", _o0 },
	{ "o1", _o1 },
	{ "o2", _o2 },
	{ "o3", _o3 },
	{ "o4", _o4 },
	{ "o5", _o5 },
	{ "o6", _o6 },
	{ "o7", _o7 },
	{ "l0", _l0 },
	{ "l1", _l1 },
	{ "l2", _l2 },
	{ "l3", _l3 },
	{ "l4", _l4 },
	{ "l5", _l5 },
	{ "l6", _l6 },
	{ "l7", _l7 },
	{ "i0", _i0 },
	{ "i1", _i1 },
	{ "i2", _i2 },
	{ "i3", _i3 },
	{ "i4", _i4 },
	{ "i5", _i5 },
	{ "i6", _i6 },
	{ "fp", _fp },
	{ "ra", _ra },
	{ "sp", _sp },
	{ 0, 0}};
#endif

/* 
 * Uch.  If we do a cross-product of all types, naming these is going to 
 * be a royal pain.
 */
#define V_TI0	v_reg(_l4)
#define V_TI1	v_reg(_l5)
#define V_TI2	v_reg(_l6)
#define V_TI3	v_reg(_l7)

#define V_SI0	v_reg(_l0)
#define V_SI1	v_reg(_l1)
#define V_SI2	v_reg(_l2)
#define V_SI3	v_reg(_l3)

#define V_TF0   v_reg(_f4)
#define V_TF1   v_reg(_f6)
#define V_TF2   v_reg(_f8)
#define V_TF3   v_reg(_f10)

/* 
 * These function prototypes are used by vcode macros to synthesize
 * mul/div/mod.
 */
void v__mul(v_reg_type rd, v_reg_type rs1, v_reg_type rs2);
void v__div(v_reg_type rd, v_reg_type rs1, v_reg_type rs2);
void v__rem(v_reg_type rd, v_reg_type rs1, v_reg_type rs2);
void v__umul(v_reg_type rd, v_reg_type rs1, v_reg_type rs2);
void v__udiv(v_reg_type rd, v_reg_type rs1, v_reg_type rs2);
void v__urem(v_reg_type rd, v_reg_type rs1, v_reg_type rs2);

/* Site-specific call state. */
struct v_cstate {
#       define V_COOKIE 0xbabebabe
        unsigned cookie;
        short offset;
        short argno;
};

/*
 * Internal vcode information.
 */
#ifdef __VCODE_INTERNAL__
#include <stdarg.h>
#include <stdio.h>

/* Because of broken header files, we have to define these. */
extern int fflush(FILE *stream);
extern int fclose(FILE *stream);
#if !defined(_V_SOLARIS_)
/* isn't included in <stdio.h> */
extern int printf(char *, ...);
#endif
#if !defined(__LCC__) && !defined(_V_SOLARIS_)
extern int fprintf(FILE *stream, char *format, ...);
extern int vfprintf(FILE *stream, char *format, va_list ap);
#endif

#define V_MAXARGS       (10)            /* maximum allowed arguments */

#define IN      (R(_i0)| R(_i1)| R(_i2)| R(_i3)| R(_i4)| R(_i5)| R(_i6)| R(_i7))
#define OUT     (R(_o0)| R(_o1)| R(_o2)| R(_o3)| R(_o4)| R(_o5)| R(_o6)| R(_o7))
#define LOCAL   (R(_l0)| R(_l1)| R(_l2)| R(_l3)| R(_l4)| R(_l5)| R(_l6)| R(_l7))
#define GLOBAL  (R(_g2)|R(_g3)|R(_g4))  /* g5..g7 are not for us, p 192 */
#define FPREGS  (R(_f4)|R(_f6)|R(_f8)|R(_f10)|R(_f12)|R(_f14)|R(_f16)|R(_f18)|  \
                 R(_f20)|R(_f22)|R(_f24)|R(_f26)|R(_f28)|R(_f30))


/* These four must be defined for the machine-independent vcode backend. */
#define TEMPI (GLOBAL)
#define VARI  (LOCAL)
#define TEMPF (FPREGS)
#define VARF (0)

#endif

#endif /* __SPARC_H__ */

#ifndef __MIPS_H__
#define __MIPS_H__

#define MIPS

/* 
 * Machine specific information needed by machine independent vcode
 * generator.
 */

/*
 * Register definitions.  Are needed by vcode macros, hence public definition.
 * at is reserved for use in loading large constants, etc.  f0,f18 are reserved
 * for synthetic FP instructions.
 */
enum {  
	/* int registers */
        _r0, _r1, _r2, _r3, _r4, _r5, _r6, _r7, 
	_r8, _r9, _r10, _r11, _r12, _r13, _r14, _r15, 
	_r16, _r17, _r18, _r19, _r20, _r21, _r22, _r23, 
	_r24, _r25, _r26, _r27, _r28, _r29, _r30, _r31,

        /* int mnemonics */
        _zero = 0, _at=1, _v0, _v1, _a0, _a1, _a2, _a3, 
	_t0, _t1, _t2, _t3, _t4, _t5, _t6, _t7, 
	_s0, _s1, _s2, _s3, _s4, _s5, _s6, _s7, _t8, _t9, 
	_k0, _k1, _gp, _sp, _s8, _ra,

        /* floats */
        _f0=0, _f1, _f2, _f3, _f4, _f5, _f6, _f7, _f8, _f9, _f10, 
	_f11, _f12, _f13, _f14, _f15, _f16, _f17, _f18, _f19, _f20, 
	_f21, _f22, _f23, _f24, _f25, _f26, _f27, _f28, _f29, _f30, _f31,

	/* fp mnemonics */
	_dat = _f18
} ;

#define V_TI0	v_reg(_t0)
#define V_TI1	v_reg(_t1)
#define V_TI2	v_reg(_t2)
#define V_TI3	v_reg(_t3)
#define V_TI4	v_reg(_t4)
#define V_TI5	v_reg(_t5)


#define V_TF0   v_reg(_f4)
#define V_TF1   v_reg(_f6)
#define V_TF2   v_reg(_f8)


#ifdef V_MAKE_REG_MAP
/* MIPS-specific map of symbolic register names to physical register names.  */
static struct reg_map {
        char *sym_name;      /* symbolic register name */
        int  phys_name;         /* physical register name */
} sym_to_phys[] = {
        { "zero", _r0 },
        { "at", _at },
        { "v0", _v0 },
        { "v1", _v1 },
        { "a0", _a0 },
        { "a1", _a1 },
        { "a2", _a2 },
        { "a3", _a3 },
        { "t0", _t0 },
        { "t1", _t1 },
        { "t2", _t2 },
        { "t3", _t3 },
        { "t4", _t4 },
        { "t5", _t5 },
        { "t6", _t6 },
        { "t7", _t7 },
        { "t8", _t8 },
        { "t9", _t9 },
        { "s0", _s0 },
        { "s1", _s1 },
        { "s2", _s2 },
        { "s3", _s3 },
        { "s4", _s4 },
        { "s5", _s5 },
        { "s6", _s6 },
        { "s7", _s7 },
        { "s8", _s8 },
        { "k0", _k0 },
        { "k1", _k1 },
        { "gp", _gp },
        { "sp", _sp },
        { "ra", _ra },
        { 0, 0}};
#endif

/* Site-specific call state. */
struct v_cstate {
#	define V_COOKIE	0xbabebabe
	unsigned cookie;
        int ty0;
        short offset;
        short argno;
};

#ifdef __VCODE_INTERNAL__
#include <stdarg.h>
#include "cachectl.h"

#define V_MAXARGS       (10)            /* maximum allowed arguments */

#define __BASE_INT_TEMPS (R(_t0)|R(_t1)|R(_t2)|R(_t3)|R(_t4)|R(_t5)|R(_t6)|R(_t7)|R(_t8))


#ifdef ELF
	/* ELF linking uses t9. */
#	define TEMPI __BASE_INT_TEMPS
#else
	/* Otherwise include t9. */
#	define TEMPI (__BASE_INT_TEMPS|R(_t9))
#endif

#define N_VARS	9	/* 9 saved int registers (s0..s8) */
#define NFP_VARS 16	/* 16 (over-estimate) saved fp registers */

#define VARI (R(_s0)|R(_s1)|R(_s2)|R(_s3)|R(_s4)|R(_s5)|R(_s6)|R(_s7)|R(_s8))
#define TEMPF (R(_f4)|R(_f6)|R(_f8)|R(_f10)|R(_f16))
#define VARF (R(_f12)|R(_f14)|R(_f20)|R(_f22)|R(_f24)|R(_f26)|R(_f28)|R(_f30))

#endif /* __VCODE_INTERNAL__ */

#endif /* __MIPS_H__ */

#ifndef __ALPHA_H__
#define __ALPHA_H__

#define ALPHA

/* 
 * Machine specific information needed by machine independent vcode
 * generator.
 */

enum {  /* gp registers */
                _r0, _r1, _r2, _r3, _r4, _r5, _r6, _r7, _r8, _r9, _r10, _r11, 
		_r12, _r13,
                _r14, _r15, _r16, _r17, _r18, _r19, _r20, _r21, _r22, _r23, 
		_r24, _r25, _r26, _r27, _r28, _r29, _r30, _r31,
                /* software names */
		_v0 = 0, 	/* integer return register */
		_t0, _t1, _t2, _t3, _t4, _t5, _t6, _t7, 	/* temporaries */
		_s0, _s1, _s2, _s3, _s4, _s5, _s6,		/* saved regs */
		_a0, _a1, _a2, _a3, _a4, _a5,			/* argument regs */
		_t8, _t9, _t10, _t11, 				/* temps */
		_ra,						/* return reg */
		_t12,
		_at,						/* assembler temp */
		_gp,						/* global pointer */
		_sp,						/* stack pointer */
		_zero,						/* 0 */
		_fp = _s6,					/* frame pointer */

                /* floats */
                _f0=0, _f1, _f2, _f3, _f4, _f5, _f6, _f7, _f8, _f9, _f10, 
		_f11, _f12, _f13, _f14, _f15, _f16, _f17, _f18, _f19, _f20, 
		_f21, _f22, _f23, _f24, _f25, _f26, _f27, _f28, _f29, _f30, _f31,
		_dat = _f18,
		_fa0 = _f16,
		_fzero = _f31
} ;

/* Hardcoded registers */
#define V_TI0	v_reg(_t0)
#define V_TI1	v_reg(_t1)
#define V_TI2	v_reg(_t2)
#define V_TI3	v_reg(_t3)

#define V_TF0	v_reg(_f11)
#define V_TF1	v_reg(_f12)
#define V_TF2	v_reg(_f13)
#define V_TF3	v_reg(_f14)

/*
 * Register        Use 			Name
 * 
 * $f0-f1	Used to hold floating-point type function results ($f0) and 
 * 		complex  type function results ($f0 has the real part, $f1 has 
 * 		the imaginary part.  Not preserved across procedure calls.
 * 
 * $f2-f9 	Saved registers.  Preserved across procedure calls.
 * 
 * $f10-f15 	Temporary registers used for expression evaluation.  Not preserved 
 * 		across procedure calls.  
 * 
 * $f16-f21 	Used to pass the first six single or double precision actual 
 * 		arguments.  Not preserved across procedure calls.  
 * 
 * $f22-f30 	Temporary registers used for expression evaluations.  Not preserved 
 * 		across procedure calls.  
 * 
 * $f31 Always has the value 0.0.
 *
 * We reserve f10 to hold intermediate fp results.
 */

/* alpha has no division instruction */
void v__div(v_reg_t rd, v_reg_t rs1, v_reg_t rs2);
void v__rem(v_reg_type rd, v_reg_type rs1, v_reg_type rs2);
void v__udiv(v_reg_type rd, v_reg_type rs1, v_reg_type rs2);
void v__urem(v_reg_type rd, v_reg_type rs1, v_reg_type rs2);

#ifdef V_MAKE_REG_MAP
/* ALPHA-specific map of symbolic register names to physical register names.  */
static struct reg_map {
        char *sym_name;      /* symbolic register name */
        int  phys_name;         /* physical register name */
} sym_to_phys[] = {
        { "zero", _r0 },
        { "at", _at },
        { "v0", _v0 },
        { "a0", _a0 },
        { "a1", _a1 },
        { "a2", _a2 },
        { "a3", _a3 },
        { "a4", _a4 },
        { "a5", _a5 },
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
        { "t10", _t10 },
        { "t11", _t11 },
        { "t12", _t12 },
        { "s0", _s0 },
        { "s1", _s1 },
        { "s2", _s2 },
        { "s3", _s3 },
        { "s4", _s4 },
        { "s5", _s5 },
        { "s6", _s6 },
        { "gp", _gp },
        { "sp", _sp },
        { "ra", _ra },
        { 0, 0}};
#endif

/* Site-specific call state. */
struct v_cstate {
#       define V_COOKIE 0xbabebabe
        unsigned cookie;
        short offset;
        short argno;
};


#ifdef __VCODE_INTERNAL__
#include <stdarg.h>
#include "cachectl.h"

#ifndef V_MAXARGS
#	define V_MAXARGS       (10)            /* maximum allowed arguments */
#endif

#define TEMPI   (R(_t0)|R(_t1)|R(_t2)|R(_t3)|R(_t4)|R(_t5)|R(_t6)|R(_t7)\
                |R(_t8)|R(_t9)|R(_t10)|R(_t11)|R(_t12))

#define VARI   (R(_s0)|R(_s1)|R(_s2)|R(_s3)|R(_s4)|R(_s5)|R(_s6))

#define TEMPF   (R(_f11)|R(_f12)|R(_f13)|R(_f14)|R(_f15)|R(_f22)|R(_f23)\
                |R(_f23)|R(_f24)|R(_f25)|R(_f26)|R(_f27)|R(_f28)|R(_f29)|R(_f30))

#define VARF   (R(_f2)|R(_f3)|R(_f4)|R(_f5)|R(_f6)|R(_f7)|R(_f8)|R(_f9))


#define N_VARS	7	/* 7 saved int registers (s0..s6) */
#define N_FP_VARS 8	/* f2..f9 (over-estimate) saved fp registers */
#endif /* __VCODE_INTERNAL__ */

/* pseudo-instructions: we fake the prototypes */
extern void __divl();
extern void __divlu();
extern void __divq();
extern void __divqu();
extern void __reml();
extern void __remlu();
extern void __remq();
extern void __remqu();

extern void subli(int rd, int rs, int imm) ;
extern void subqi(int rd, int rs, int imm) ;
extern void addqi(int rd, int rs, int imm) ;
extern void addli(int rd, int rs, int imm) ;

void stus(int t0, int a0, long offset) ;
void stus(int rd, int rs, long offset) ;
void ldus(int rd, int rs, long offset) ;
void uldus(int rd, int rs, int offset) ;
void ulds(int rd, int rs, int offset) ;
void uldq(int rd, int rs, int offset) ;
void uldl(int rd, int rs, int offset) ;
void uldu(int rd, int rs, int offset) ;
void uldi(int rd, int rs, int offset) ;
void ulduc(int rd, int rs, int offset) ;
void uldc(int rd, int rs, int offset) ;
void ustq(int rd, int rs, int offset) ;
void usti(int rd, int rs, int offset) ;
void usts(int rd, int rs, int offset) ;
void ustb(int rd, int rs, int offset) ;

void aldus(int rd, int rs, long offset, long align);
void alds(int rd, int rs, long offset, long align);
void aldc(int rd, int rs, long offset, long align);
void alduc(int rd, int rs, long offset, long align);
void aldq(int rd, int rs, long offset, long align) ;
void aldl(int rd, int rs, long offset, long align) ;

#endif /* __ALPHA_H__ */

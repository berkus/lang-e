#ifndef __SIMPLE_SCALAR_H__
#define __SIMPLE_SCALAR_H__

#ifndef __SIMPLE_SCALAR__
#define __SIMPLE_SCALAR__
#endif

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
	_r32, _r33, _r34, _r35, _r36, _r37, _r38, _r39,
	_r40, _r41, _r42, _r43, _r44, _r45, _r46, _r47,
	_r48, _r49, _r50, _r51, _r52, _r53, _r54, _r55,
	_r56, _r57, _r58, _r59, _r60, _r61, _r62, _r63,
	_r64, _r65, _r66, _r67, _r68, _r69, _r70, _r71,
	_r72, _r73, _r74, _r75, _r76, _r77, _r78, _r79,
	_r80, _r81, _r82, _r83, _r84, _r85, _r86, _r87,
	_r88, _r89, _r90, _r91, _r92, _r93, _r94, _r95,
	_r96, _r97, _r98, _r99, _r100, _r101, _r102, _r103,
	_r104, _r105, _r106, _r107, _r108, _r109, _r110, _r111,
	_r112, _r113, _r114, _r115, _r116, _r117, _r118, _r119,
	_r120, _r121, _r122, _r123, _r124, _r125, _r126, _r127,
	_r128, _r129, _r130, _r131, _r132, _r133, _r134, _r135,
	_r136, _r137, _r138, _r139, _r140, _r141, _r142, _r143,
	_r144, _r145, _r146, _r147, _r148, _r149, _r150, _r151,
	_r152, _r153, _r154, _r155, _r156, _r157, _r158, _r159,
	_r160, _r161, _r162, _r163, _r164, _r165, _r166, _r167,
	_r168, _r169, _r170, _r171, _r172, _r173, _r174, _r175,
	_r176, _r177, _r178, _r179, _r180, _r181, _r182, _r183,
	_r184, _r185, _r186, _r187, _r188, _r189, _r190, _r191,
	_r192, _r193, _r194, _r195, _r196, _r197, _r198, _r199,
	_r200, _r201, _r202, _r203, _r204, _r205, _r206, _r207,
	_r208, _r209, _r210, _r211, _r212, _r213, _r214, _r215,
	_r216, _r217, _r218, _r219, _r220, _r221, _r222, _r223,
	_r224, _r225, _r226, _r227, _r228, _r229, _r230, _r231,
	_r232, _r233, _r234, _r235, _r236, _r237, _r238, _r239,
	_r240, _r241, _r242, _r243, _r244, _r245, _r246, _r247,
	_r248, _r249, _r250, _r251, _r252, _r253, _r254, _r255,
        /* int mnemonics */
        _zero = 0, _at=1, _v0, _v1, _a0, _a1, _a2, _a3, 
	_t0, _t1, _t2, _t3, _t4, _t5, _t6, _t7, 
	_s0, _s1, _s2, _s3, _s4, _s5, _s6, _s7, _t8, _t9, 
	_k0, _k1, _gp, _sp, _s8, _ra,
        /* floats */
        _f0=0, _f1, _f2, _f3, _f4, _f5, _f6, _f7, _f8, _f9, _f10, 
	_f11, _f12, _f13, _f14, _f15, _f16, _f17, _f18, _f19, _f20, 
	_f21, _f22, _f23, _f24, _f25, _f26, _f27, _f28, _f29, _f30, _f31,
	_f32, _f33, _f34, _f35, _f36, _f37, _f38, _f39,
	_f40, _f41, _f42, _f43, _f44, _f45, _f46, _f47,
	_f48, _f49, _f50, _f51, _f52, _f53, _f54, _f55,
	_f56, _f57, _f58, _f59, _f60, _f61, _f62, _f63,
	_f64, _f65, _f66, _f67, _f68, _f69, _f70, _f71,
	_f72, _f73, _f74, _f75, _f76, _f77, _f78, _f79,
	_f80, _f81, _f82, _f83, _f84, _f85, _f86, _f87,
	_f88, _f89, _f90, _f91, _f92, _f93, _f94, _f95,
	_f96, _f97, _f98, _f99, _f100, _f101, _f102, _f103,
	_f104, _f105, _f106, _f107, _f108, _f109, _f110, _f111,
	_f112, _f113, _f114, _f115, _f116, _f117, _f118, _f119,
	_f120, _f121, _f122, _f123, _f124, _f125, _f126, _f127,
	_f128, _f129, _f130, _f131, _f132, _f133, _f134, _f135,
	_f136, _f137, _f138, _f139, _f140, _f141, _f142, _f143,
	_f144, _f145, _f146, _f147, _f148, _f149, _f150, _f151,
	_f152, _f153, _f154, _f155, _f156, _f157, _f158, _f159,
	_f160, _f161, _f162, _f163, _f164, _f165, _f166, _f167,
	_f168, _f169, _f170, _f171, _f172, _f173, _f174, _f175,
	_f176, _f177, _f178, _f179, _f180, _f181, _f182, _f183,
	_f184, _f185, _f186, _f187, _f188, _f189, _f190, _f191,
	_f192, _f193, _f194, _f195, _f196, _f197, _f198, _f199,
	_f200, _f201, _f202, _f203, _f204, _f205, _f206, _f207,
	_f208, _f209, _f210, _f211, _f212, _f213, _f214, _f215,
	_f216, _f217, _f218, _f219, _f220, _f221, _f222, _f223,
	_f224, _f225, _f226, _f227, _f228, _f229, _f230, _f231,
	_f232, _f233, _f234, _f235, _f236, _f237, _f238, _f239,
	_f240, _f241, _f242, _f243, _f244, _f245, _f246, _f247,
	_f248, _f249, _f250, _f251, _f252, _f253, _f254, _f255,
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
	/* ignore symbolic names for other regs for now */
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

#define R(x) (1<<(x))

#define _TEMPI \
	(R(_t0)|R(_t1)|R(_t2)|R(_t3)|R(_t4)|R(_t5)|R(_t6)|R(_t7)|R(_t8)|R(_t9))
#define _VARI \
	(R(_s0)|R(_s1)|R(_s2)|R(_s3)|R(_s4)|R(_s5)|R(_s6)|R(_s7)|R(_s8))
#define _TEMPF (R(_f4)|R(_f6)|R(_f8)|R(_f10)|R(_f16))
#define _VARF (R(_f12)|R(_f14)|R(_f20)|R(_f22)|R(_f24)|R(_f26)|R(_f28)|R(_f30))

#define N_VARS	9+32	/* 9 saved int registers (s0..s8) */
#define NFP_VARS 16	/* 16 (over-estimate) saved fp registers */

#define VARI \
  { _VARI, 	0xffffffff, 0x00000000, 0x00000000,             \
    0x00000000, 0x00000000, 0x00000000, 0x00000000 }

#define TEMPI \
  { _TEMPI, 	0x00000000, 0xffffffff, 0xffffffff,             \
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff }

#define VARF \
  { _VARF, 	0x00000000, 0x00000000, 0x00000000,		\
    0x00000000, 0x00000000, 0x00000000, 0x00000000 }

#define TEMPF \
  { _TEMPF, 	0xffffffff, 0xffffffff, 0xffffffff,             \
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff }

#define MAX_REGS 256

#endif /* __VCODE_INTERNAL__ */

#endif /* __SIMPLE_SCALAR_H__ */

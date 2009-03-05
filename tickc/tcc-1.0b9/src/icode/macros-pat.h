/* $Id: macros-pat.h,v 1.3 1998/05/17 20:38:38 maxp Exp $ */

#ifndef __MACROS_GEN_H__
#define __MACROS_GEN_H__

/*
 * Macros for generating icode instructions
 */

/*
   Icode instructions have 2 encodings:
        63 60        50        40        30        20        10        0
	xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    	[ opcode ][  rd     ][  rs1    ][  rs2    ][  garbage	       ]
	opcode: 10 bits
	rd,rs1,rs2: 11 bits each

        63 60        50        40        30        20        10        0
	xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    	[ opcode ][  rd     ][  rs     ][  immediate         	       ]
	opcode: 10 bits
	rd,rs: 11 bits each
	immediate: 32 bits
*/

#define set_op(_op) (((unsigned)(_op)&0x000003FF)<<22)
#define get_op(_is) (((unsigned)(*(_is))&0xFFC00000)>>22)

#define set_rd(_rd) (((unsigned)(_rd)&0x000007FF)<<11)
#define get_rd(_is) (((unsigned)(*(_is))&0x003FF800)>>11)

#define set_rs(_rs) (((unsigned)(_rs)&0x000007FF))
#define get_rs(_is) (((unsigned)(*(_is))&0x000007FF))

#define set_rs2(_rs) (((unsigned)(_rs)&0x000007FF)<<21)
#define get_rs2(_is) (((unsigned)(*(_is+1))&0xFFE00000)>>21)

#define set_imm(_im) ((unsigned)(_im))
#define get_imm(_is) ((unsigned)(*(_is+1)))

#define get_isav(_is) ((void *)(*(_is+2)))
#define get_fsav(_is) ((void *)(*(_is+3)))

#define f_bop(op, rd, rs1, rs2) do {				\
     *i_ip++ = set_op(op)|set_rd(rd)|set_rs(rs1);		\
     *i_ip++ = set_rs2(rs2);					\
     REFCNT(rd) += i_refc; REFCNT(rs1) += i_refc; REFCNT(rs2) += i_refc; \
} while (0)
#define f_bopi(op, rd, rs, imm) do {				\
     *i_ip++ = set_op(op)|set_rd(rd)|set_rs(rs);		\
     *i_ip++ = set_imm(imm);					\
     REFCNT(rd) += i_refc; REFCNT(rs) += i_refc;		\
} while (0)

#define f_set(op, rd, imm) do {					\
     *i_ip++ = set_op(op)|set_rd(rd);				\
     *i_ip++ = set_imm(imm);					\
     REFCNT(rd) += i_refc;					\
} while (0)

#define f_uop(op, rd, rs) do { 					\
     *i_ip = set_op(op)|set_rd(rd)|set_rs(rs); i_ip += i_isize;	\
     REFCNT(rd) += i_refc; REFCNT(rs) += i_refc;		\
} while (0)
#define f_uopi(op, rd, imm) f_set(op, rd, imm)

#define f_mopr(op, rd, rs, offset) f_bop(op, rd, rs, offset)
#define f_mopri(op, rd, rs, offset) f_bopi(op, rd, rs, offset)
#define f_mopw(op, rd, rs, offset) f_mopr(op, rd, rs, offset)
#define f_mopwi(op, rd, rs, offset) f_mopri(op, rd, rs, offset)

#define f_ret(op, val) do {					\
     *i_ip = set_op(op)|set_rd(val); i_ip += i_isize;		\
     REFCNT(val) += i_refc;					\
     ++i_nbb;							\
} while (0)
#define f_reti(op, val) do {					\
     *i_ip++ = set_op(op); *i_ip++ = set_imm(val);		\
     ++i_nbb;							\
} while (0)

#define f_jmp(op, rdst) f_ret(op, rdst)
#define f_jmpi(op, idst) f_reti(op, idst)

#define f_br(op, rs1, rs2, label) do {				\
     /* Warning: label cannot exceed 11 bits */			\
     *i_ip++ = set_op(op)|set_rd(label)|set_rs(rs1);		\
     *i_ip++ = set_rs2(rs2);					\
     ++i_nbb;							\
     REFCNT(rs1) += i_refc; REFCNT(rs2) += i_refc;		\
} while (0)

#define f_bri(op, rs1, is2, label) do {				\
     /* Warning: label cannot exceed 11 bits */			\
     *i_ip++ = set_op(op)|set_rd(label)|set_rs(rs1);		\
     *i_ip++ = set_imm(is2);					\
     ++i_nbb;							\
     REFCNT(rs1) += i_refc;					\
} while (0)

#define f_call(op, result, callee) do {				\
     *i_ip = set_op(op)|set_rd(result)|set_rs(callee);		\
     *(i_ip+i_isize) = 0; *(i_ip+i_isize+1) = 0;		\
     i_ip += (i_isize*2); /* 2 slots: store live var info */    \
     ++i_nbb; i_leafp = 0;					\
     REFCNT(result) += i_refc; REFCNT(callee) += i_refc;	\
} while (0)

#define f_calli(op, result, callee) do {			\
     *i_ip = set_op(op)|set_rd(result);				\
     *(i_ip+1) = set_imm(callee);				\
     *(i_ip+i_isize) = 0; *(i_ip+i_isize+1) = 0;		\
     i_ip += (i_isize*2); /* 2 slots: store live var info */    \
     ++i_nbb; i_leafp = 0;					\
     REFCNT(result) += i_refc;					\
} while (0)

#define f_arg(op, arg) do {					\
     *i_ip = set_op(op)|set_rd(arg); i_ip += i_isize;		\
     REFCNT(arg) += i_refc;					\
} while (0)

				/* Load effective address */
#define f_lea(op, dst, src) do {				\
     *i_ip = set_op(op)|set_rd(dst)|set_rs(src);		\
     i_ip += i_isize;						\
     REFCNT(dst) += i_refc; REFCNT(src) += i_refc;		\
} while (0)

/*
 * Oddball instructions
 */
				/* No-op */
#define i_nop() do {						\
     *i_ip = 0; i_ip += i_size;					\
} while (0)
				/* rd <- addr of dynamic code */
#define i_self(rd) do {						\
     *i_ip = set_op(i_op_self)|set_rd(rd); i_ip += i_isize;	\
     REFCNT(rd) += i_refc;					\
} while (0)
				/* Label */
#define i_label(label) do {					\
     ++i_nbb;							\
     *i_ip = set_op(i_op_lbl)|set_rd(label); i_ip += i_isize;	\
} while (0)
				/* Void return */
#define i_retv() do {						\
     *i_ip = set_op(i_op_retv); i_ip += i_isize;		\
     ++i_nbb;							\
} while (0)
				/* Void call */
#define i_callv(callee) do {					\
     *i_ip = set_op(i_op_callv)|set_rs(callee);			\
     *(i_ip+i_isize) = 0; *(i_ip+i_isize+1) = 0;		\
     i_ip += (i_isize*2); /* 2 slots: store live var info */    \
     ++i_nbb; i_leafp = 0;					\
     REFCNT(callee) += i_refc;					\
} while (0)
				/* Void call immediate */
#define i_callvi(callee) do {					\
     *i_ip = set_op(i_op_callvi);				\
     *(i_ip+1) = set_imm(callee);				\
     *(i_ip+i_isize) = 0; *(i_ip+i_isize+1) = 0;		\
     i_ip += (i_isize*2); /* 2 slots: store live var info */    \
     ++i_nbb; i_leafp = 0;					\
} while (0)

#define i_setf(rd,imm) do {					\
     *i_ip++ = set_op(i_op_setf)|set_rd(rd);			\
     *i_ip++ = set_imm(i_fpi_addf(imm));			\
     REFCNT(rd) += i_refc;					\
} while (0)
#define i_setd(rd,imm) do {					\
     i_fpi_addd(imm);						\
     *i_ip++ = set_op(i_op_setd)|set_rd(rd);			\
     *i_ip++ = set_imm(i_fpi_addd(imm));			\
     REFCNT(rd) += i_refc;					\
} while (0)
				/* Set reference weights */
#define i_refmul(wt) do { i_refc *= (float)wt; } while (0)
#define i_refdiv(wt) do { i_refc /= (float)wt; } while (0)

#endif

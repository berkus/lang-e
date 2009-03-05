#include "demand.h"
#include "binary.h"

extern void v_jalpi(v_reg_type r, void *ip);
extern void cvd2l(int rd, int rs);
extern void cvd2i(int rd, int rs);
extern void cvl2f(int rd, int rs);
extern void cvl2d(int rd, int rs);

/* Need to fix the damn preprocessor. */
#define v_alduci(rd, rs, offset, align)	\
	alduc(_vrr(rd), _vrr(rs), offset, align)

#define v_aldci(rd, rs, offset, align)	\
	aldc(_vrr(rd), _vrr(rs), offset, align)

#define v_aldsi(rd, rs, offset, align)	\
	alds(_vrr(rd), _vrr(rs), offset, align)

#define v_aldusi(rd, rs, offset, align)	\
	aldus(_vrr(rd), _vrr(rs), offset, align)

#define v_aldui(rd, rs, offset, align)	\
	aldl(_vrr(rd), _vrr(rs), offset, align)
#define v_aldi	v_aldui

#define v_aldli(rd, rs, offset, align)	\
	aldq(_vrr(rd), _vrr(rs), offset, align)
#define v_alduli	v_aldli
#define v_aldpi	v_aldli

/* Only do a jump-subroutine when we are actually going to do a call */
#define v_jalp(r, dst) do {				\
	if(_vrr(r) == _ra) {				\
	     jsr(_vrr(r), _vrr(dst), 0);		\
	} else {					\
		jmp_link(_vrr(r), _vrr(dst)); 		\
	}						\
} while(0)

#define v_jpi(dst)       br(_zero, (unsigned)(dst))
#define v_jp(dst)        jmp_nolink(_vrr(dst))
/* Inverted order of jpi and jmark to make bpo work correctly */
#define v_jv(l)		do { v_jpi(0); v_jmark(v_ip, l); } while(0)
#define v_retv() 	v_jv(v_epilogue_l)

#define v_setp(r, imm)	v_setul(r, (unsigned long)imm)

#define v_nop() nop()

extern v_label_type v_epilogue_l;
extern void stus(int rd, int rs, long offset);
extern void ldus(int rd, int rs, long offset);

/* v_schedule_delay: alpha has no delay slots.  Place in order */
#define v_schedule_delay(branch, insn) do {  insn; branch; } while(0)

/* v_raw_load: alpha does not have delay slots. */
#define v_raw_load(mem, n) mem
#define v_nuke_nop()

extern void pseudo_call(v_vptr insn, int rd, int rs1, int rs2);

#define v_setf(rd, imm) v_float_imm(rd, imm)
#define v_setd(rd, imm) v_double_imm(rd, imm)

#define v_retfi(imm) do { v_setf(v_fp_rr, imm); v_retv(); } while(0)
#define v_retdi(imm) do { v_setd(v_fp_rr, imm); v_retv(); } while(0)

%%
(ret ((@ v_callee_int_rr, rs_or_imm) (v_jv v_epilogue_l)) (i l u ul p  v_mov: v_set:))
(ret ((@ v_fp_rr, rs) (v_jv v_epilogue_l)) (f d v_mov:))

(add (i u addl addli) (l ul p addq addqi) (f adds) (d addt))
(sub (i u subl subli) (l ul p subq subqi) (f subs) (d subt))

(mul (s i u mull mulli) (l ul mulq mulqi) (f muls) (d mult))

; The great thing about alpha pseudo-insns is that they 
; save temp registers.

(div (s i ((pseudo_call __divl, rd, rs1, rs2)))
	(u ((pseudo_call __divlu, rd, rs1, rs2)))
	(l ((pseudo_call __divq, rd, rs1, rs2)))
	(ul ((pseudo_call __divqu, rd, rs1, rs2)))
	(f divs) (d divt))
(mod (s i ((pseudo_call __reml, rd, rs1, rs2)))
	(u ((pseudo_call __remlu, rd, rs1, rs2)))
	(l ((pseudo_call __remq, rd, rs1, rs2)))
	(ul ((pseudo_call __remqu, rd, rs1, rs2))))

(def no_reg_plus_reg)
(ld (c uldc) (uc ulduc) (s ulds) (us uldus) 
	(i u ldl)
	(ul l p ldq)
	(f lds) (d ldt))
(st (c uc ustb) (s us usts)
	(i u stl)
	(ul l p stq) 
	(f sts) (d stt))

(uld (s ulds) (us uldus) 
	(i u uldl)
	(ul l p uldq))

(ust (s us usts)
	(i u ustl)
	(ul l p ustq) )

; sort of boolean ops
(lsh
        (i
		; sign extension
                ((sll rd, rs1, rs2) (addl rd, _zero, rd))
                ((slli rd, rs1, imm) (addl rd, _zero, rd)))
        (u
                ((zapnoti rd, rs1, 0xf) (sll rd, rd, rs2))
                ((zapnoti rd, rs1, 0xf) (slli rd, rd, imm)))
        (l ul sll slli))

(rsh (i l sra srai) (ul srl srli)
	(u 
		((zapnoti rd, rs1, 0xf) (srl rd, rd, rs2))
		((zapnoti rd, rs1, 0xf) (srli rd, rd, imm))))

(and (us s i l u ul and andi))
(or (us s i l u ul bis bisi))
(xor (us s i l u ul xor xori))

(nor (i u l ul ( (v_or: rd, rs1, rs2) (v_com: rd, rd) ) ))
(nxor (i u l ul ((v_xor: rd, rs1, rs2) (v_com: rd, rd))))
(nand (i u l ul ((v_and: rd, rs1, rs2) (v_com: rd, rd))))

(andnot (i u l ul andnot andnoti))
(ornot (i u l ul ornot ornoti))
(xornot (i u l ul xornot xornoti))

(com (i l u ul com))

(neg 
	(i u ((subl rd, _zero, rs))) 
	(l ul ((subq rd, _zero, rs)))
  	(f d fneg))
(not (i l u ul not))
(mov (i l u ul p mov) (f d fmov))

(set (c uc s us i u l set) (ul setu))

(abs (f fabss) (d fabsd))
(nabs (f d ((v_abs: rd, rs) (v_neg: rd, rd))))
(sqrt (f fsqrts) (d fsqrtd))

; some useful macros
(def sext24 ((v_lshui rd,rs,24) (v_rshii rd,rd,24)))
(def sext16 ((v_lshui rd,rs,16) (v_rshii rd,rd,16)))
(def trunc16 ((v_andii rd,rd,0xffff)))
; Uch
; (def f2l ((fstoi rs, rs) (stf rs, v_lpr, v_carea) (ld rd, v_lpr, v_carea)))
; (def d2l ((fdtoi rs, rs) (stf rs, v_lpr, v_carea) (ld rd, v_lpr, v_carea)))
; (def i2f ((st rs, v_lpr, v_carea) (ldf rd, v_lpr, v_carea)))
; (def i2d ((std rs, v_lpr, v_carea) (lddf rd, v_lpr, v_carea)))

(cvc2 (i u l ul sext24))
(cvs2 (i u l ul sext16))
(cvus2 (i u l ul trunc16))
(cvi2 (c sext24) (s sext16) (u l ul mov))
(cvu2 (c sext24) (s sext16) (i mov) (l ul ((zapnoti rd, rs, 0xf))))
(cvl2 (c sext24) (s sext16) (i u ((addl rd, rs, _zero))) (ul mov) (f cvl2f) (d cvl2d))
(cvul2 (c sext24) (s sext16) (u i ((addl rd, rs, _zero))) (l p mov))
(cvp2 (ul mov))	
(cvf2 (d ((addt rd, rs, _fzero))) (l cvd2l))
(cvd2 (f ((cvtts rd, rs))) (l cvd2l))

(blt (i l cmplq cmplqi) (u cmpluq cmpluqi) (ul p cmpluq cmplulqi) (f d bltf))
(bge (i l cmpgeq cmpgeqi) (u cmpgeuq cmpgeuqi) (ul p cmpgeuq cmpgeulqi) (f d bgef))
(ble (i l cmpleq cmpleqi) (u cmpleuq cmpleuqi) (ul p cmpleuq cmpleulqi) (f d blef))
(bgt (i l cmpgtq cmpgtqi) (u cmpgtuq cmpgtuqi) (ul p cmpgtuq cmpgtulqi) (f d bgtf))
(beq (i u cmpeql cmpeqli) (ul l p cmpeqq cmpeqqi) (f d beqf))
(bne (i u cmpnel cmpneli) (l ul p cmpneq cmpneqi) (f d bnef))

(muladd ((v_mul: v_dat, rs1, rs2) (v_add: rd, v_dat, rs3)) (f) (d))
(mulsub ((v_mul: v_dat, rs1, rs2) (v_sub: rd, v_dat, rs3)) (f) (d))
(negmuladd ((v_mul: v_dat, rs1, rs2) (v_add: rd, v_dat, rs3) (v_neg: rd,rd)) (f) (d))
(negmulsub ((v_mul: v_dat, rs1, rs2) (v_sub: rd, v_dat, rs3) (v_neg: rd,rd)) (f) (d))

#include "demand.h"
#include "binary.h"

extern int v_carea;

#define v_jpi(dst)       do { jmpi((unsigned)(dst)); v_nop(); } while(0)
#define v_jp(dst)        do { jmp(_vrr(dst)); v_nop(); } while(0)
#define v_jv(l)          do { ba(_vlr(l)); } while(0)

#define v_nop() { (*v_ip++) = NOP; }
#define v_interlock()	/* hardware interlock */
#define v_delay_slot()	v_nop()

/* Need to add jalv */
#define v_jalp(dst, target) do { 		\
	unsigned _rdst = _vrr(dst);		\
						\
	jmpl(_rdst, _vrr(target), 0);		\
	addi(_rdst, _rdst, 8);			\
} while(0)

#define v_jalpi(dst, target) do { 		\
	unsigned _rdst = _vrr(dst);		\
						\
	jmpl(_rdst, _g0, target);		\
	addi(_rdst, _rdst, 8);			\
} while(0)

/*
 * Get rid of a nop.
 */
#define v_nuke_nop() do {                               \
        if(v_ip[-1] == NOP)                             \
                v_ip--;                                 \
} while(0)


/* 
 * Assumption: if macro2 is only one instruction, it cannot use
 * any registers that would screw with the branch delayed instruction
 * (macro1), in this case we can safely insert macro2 in the branch
 * delay slot.  A more sophisticated back end would use a more
 * aggressive strategy.
 * 
 */
#define v_schedule_delay(branch, insn) do {		\
	v_code *_ip, _insn;				\
 							\
	_ip = v_ip;					\
	insn;						\
	/* cannot safely fill the delay slot. */	\
	if((v_ip - _ip) > 1) {				\
		branch;					\
	} else {					\
		/* avoid double eval of macro1 */	\
		_insn = *_ip;				\
		v_ip = _ip;				\
		branch;					\
		if(v_ip[-1] != NOP)			\
			v_fatal("v_schedule_delay: insn is not a branch.");\
		v_ip[-1] = _insn; /* overwrite nop */	\
	}						\
} while(0)

/* simply instantiate execute the macro */
#define v_raw_load(macro, n) macro

#define v_retv()  do { ret(); restore(); } while(0)
#define v_setp(rd, imm) v_setu(rd, (unsigned)imm)
#define v_setf(rd, imm) v_float_imm(rd, imm)
#define v_setd(rd, imm) v_double_imm(rd, imm)
#define v_retfi(imm) do { v_setf(v_fp_rr, imm); v_retv(); } while(0)
#define v_retdi(imm) do { v_setd(v_fp_rr, imm); v_retv(); } while(0)
%%
;
; Core vcode instruction set.
;
(ret ((@ v_callee_int_rr, rs_or_imm) (ret) (restore)) (i l u ul p  v_mov: v_set:))
(ret ((@ v_fp_rr, rs) (ret) (restore)) (f d v_mov:))


(ld (c ldsbr ldsb) (uc ldubr ldub) (s ldshr ldsh) (us lduhr lduh) (i u ul l p ldr ld) 
	(f ldfr ldf) (d lddfr lddf))
(st (c uc stbr stb) (s us sthr sth) (i u ul l p str st) (f stfr stf) (d stdfr stdf))

; sort of boolean ops
(lsh (i l u ul sll slli))
(rsh (i l sra srai) (u ul srl srli))

(and (us s i l u ul and andi))
(or (us s i l u ul or ori))
(xor (us s i l u ul xor xori))
(not (i l u ul not))
(com (i l u ul com))

; Standard arithmatic operations.
(add (i u l ul p add addi) (f fadds) (d faddd))  	
(sub (i u l ul p sub subi) (f fsubs) (d fsubd))
(mul (s i l v__mul) (us u ul v__umul) (f fmuls) (d fmuld))
(div (s i l v__div)  (us u ul v__udiv) (f fdivs) (d fdivd))
(mod (s i l v__rem)  (us u ul v__urem))
(neg (i l u ul neg)  (f fnegs) (d fnegd))

(mov (i l u ul p mov) (f fmovs) (d fmovd))
(set (c uc us s i l u ul set))


; Branches
(beq (i u l ul p be bei) (f fbes) (d fbed))
(bne (i u l ul p bne bnei) (f fbnes) (d fbned))
(blt (i l bl bli) (u ul p ((bgu rs2, rs1, label))) (f fbls) (d fbld))
(ble (i l ble blei) (u ul p bleu bleui) (f fbles) (d fbled))
(bgt (i l bg bgi) (u ul p bgu bgui)  (f fbgs) (d fbgd))
(bge (i l bge bgei) (u ul p ((bleu rs2, rs1, label))) (f fbges) (d fbged))

;
; Primary integer extensions to the vcode instruction set.
;

; Conversions.

; some useful macros
(def sext24 ((v_lshui rd,rs,24) (v_rshii rd,rd,24)))
(def sext16 ((v_lshui rd,rs,16) (v_rshii rd,rd,16)))
(def trunc16 ((v_andii rd,rd,0xffff)))
; Uch
(def f2i ((fstoi rs, rs) (stf rs, v_lpr, v_carea) (ld rd, v_lpr, v_carea)))
(def d2i ((fdtoi rs, rs) (stf rs, v_lpr, v_carea) (ld rd, v_lpr, v_carea)))
(def i2f ((st rs, v_lpr, v_carea) (ldf rd, v_lpr, v_carea) (fitos rd, rd)))
(def i2d ((std rs, v_lpr, v_carea) (lddf rd, v_lpr, v_carea) (fitod rd, rd)))

; Necessary conversions.
(cvp2 (ul mov))	
(cvf2 (d fstod) (i u l ul f2i))
(cvd2 (f fdtos) (i u l ul d2i))

(cvc2 (i u l ul sext24))
(cvs2 (i u l ul sext16))
(cvus2 (i u l ul trunc16))
(cvi2 (c sext24) (s sext16) (u l ul mov) (f i2f) (d i2d))
(cvu2 (c sext24) (s sext16) (i l ul mov) (f i2f) (d i2d))
(cvl2 (c sext24) (s sext16) (i u ul mov) (f i2f) (d i2d))
(cvul2 (c sext24) (s sext16) (i u l p mov) (f i2f) (d i2d))

; Logical extensions
(nor (i u l ul ( (v_or: rd, rs1, rs2) (v_com: rd, rd) ) ))
(nxor (i u l ul xnor xnori))
(nand (i u l ul ((v_and: rd, rs1, rs2) (v_com: rd, rd))))
(andnot (i u l ul andn andni))
(ornot (i u l ul orn orni))
(xornot (i u l ul ((v_com: v_at, rs2) (v_xor: rd, rs1, v_at))))

; Conditional move extensions.
(def synthetic_cmv)
(cmveq (i u ul l p))
(cmvne (i u ul l p))
(cmvlt (i u ul l p))
(cmvle (i u ul l p))
(cmvgt (i u ul l p))
(cmvge (i u ul l p))

;
; Primary floating point extensions to the vcode instruction set.
;

(nabs (f d ((v_abs: rd, rs) (v_neg: rd, rd))))
(sqrt (f fsqrts) (d fsqrtd))
(abs (f fabss) (d fabsd))


; secondary fp extensions
(muladd ((v_mul: v_dat, rs1, rs2) (v_add: rd, v_dat, rs3)) (f) (d))
(mulsub ((v_mul: v_dat, rs1, rs2) (v_sub: rd, v_dat, rs3)) (f) (d))
(negmuladd ((v_mul: v_dat, rs1, rs2) (v_add: rd, v_dat, rs3) (v_neg: rd,rd)) (f) (d))
(negmulsub ((v_mul: v_dat, rs1, rs2) (v_sub: rd, v_dat, rs3) (v_neg: rd,rd)) (f) (d))

; MISSING FUNCTIONALITY (some of it)
; 	(def uload ((lwl rd,rs1,imm) (lwr rd, rs1, imm)))
; 		uck a bit painful
; 	(uld s i l ul u p)
; 	(ust s i l ul u p)
; still need to swap bytes now
;
; (mulhi ((@ rs1,rs2) (mfhi rd)) (s i l mult) (us u ul multu) )
; (mulhilo ((@ rs1,rs2) (mflo rd1) (mfhi rd2)) (i l u ul mul) )
; (divmod  ((@ rs1,rs2) (mflo rd1) (mfhi rd2)) (i l u ul div) )
; (mod ((@ rs1,rs2) (mfhi rd)) (s i l div)  (us u ul divu) )
;
; Conditional expressions Table 9
; (lt  (i l slt slti) (u ul p sltu sltiu) )
; (le  (i u p l ul ((v_lt: rd, rs2, rs1) (v_not: rd, rd))))
; (ge  (i u p l ul ((v_lt: rd, rs1, rs2) (v_not: rd, rd))))
; (gt  (i u p l ul ((v_lt: rd, rs2, rs1))))
; (eq  (i u p l ul ((v_xor: rd, rs1, rs2) (v_ltui rd, rd, 1))))
; (ne  (i u p l ul ((v_xor: rd, rs1, rs2) (v_ltu rd, v_zero, rd))))
;
; (ceil (f ceils) (d ceild))
; (floor (f floors) (d floord))
;
; move between fp and int (used for argument passing) (movd2r movf2r movr2d movr2f)

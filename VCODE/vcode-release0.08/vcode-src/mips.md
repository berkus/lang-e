#include "demand.h"
#include "binary.h"

#define uld(rd, rs1, imm) do {			\
        lwl(rd,rs1,imm+3);           			\
        lwr(rd,rs1,imm);           			\
} while(0)

#define ust(rd, rs1, imm) do {			\
        swl(rd,rs1,imm+3);			        \
        swr(rd,rs1,imm);           			\
} while(0)

/* need to add jalv */
#define v_jalpi(dst, target)   do { 	\
	if(_vrr(dst) == _ra)		\
		jal((unsigned)target);	\
	else {				\
		v_setp(v_at, target);	\
		v_jalp(dst, v_at);	\
	}				\
	v_nop();			\
} while(0)

#define v_jalp(dst, target) do { 		\
	jalr(_vrr(target), _vrr(dst)); 		\
	v_nop(); 				\
} while(0)

#define v_jpi(dst)       do { j((unsigned)(dst)); v_nop(); } while(0)
#define v_jp(dst)        do { jr(_vrr(dst)); v_nop(); } while(0)
#define v_jv(l)		do { v_jmark(v_ip, l); v_nop(); v_nop(); } while(0)

#define v_nop() nop()
#define v_retv()  do { v_nop(); v_jv(v_epilogue_l); v_nuke_nop(); } while(0)
#define v_setf(rd, imm) v_float_imm(rd, imm)
#define v_setp(rd, imm) v_setul(rd, (unsigned long)imm)
#define v_setd(rd, imm) v_double_imm(rd, imm)
#define v_retfi(imm) do { v_setf(v_fp_rr, imm); v_retv(); } while(0)
#define v_retdi(imm) do { v_setd(v_fp_rr, imm); v_retv(); } while(0)

#define v_setv(rd, l) do { v_smark(v_ip, l, rd); v_nop(); v_nop(); } while(0)

extern void cvtd2w(unsigned dst, unsigned src) ;
extern void cvts2w(unsigned dst, unsigned src) ;

extern v_label_type v_epilogue_l;

/*
 * Get rid of a nop.
 */
#if 0
#define v_nuke_nop() do {				\
	demand(v_ip[-1] == NOP, bogus nop!);		\
	v_ip--;						\
} while(0)
#endif
#define v_nuke_nop() do {				\
	if(v_ip[-1] == NOP)				\
		v_ip--;					\
} while(0)

/*
 * v_schedule_delay: schedule the insn in the branch delay slot.
 * insn must not depend on being executed before or after the
 * branch.
 *	branch - a branch macro invocation (e.g., `v_bnei(r1,r2,label)).
 *	insn - an instruction macro
 *
 * Assumption: if macro2 is only one instruction, it cannot use
 * any registers that would screw with the branch delayed instruction
 * (macro1), in this case we can safely insert macro2 in the branch
 * delay slot.  A more sophisticated back end would use a more
 * aggressive strategy.
 *
 */
#define v_schedule_delay(branch, insn) do {             \
        v_code *_ip, _insn;                             \
                                                        \
        _ip = v_ip;                                     \
        insn;                                           \
        /* cannot safely fill the delay slot. */        \
        if((v_ip - _ip) > 1) {                          \
                branch;                                 \
        } else {                                        \
                /* avoid double eval of macro1 */       \
                _insn = *_ip;                           \
                v_ip = _ip;                             \
                branch;                                 \
                if(v_ip[-1] != NOP)                     \
                        v_fatal("v_schedule_delay: insn is not a branch.");\
                v_ip[-1] = _insn; /* overwrite nop */   \
        }                                               \
} while(0)

/*
 * v_raw_load: Perform a memory operation without hardware interlocks for n cycles.
 * If the load latency is greater than n cycles, the backend will insert
 * them.  MIPS only has a delay of 1 cycle.
 *      mem - macro
 *      n - number of cycles allowed before interlock is required.
 */
#define v_raw_load(mem, n) do {						\
	mem;								\
	if(n >= 1) {							\
		v_ip--;							\
		if(v_ip[0] != NOP)					\
			v_fatal("v_raw_load: insn is not a load");	\
	}								\
} while(0)

%%
;
; Core vcode instruction set.
;

(ret ((@ v_caller_int_rr, rs_or_imm) (v_jv v_epilogue_l) (v_nuke_nop)) (i l u ul p  v_mov: v_set:))
(ret ((@ v_fp_rr, rs) (v_jv v_epilogue_l) (v_nuke_nop)) (f d v_mov:))

; Standard arithmetic operations.
; (uses unsigned addition to prevent overflow exceptions)
(add (i u l ul p addu addiu) (f fadds) (d faddd))  	
(sub (i u l ul p subu ((addiu rd, rs1, -imm))) (f fsubs) (d fsubd))
(mul ((@ rs1,rs2) (mflo rd)) (s i l mult) (us u ul multu) )
(mul (f fmuls) (d fmuld))
(div ((@ rs1,rs2) (mflo rd)) (s i l div)  (us u ul divu) )
(div (f fdivs) (d fdivd))
(mod ((@ rs1,rs2) (mfhi rd)) (s i l div)  (us u ul divu) )

; Logical operations
(lsh (i l u ul sllv sll))
(rsh (i l srav sra) (u ul srlv srl))
(and (us s i l u ul and andi))
(or (us s i l u ul or ori))
(xor (us s i l u ul xor xori))

; Branches
(beq (i u l ul p beq beqi) (f feqs) (d feqd))
(bne (i u l ul p bne bnei) (f fnes) (d fned))
(blt (i l blt blti) (u ul p bltu bltui) (f flts) (d fltd))
(ble (i l ble blei) (u ul p bleu bleui) (f fles) (d fled))
(bgt (i l bgt bgti) (u ul p bgtu bgtui)  (f fgts) (d fgtd))
(bge (i l bge bgei) (u ul p bgeu bgeui) (f fges) (d fged))

; Unary operations
(set (c uc s us i l set) (u ul setu))
(com (i l u ul com))
(not (i l u ul not))
(mov (i l u ul p mov) (f fmovs) (d fmovd))
(neg (i l u ul neg)  (f fnegs) (d fnegd))

; Conversions 
(cvul2 (p mov))
(cvp2 (ul mov))	
(cvf2 (d cvtds) (i u ul l cvts2w))
(cvd2 (f cvtsd) (i u ul l cvtd2w))

; Memory operations

(def no_reg_plus_reg)
(ld  ((@ rd,rs1,rs_or_imm) (nop)) (c lb) (uc lbu) (s lh) (us lhu) (i u ul l p lw) (f ls) (d ld))
(st (c uc sb) (s us sh) (i u ul l p sw) (f ss) (d sd))


;
; Primary integer extensions
;

(nor (i u l ul nor))
(nand (i u l ul ((v_and: rd, rs1, rs2) (v_com: rd, rd))))
(nxor (i u l ul ((v_xor: rd, rs1, rs2) (v_com: rd, rd))))

; Conditional move extensions
(def synthetic_cmv)
(cmveq (i u ul l p))
(cmvne (i u ul l p))
(cmvlt (i u ul l p))
(cmvle (i u ul l p))
(cmvgt (i u ul l p))
(cmvge (i u ul l p))

; Unary extensions 
(abs (f fabs) (d fabd))
(nabs (f d ((v_abs: rd, rs) (v_neg: rd, rd))))
(ceil (f ceils) (d ceild))
(floor (f floors) (d floord))
(sqrt (f fsqrts) (d fsqrtd))


; Conditional expressions 
(lt  (i l slt slti) (u ul p sltu sltiu) )
(le  (i u p l ul ((v_lt: rd, rs2, rs1) (v_not: rd, rd))))
(ge  (i u p l ul ((v_lt: rd, rs1, rs2) (v_not: rd, rd))))
(gt  (i u p l ul ((v_lt: rd, rs2, rs1))))
(eq  (i u p l ul ((v_xor: rd, rs1, rs2) (v_ltui rd, rd, 1))))
(ne  (i u p l ul ((v_xor: rd, rs1, rs2) (v_ltu rd, v_zero, rd))))


; some useful macros
(def sext24 ((sll rd,rs,24) (sra rd,rd,24)))
(def sext16 ((sll rd,rs,16) (sra rd,rd,16)))
(def trunc16 ((andi rd,rd,0xffff)))
(def i2d ((mtc1 rd,rs) (cvtdw rd,rd)))

; Conversions
(cvc2 (i u l ul sext24))
(cvs2 (i u l ul sext16))
(cvus2 (i u l ul trunc16))
(cvi2 (c sext24) (s sext16) (u l ul mov) (f d v_cvl2:))
(cvu2 (c sext24) (s sext16) (i l ul mov) (f d v_cvl2:))
(cvl2 (c sext24) (s sext16) (i u ul mov) 
	(f ((movi2f rd, rs) (nop) (cvtsw rd, rd))) 
	(d ((movi2d rd, rs) (nop) (cvtdw rd, rd))))
(cvul2 (c sext24) (s sext16) (i u l mov) (f d v_cvl2:))

; Unaligned memory operations
(def uload ((lwl rd,rs1,imm) (lwr rd, rs1, imm)))
; uck a bit painful
(uld (s  ((uld rd,rs1,imm) (sll rd,rs,16) (sra rd,rd,16)))
	(us ((uld rd,rs1,imm) (andi rd,rd,0xffff)))
	(i l u ul p uld)
	(f d halt))
(ust 	(s  ((sb rd, rs1, imm) (sra at, rd, 8) (sb at, rs1, imm+1)))
	(us ((sb rd, rs1, imm) (srl at, rd, 8) (sb at, rs1, imm+1)))
	(i l u ul p ((swl rd, rs1, imm) (swr rd, rs1, imm)))
	(f d halt))
;
; Secondary extensions
;

(andnot (i u l ul ((v_com: v_at, rs2) (v_and: rd, rs1, v_at))))
(ornot (i u l ul ((v_com: v_at, rs2) (v_or: rd, rs1, v_at))))
(xornot (i u l ul ((v_com: v_at, rs2) (v_xor: rd, rs1, v_at))))

(mulhi ((@ rs1,rs2) (mfhi rd)) (s i l mult) (us u ul multu) )
(mulhilo ((@ rs1,rs2) (mflo rd1) (mfhi rd2)) (i l u ul mul) )
(divmod  ((@ rs1,rs2) (mflo rd1) (mfhi rd2)) (i l u ul div) )

(muladd ((v_mul: v_dat, rs1, rs2) (v_add: rd, v_dat, rs3)) (f) (d))
(mulsub ((v_mul: v_dat, rs1, rs2) (v_sub: rd, v_dat, rs3)) (f) (d))
(negmuladd ((v_mul: v_dat, rs1, rs2) (v_add: rd, v_dat, rs3) (v_neg: rd,rd)) (f) (d))
(negmulsub ((v_mul: v_dat, rs1, rs2) (v_sub: rd, v_dat, rs3) (v_neg: rd,rd)) (f) (d))

#include "demand.h"
#include "binary.h"

extern v_label_type v_epilogue_l;

/* need to add jalv */
#define v_jalpi(dst, target)   do { 	\
	if(_vrr(dst) == _ra)		\
		jal((unsigned)target);	\
	else {				\
		v_setp(v_at, target);	\
		v_jalp(dst, v_at);	\
	}				\
} while(0)

#define v_jalp(dst, target) do { 		\
	jalr(_vrr(dst), _vrr(target)); 		\
} while(0)

#define v_jpi(dst)      do { jump((unsigned)(dst)); } while(0)
#define v_jp(dst)       do { jr(_vrr(dst)); } while(0)
#define v_jv(l)		do { v_beqi(v_zero,v_zero,l); } while(0)

#define v_retv()     do { v_jv(v_epilogue_l); } while(0)
#define v_retfi(imm) do { v_setf(v_fp_rr, imm); v_retv(); } while(0)
#define v_retdi(imm) do { v_setd(v_fp_rr, imm); v_retv(); } while(0)

#define v_setp(rd, imm) v_setul(rd, (unsigned long)imm)
#define v_setf(rd, imm) v_float_imm(rd, imm)
#define v_setd(rd, imm) v_double_imm(rd, imm)

#define v_nop() nop()
#define v_nuke_nop() do { if(v_ip[-1].l == 0x01) v_ip--; } while(0)
%%
;missing:
;negmuladd, negmulsub, mulsub, muladd, mulhi, mullo, divmod
;xornot, ornot, andnot
;ceil,floor
;cmv*
;ust,uld

(ld (c lb_rr lb)  (uc lbu_rr lbu) (s lh_rr lh) (us lhu_rr lhu) (i u ul l p lw_rr lw) (f l_s_rr l_s) (d l_d_rr l_d))

(st (c uc sb_rr sb) (s us sh_rr sh) (i u ul l p sw_rr sw) (f s_s_rr s_s) (d s_d_rr s_d))

(lsh (i l u ul sllv sll))
(rsh (i l srav sra) (u ul srlv srl))
(and (us s i l u ul and andi))
(or (us s i l u ul or ori))
(xor (us s i l u ul xor xori))
(not (i l u ul ((sltiu rd, rs, 1))))
(com (i l u ul ((nor rd, _zero, rs))))

(add (i u l ul p addu addiu) (f fadd_s) (d fadd_d))
(sub (i u l ul p subu ((addiu rd, rs1, -imm))) (f fsub_s) (d fsub_d))
(mul ((@ rs1,rs2) (mflo rd)) (s i l mult) (us u ul multu) )
(mul (f fmul_s) (d fmul_d))
(div ((@ rs1,rs2) (mflo rd)) (s i l vdiv)  (us u ul vdivu))
(div (f fdiv_s) (d fdiv_d))
(mod ((@ rs1,rs2) (mfhi rd)) (s i l div)  (us u ul divu) )
(neg (i l u ul ((v_sub: rd, _zero, rs)))  (f fneg_s) (d fneg_d))

(mov (i l u ul p mov) (f fmov_s) (d fmov_d))
(set (c uc us s i l set) (u ul setu))

(lt  (i l slt slti) (u ul p sltu sltiu))
(le  (i u p l ul ((v_lt: rd, rs2, rs1) (v_not: rd, rd))))
(ge  (i u p l ul ((v_lt: rd, rs1, rs2) (v_not: rd, rd))))
(gt  (i u p l ul ((v_lt: rd, rs2, rs1))))
(eq  (i u p l ul ((v_xor: rd, rs1, rs2) (v_ltui rd, rd, 1))))
(ne  (i u p l ul ((v_xor: rd, rs1, rs2) (v_ltu rd, v_zero, rd))))

(beq (i u l ul p beq beqi)
	(f ((c_eq_s rs1, rs2) (bc1t label)))
	(d ((c_eq_d rs1, rs2) (bc1t label))))
(bne (i u l ul p bne bnei)
	(f ((c_eq_s rs1, rs2) (bc1f label)))
	(d ((c_eq_d rs1, rs2) (bc1f label))))
(blt (i l blt blti) (u ul p bltu bltui)
	(f ((c_lt_s rs1, rs2) (bc1t label)))
	(d ((c_lt_d rs1, rs2) (bc1t label))))
(ble (i l ble blei) (u ul p bleu bleui)
	(f ((c_le_s rs1, rs2) (bc1t label)))
	(d ((c_le_d rs1, rs2) (bc1t label))))
(bgt (i l bgt bgti) (u ul p bgtu bgtui)
	(f ((c_le_s rs1, rs2) (bc1f label)))
	(d ((c_le_d rs1, rs2) (bc1f label))))
(bge (i l bge bgei) (u ul p bgeu bgeui)
	(f ((c_lt_s rs1, rs2) (bc1f label)))
	(d ((c_lt_d rs1, rs2) (bc1f label))))

; Conversions
(def sext24 ((sll rd,rs,24) (sra rd,rd,24)))
(def sext16 ((sll rd,rs,16) (sra rd,rd,16)))
(def trunc16 ((andi rd,rd,0xffff)))

; Conversions
(cvc2 (i u l ul sext24))
(cvs2 (i u l ul sext16))
(cvus2 (i u l ul trunc16))
(cvi2 (c sext24) (s sext16) (u l ul mov) (f d v_cvl2:))
(cvu2 (c sext24) (s sext16) (i l ul mov) (f d v_cvl2:))
(cvl2 (c sext24) (s sext16) (i u ul mov) 
	(f ((mtc1 rd, rs) (cvt_s_w rd, rd)))
	(d ((mtc1 rd, rs) (cvt_d_w rd, rd))))
(cvul2 (c sext24) (s sext16) (i u l mov) (f d v_cvl2:) (p mov))
(cvp2 (ul mov))	
(cvf2 (d cvt_d_s) (i u ul l ((cvt_w_s rs,rs) (mfc1 rd, rs))))
(cvd2 (f cvt_s_d) (i u ul l ((cvt_w_d rs,rs) (mfc1 rd, rs))))

(nor (i u l ul ((v_or: rd, rs1, rs2) (v_com: rd, rd))))
(nxor (i u l ul ((v_xor: rd, rs1, rs2) (v_com: rd, rd))))
(nand (i u l ul ((v_and: rd, rs1, rs2) (v_com: rd, rd))))

(abs (f fabs_s) (d fabs_d))
(nabs (f d ((v_abs: rd, rs) (v_neg: rd, rd))))
(sqrt (f fsqrt_s) (d fsqrt_d))

(ret ((@ v_caller_int_rr, rs_or_imm) (v_jv v_epilogue_l) (v_nuke_nop)) (i l u ul p  v_mov: v_set:))
(ret ((@ v_fp_rr, rs) (v_jv v_epilogue_l) (v_nuke_nop)) (f d v_mov:))

/* $Id: reg-manager.h,v 1.2 1998/05/17 03:41:33 maxp Exp $ */

#ifndef __REG_MANAGER_H__
#define __REG_MANAGER_H__

extern bvdt i_ireg_alltmp[];
extern bvdt i_freg_alltmp[];
extern bvt i_itmp, i_ivar, i_ireg, i_iregx, i_imsk, i_iwrk;
extern bvt i_ftmp, i_fvar, i_freg, i_fregx, i_fmsk, i_fwrk;

extern void i_reg_init (void);

static inline void i_reg_reset (int fp) {
     if (fp) bv_cpto(i_freg, i_fregx);
     else bv_cpto(i_ireg, i_iregx);
}

static inline int i_reg_get (int fp) {
     bvt tmp, var, reg, wrk;
     int rn, class;

     if (fp) { 
	  tmp = i_ftmp; var = i_fvar; reg = i_freg; wrk = i_fwrk; class = V_F;
     } else { 
	  tmp = i_itmp; var = i_ivar; reg = i_ireg; wrk = i_iwrk; class = V_I; 
     }

     bv_inter2(wrk, reg, i_leafp ? tmp : var);
     rn = bv_firstbit(wrk);
     if (rn == -1)
	  rn = bv_firstbit(reg);
     if (rn == -1) 
	  return -1;
     bv_unset(reg, rn);
     v_pickreg(rn, class);
     return rn;
}

static inline void i_reg_put (int fp, int r) {
     bv_set(fp ? i_freg : i_ireg, r);
}

static inline void i_reg_use (int fp, int r) {
     bv_unset(fp ? i_freg : i_ireg, r);
}

static inline int i_reg_navail (int fp) {
     return bv_num(fp ? i_freg : i_ireg);
}

/* Mask functions */

static inline void i_reg_setmask (int fp, int r) {
     if (fp) {
	  bv_unset(i_freg, r);  bv_set(i_fmsk, r);
     } else {
	  bv_unset(i_ireg, r);  bv_set(i_imsk, r);
     }
}

static inline void i_reg_resetmask (int fp) {
     bv_clear(fp ? i_fmsk : i_imsk);
}

static inline void i_reg_mask (int fp) {
     if (fp) bv_diff2(i_freg, i_fregx, i_fmsk);
     else bv_diff2(i_ireg, i_iregx, i_imsk);
}

#endif /* __REG_MANAGER_H__ */

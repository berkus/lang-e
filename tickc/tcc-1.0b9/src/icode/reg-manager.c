/* $Id: reg-manager.c,v 1.1 1998/05/06 19:03:38 maxp Exp $ */

#include "icode.h"
#include "icode-internal.h"

#include "icode-arch.c"

#define nelems(x) ((sizeof x) / (sizeof *x))

bvt i_itmp, i_ivar, i_ireg, i_iregx, i_imsk, i_iwrk;
bvt i_ftmp, i_fvar, i_freg, i_fregx, i_fmsk, i_fwrk;

void i_reg_init (void) {
     static init = 0;
     assert(nelems(i_ireg_tmp) == nelems(i_ireg_var));
     assert(nelems(i_freg_tmp) == nelems(i_freg_var));
	    
     if (init) return;

     i_itmp = bv_pinit(i_ireg_tmp, nelems(i_ireg_tmp));
     i_ivar = bv_pinit(i_ireg_var, nelems(i_ireg_var));
     i_ireg = bv_prunion(i_itmp, i_ivar);
     i_iregx = bv_pcp(i_ireg);
     i_imsk = bv_pnew(nelems(i_ireg_tmp) * 8 * sizeof(bvdt));
     i_iwrk = bv_pnew(nelems(i_ireg_tmp) * 8 * sizeof(bvdt));

     i_ftmp = bv_pinit(i_freg_tmp, nelems(i_freg_tmp));
     i_fvar = bv_pinit(i_freg_var, nelems(i_freg_var));
     i_freg = bv_prunion(i_ftmp, i_fvar);
     i_fregx = bv_pcp(i_freg);
     i_fmsk = bv_pnew(nelems(i_freg_tmp) * 8 * sizeof(bvdt));
     i_fwrk = bv_pnew(nelems(i_freg_tmp) * 8 * sizeof(bvdt));

     init = 1;
}

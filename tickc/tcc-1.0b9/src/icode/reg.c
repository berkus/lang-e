/* $Id: reg.c,v 1.11 1998/06/24 18:09:10 maxp Exp $ */

#include <stdlib.h>

#include "icode.h"
#include "icode-internal.h"

/*
 * Simple register allocation
 */

static int psort (const void *a, const void *b) {
     i_local_t la = *(i_local_t *)a, lb = *(i_local_t *)b;
     return (REFCNT(lb) - REFCNT(la));
}

static inline void spill (i_local_t loc) {
     if (TYPE(loc) == I_B) ADDR(loc) = v_localb(ADDR(loc));
     else ADDR(loc) = v_local(TYPE(loc));
}

static int noregs;

/* i_ra_priorityinternal: find storage for local i of type ty */
static void ra_priorityinternal (i_local_t i, int ty) {
     int reg;

     if (PARAMP(i))		/* Skip params */
	  return;
     if (SCLASS(i) == STACK) {
	  spill(i);		/* Put on stack things that must be there */
	  return;
     }
     assert(SCLASS(i) == UNDECIDED);
     if (!noregs) {		/* If reg is avail, put this local in a reg */
	  reg = i_reg_get(ty);
	  if (reg != -1) {
	       ADDR(i) = reg;
	       SCLASS(i) = REGISTER;
	       return;
	  }
	  noregs = 1;
     }
     spill(i);			/* Out of registers: spill */
     SCLASS(i) = STACK;
}

/* i_ra_priority: map names to registers (1st come - 1st serve) until
   out of registers, then to stack locations. */
static void i_ra_priority (void) {
#define min_regnum 8
     unsigned int i;
     i_local_t *order;

     if (num_i) {
	  noregs = 0;
	  NEW(order, num_i);
	  for (i = 0; i < num_i; i++) order[i] = i;
	  qsort((void *)order, num_i, sizeof(i_local_t), psort);
	  if (!i_leafp)	{	/* Don't use temporaries if we have enough */
	       bv_diff(i_ireg, i_itmp);
	       if (bv_num(i_ireg) < min_regnum) bv_union(i_ireg, i_itmp);
	  }
	  for (i = 0; i < num_i; i++)
	       ra_priorityinternal(order[i], INT);
     }
     if (num_f) {
	  noregs = 0;
	  NEW(order, num_f);
	  for (i = 0; i < num_f; i++) order[i] = i+i_loc_ilim;
	  qsort((void *)order, num_f, sizeof(i_local_t), psort);
	  if (!i_leafp)	{	/* Don't use temporaries if we have enough */
	       bv_diff(i_freg, i_ftmp);
	       if (bv_num(i_freg) < min_regnum) bv_union(i_freg, i_ftmp);
	  }
	  for (i = 0; i < num_f; i++)
	       ra_priorityinternal(order[i], FP);
     }
}

/*
 * Driver for register allocation
 */

int i_ty2sc[] = {
     INT, /* I_C */ INT, /* I_UC */ INT, /* I_S */  INT, /* I_US */
     INT, /* I_I */ INT, /* I_U */  INT, /* I_L */  INT, /* I_UL */
     INT, /* I_P */ FP, /* I_F */   FP, /* I_D */   INT, /* I_V */
     INT, /* I_B */
};

/* i_regalloc: perform register allocation */
void i_regalloc (i_pmap_t params[max_param]) {
     unsigned int i;
				/* Registers used for params are not available
				   for register allocation */
     if (i_ralloctype == RA_GC) {
	  for (i=0; i < i_params_cur; i++)
	       if (SCLASS(i_params[i].i) == REGISTER)
		    i_reg_setmask(i_ty2sc[i_params[i].t], ADDR(i_params[i].i));
     } else {
	  for (i=0; i < i_params_cur; i++)
	       if (SCLASS(i_params[i].i) == REGISTER)
		    i_reg_use(i_ty2sc[i_params[i].t], ADDR(i_params[i].i));
     }

#ifndef NDEBUG
     for (i=0; i < i_params_cur; i++)
	  if (SCLASS(i_params[i].i) == REGISTER) {
	       v_reg_type r = ADDR(i_params[i].i);
	       if ((i_ty2sc[i_params[i].t] == INT && (r==t0 || r==t1 || r==t2))
		   || (i_ty2sc[i_params[i].t] == FP && (r==ft0 || r==ft1)))
		    i_fatal("internal error: parameter conflicts "
			    "with icode temporary");
	  }
#endif

     switch (i_ralloctype) {
     case RA_EZ:
	  if (i_quit == END_LV || i_quit == END_RA1)
	       return;
	  i_ra_priority();
	  break;
     case RA_GC:
	  i_fg_lv();
	  if (i_quit == END_LV)
	       return;
	  i_ig_build();		/* Build interference graph and color it */
	  if (i_quit == END_RA1)
	       return;
	  if (num_i) i_ig_color(INT);
	  if (num_f) i_ig_color(FP);
	  break;
     case RA_LR:
	  i_fg_lv();
	  if (i_quit == END_LV)
	       return;
	  i_li_full();
	  if (i_quit == END_RA1)
	       return;
	  i_ra_ls();
	  break;
     case RA_LS:
	  if (i_quit == END_LV)
	       return;
	  i_li_scc();
	  if (i_quit == END_RA1)
	       return;
	  i_ra_ls();
	  break;
     default: 
	  assert(0);
     }

     if (i_debug) {
	  for (i = 0; i < i_loc_icur; i++) {
	       char c = 0;
	       switch (SCLASS(i)) {
	       case UNDECIDED: c = 'U'; break; case REGISTER: c = 'R'; break;
	       case STACK: c = 'S'; break;     case DATA: c = 'D'; break;
	       default: assert(0);
	       }
	       printf("int %2d: %c%3d [ref=%4.4f]\n", 
		      i, c, ADDR(i), REFCNT(i));
	  }
	  for (i = i_loc_ilim; i < i_loc_fcur; i++) {
	       char c = 0;
	       switch (SCLASS(i)) {
	       case UNDECIDED: c = 'U'; break; case REGISTER: c = 'R'; break;
	       case STACK: c = 'S'; break;     case DATA: c = 'D'; break;
	       default: assert(0);
	       }
	       printf("fp %2d: %c%3d [ref=%4.4f]\n", i, c, ADDR(i), REFCNT(i));
	  }
     }
}

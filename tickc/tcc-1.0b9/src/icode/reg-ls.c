/* $Id: reg-ls.c,v 1.9 1998/06/24 18:09:09 maxp Exp $ */

#include "icode.h"
#include "icode-internal.h"
#include "ll.h"

/*
 * Live intervals
 */

i_lr_t i_ilrs;		/* Int and float register live intervals */
i_lr_t i_flrs;
i_cnt_t i_ilr_cur;		/* Live interval counters */
i_cnt_t i_flr_cur;

/* i_li_unparse: unparse the live interval information. */
void i_li_unparse (void) {
     i_cnt_t i;

     assert((i_ilrs || !num_i) && (i_flrs || !num_f));

     if (num_i)
	  printf("Ints:\n");
     for (i = i_ilr_cur+1; i < num_i; i++)
	  printf("\t%4d :: %d - %d\n", 
		 i_ilrs[i].n, i_ilrs[i].beg, i_ilrs[i].end);
     if (num_f)
	  printf("Floats:\n");
     for (i = i_flr_cur+1; i < num_f; i++)
	  printf("\t%4d :: %d - %d\n", 
		 i_flrs[i].n, i_flrs[i].beg, i_flrs[i].end);
}

/*
 * Register allocation using live intervals, scanning endpoints of ranges
 */

/* lrexpire: l is the list of live intervals, e is the list element being 
   processed, v1 is the end point of the current live interval,
   (unsigned int)sc is the storage class (int/float).
   v2 is the register pool.  Applied by i_llsbwd.
   Removes e from l if the start point of e's live interval follows v1, and
   places the register used for e's live interval back into register pool. */
static inline unsigned int lrexpire (i_ll_t l, i_lle_t e, 
				     void *v1, void *sc) {
     i_cnt_t pos = (i_cnt_t)v1;
     i_local_t loc;
     if (e->lr->beg <= pos)
	  return 1;		/* Stop traversing now */

     i_llrem(l, e);		/* Remove expired live interval */
     loc = e->lr->n;
				/* Put register back into free pool */
     assert(SCLASS(loc) == REGISTER);
     i_reg_put((unsigned int)sc, ADDR(loc));
     return 0;			/* Not done yet */
}

/* lrstartsfirst: le is list element being processed, v1 is end point of
   current live interval.  Applied by i_llsfwd.  Leaves in i_lle the element
   in the live interval list overlapping v and having the earliest start
   point (i.e. the first element in the list). */
static i_lle_t i_lle;
static inline unsigned int lrstartsfirst (i_ll_t l, i_lle_t le, 
					  void *v1, void *v2) {
     i_lle = le;
     return 1;
}

/* lrleastcost: le is list element being processed.  Applied by llsfwd.
   Leaves in lle the element in the live range list overlapping point end
   and having the lowest ref count. */
static i_cnt_t refc;
static inline unsigned int lrleastcost (i_ll_t l, i_lle_t le, 
					void *v1, void *v2) {
     i_lr_t r = le->lr;
     if (REFCNT(r->n) < refc) {
	  i_lle = le;
	  refc = REFCNT(r->n);
     }
     return 0;
}

/* lrstartsbefore: le is list element being processed, v1 is start point of
   current live interval.  Returns whether start of le's live interval
   precedes v. */
static inline unsigned int lrstartsbefore (i_ll_t l, i_lle_t le, 
					   void *v1, void *v2) {
     i_cnt_t pos = (i_cnt_t)v1;
     assert(le && le->lr && le->lr->beg > 0);
     return (le->lr->beg < pos);
}

/* lrspill: put loc on the stack */
static inline void lrspill (i_local_t loc) {
     SCLASS(loc) = STACK;
     if (TYPE(loc) == I_B)
	  ADDR(loc) = v_localb(ADDR(loc));
     else ADDR(loc) = v_local(TYPE(loc));
}

/* ra_ls_internal: allocate registers by scanning endpoints of live 
   ranges. */
static void ra_ls_internal (int fp, i_lr_t lrbeg, i_lr_t lrlim) {
     i_lr_t lr;
     i_lle_t le;
     i_ll_t active;
     i_local_t loc;
     i_cnt_t r;
     int use_refc = (i_ralloctype == RA_LS);

     assert(lrbeg && lrlim);
     active = i_llcreate(i_reg_navail(fp));

#ifndef NDEBUG
     for (lr = lrlim-1; lr >= lrbeg; lr--)
	  assert((lr+1)->end >= lr->end);
#endif

     for (lr = lrlim; lr  >= lrbeg; lr--) {
	  loc = lr->n;		/* Skip params and stack objects */
	  if (PARAMP(loc)) continue;
	  if (SCLASS(loc) == STACK) { lrspill(loc); continue; }
				/* Expire live intervals that start after
				   end of current live interval. */
	  i_llsbwd(active, lrexpire, (void *)lr->end, (void *)fp);
				/* If all registers are allocated to a live
				   range, spill the one that begins first. */
	  if (i_llisfull(active)) {
	       i_lle = NULL;
	       if (use_refc) {
		    refc = max_cnt;
		    i_llsfwd(active, lrleastcost, 0, 0);
				/*  Spill i_lle if it weighs less than current
				    lr or weighs the same but starts first */
		    if (i_lle 
			&& (refc < REFCNT(lr->n)
			    || (refc == REFCNT(lr->n)
				&& i_lle->lr->beg < lr->beg))) {
			 i_local_t spill = i_lle->lr->n;
			 assert(SCLASS(spill) == REGISTER);
			 r = ADDR(spill);
			 lrspill(spill);
			 i_llrem(active, i_lle);
		    } else
			 r = -1; /* No good candidate; spill current lr */
	       } else {
		    i_llsfwd(active, lrstartsfirst, (void *)lr->end, NULL);
		    if (i_lle->lr->beg < lr->beg) {
			 i_local_t spill = i_lle->lr->n;
			 assert(SCLASS(spill) == REGISTER);
			 r = ADDR(spill);
			 lrspill(spill);
			 i_llrem(active, i_lle);
		    } else
			 r = -1;/* If no lr begins before current, spill it */
	       }
	  } else {
	       r = i_reg_get(fp);
	       assert(r != -1);
	  }
				/* Insert the current live interval, in order
				   sorted by increasing start point */
	  if (r != -1) {
	       ADDR(loc) = r; SCLASS(loc) = REGISTER;
				/* Search back until start point of a live 
				   range precedes current lr's start point */
	       le = i_llsbwd(active, lrstartsbefore, (void *)lr->beg, NULL);
	       if (le == NULL)	/* If no such live interval is found, insert lr
				   at head of list */
		    le = i_llibwd(active, le);
	       else		/* Else after the found live interval */
		    le = i_llifwd(active, le);
	       i_llset(le, lr);	/* Set the LR info of the list element */
	  } else {
	       lrspill(loc);
	  }
     }
}

/* i_ra_ls: reg alloc floats and ints with linear scan */
void i_ra_ls (void) {
     if (num_i)
	  ra_ls_internal(INT, &i_ilrs[i_ilr_cur+1], &i_ilrs[num_i-1]);
     if (num_f)
	  ra_ls_internal(FP, &i_flrs[i_flr_cur+1], &i_flrs[num_f-1]);
}

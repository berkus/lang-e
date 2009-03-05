/* $Id: li-full.c,v 1.5 1998/05/17 20:38:35 maxp Exp $ */

/* Full live interval analysis */

#include <assert.h>
#include "icode.h"
#include "icode-internal.h"
#include "mem.h"

static i_cnt_t hpos, tpos;

/* lruseinternal: define live range for variable l, if necessary */
static inline void lruseinternal (i_local_t l, i_cnt_t bpos, i_cnt_t epos, 
				  int sc) {
     i_cnt_t tlr;
     i_lr_t lrs = sc == FP ? i_flrs : i_ilrs;

     if (LRANGE(l) == NOLRINFO) {
	  tlr = sc == FP ? i_flr_cur-- : i_ilr_cur--;
	  LRANGE(l) = tlr;
	  lrs[tlr].n = l; lrs[tlr].end = epos;
     } else {
	  tlr = LRANGE(l);
	  assert(lrs[tlr].end >= epos);
     }
     assert(lrs[tlr].beg >= bpos || lrs[tlr].beg == 0);
     lrs[tlr].beg = bpos;
}

/* lriuseiter: like lruseinternal, but for use in iterators */
static inline void lriuseiter (i_local_t l, bvt vec, void *v) {
     i_cnt_t pos = (i_cnt_t)v;
     lruseinternal(l, pos, pos, INT);
}

/* lrfuseiter: like lruseinternal, but for use in iterators */
static inline void lrfuseiter (i_local_t l, bvt vec, void *v) {
     i_cnt_t pos = (i_cnt_t)v;
     lruseinternal(fp_mk(l), pos, pos, FP);
}

/* lriskip: extend the live range of int variable l to span the
   whole body of the current basic block */
static inline void lriskip (i_local_t l, bvt vec, void *v) {
     lruseinternal(l, hpos, tpos, INT);
}

/* lrfskip: extend the live range of float variable l to span the
   whole body of the current basic block */
static inline void lrfskip (i_local_t l, bvt vec, void *v) {
     lruseinternal(fp_mk(l), hpos, tpos, FP);
}

/* lrdef: handle a new definition in the flow graph.
   lrs is the list of live ranges (int or float).
   Extend the live range of l to include pos. */
static inline void lrdef (i_local_t l, i_lr_t lrs,
			  bvt skip, bvt live, i_cnt_t pos) {
     int tlr = LRANGE(l);
     assert(tlr != NOLRINFO && lrs[tlr].n == l && lrs[tlr].beg >= pos);
     lrs[tlr].beg = pos;
}

/* Macros for handling variable definitons: if l is not live after pos, 
   delete the definition and ignore its uses. */
#define IDEF(l) do {						\
     if (!bv_setp(iskip, l)) {					\
	  if (!bv_unsetf(ilv, l)) {				\
	       mk_nop(cp);					\
	       break;						\
	  }							\
	  lrdef(l, i_ilrs, iskip, ilv, pos);			\
     }								\
} while (0)
#define FDEF(l) do {						\
     i_local_t j = fp_id(l);                                    \
     if (!bv_setp(fskip, j)) {					\
	  if (!bv_unsetf(flv, j)) {				\
	       mk_nop(cp);					\
	       break;						\
	  }							\
	  lrdef(l, i_flrs, fskip, flv, pos);			\
     }								\
} while (0)
#define DEF(l) do { if (sc_var(l)) { 		\
     if (sc_fp(l)) FDEF(l); else IDEF(l); 	\
} } while (0)

/* Like IDEF and FDEF, but do not remove apparently dead code: 
   should be used in situations when def/use info is not enough
   to tell whether an insn is dead or not (i.e. calls). */
#define IDEFKP(l) do {						\
     if (!bv_setp(iskip, l)) {					\
	  if (bv_unsetf(ilv, l))				\
	       lrdef(l, i_ilrs, iskip, ilv, pos);	        \
     }								\
} while (0)
#define FDEFKP(l) do {						\
     i_local_t j = fp_id(l);                                    \
     if (!bv_setp(fskip, j)) {					\
	  if (bv_unsetf(flv, j))			       	\
	       lrdef(l, i_flrs, fskip, flv, pos);		\
     }								\
} while (0)
#define DEFKP(l) do { if (sc_var(l)) {		\
     if (sc_fp(l)) FDEFKP(l); else IDEFKP(l); 	\
} } while (0)

/* Handle variable uses */
#define USE(l) do {							\
     if (sc_var(l)) {							\
     if (sc_fp(l)) {							\
          i_local_t j = fp_id(l);                                       \
	  if (!bv_setp(fskip, j) && bv_setf(flv, j))			\
	       lruseinternal(l, pos, pos, FP);           		\
     } else {								\
	  if (!bv_setp(iskip, l) && bv_setf(ilv, l))		\
	       lruseinternal(l, pos, pos, INT);         		\
     }									\
     }									\
} while (0)

void i_li_full (void) {
     bvt iskip=0, fskip=0;	/* Variables live at both start & end of bb */
     bvt ilv=0, flv=0;		/* Live variables at each step */
     i_bb_t b;			/* Temp bblock */
     i_puint32 cp;		/* Code pointer */
     i_cnt_t pos = max_cnt;	/* Absolute layout-order position */
     unsigned int op;		/* Current opcode */

				/* Allocate live ranges */
     if (num_i) NEW0(i_ilrs, num_i);
     if (num_f) NEW0(i_flrs, num_f);
     i_ilr_cur = num_i-1; i_flr_cur = num_f-1;
				/* Now calculate the live ranges */
     for (b = i_fg_tail; b; pos--, b = b->lprev) {
	  tpos = pos;
	  hpos = pos - (b->t - b->h)/i_isize;
				/* Compute skip set */
	  if (num_i) {
	       iskip = bv_rinter(b->ilv_out, b->ilv_in);
	       ilv = bv_rdiff(b->ilv_out, iskip);
	       bv_eachbit(iskip, lriskip, b);
	  }
	  if (num_f) {
	       fskip = bv_rinter(b->flv_out, b->flv_in);
	       flv = bv_rdiff(b->flv_out, fskip);
	       bv_eachbit(fskip, lrfskip, b);
	  }

	  assert(b && b->t >= b->h);
	  for (cp = b->t; cp >= b->h; pos--, cp -= i_isize) {
	       op = get_op(cp);
	       switch(i_op2class[op]) {
	       case I_MOPW:	case I_MOPWF:
		    USE(get_rs2(cp));
				/* Fall through */
	       case I_MOPWI:	case I_MOPWIF:
		    USE(get_rd(cp));
		    USE(get_rs(cp));
		    break;
	       case I_MOPR:	case I_BOP:
	       case I_MOPRF:	case I_BOPF:
	       case I_MOPRI:	case I_BOPI:	case I_MOPRIF:
		    DEF(get_rd(cp));
				/* Fall through */
	       case I_BR:	case I_BRI:	case I_BRF:
		    USE(get_rs(cp));
		    if (!isimmed(op)) USE(get_rs2(cp));
		    break;
	       case I_UOP:	case I_UOPI:	case I_UOPF:
		    DEF(get_rd(cp));
		    if (!isimmed(op)) USE(get_rs(cp));
		    break;
	       case I_SET:	case I_SETF:
		    DEF(get_rd(cp));
		    break;
	       case I_LEA:	case I_LEAF:
		    DEF(get_rd(cp));
		    USE(get_rs(cp));
		    break;
	       case I_ARG:	case I_ARGF:
	       case I_RETF:	case I_JMP:
		    USE(get_rd(cp));
		    break;
	       case I_RET:
		    if (op != i_op_retv) USE(get_rd(cp));
		    break;
	       case I_CALL:	case I_CALLF:
	       case I_CALLI:	case I_CALLIF:
		    if (op != i_op_callv && op != i_op_callvi) 
			 DEFKP(get_rd(cp));
				/* Add use of callee if not immediate; it is in
				   an int register, so treat it like an int */
		    if (!isimmed(op)) USE(get_rs(cp));
				/* Set info for caller-saved registers */
		    if (num_i) icallsav(cp, bv_runion(ilv, iskip));
		    if (num_f) fcallsav(cp, bv_runion(flv, fskip));
		    break;
	       case I_MISC:	case I_JMPI:	case I_RETI:
		    continue;
	       default: assert(0);
	       }
	  }
	  assert(pos+1 == hpos);
	  if (num_i) bv_eachbit(ilv, lriuseiter, (void *)hpos);
	  if (num_f) bv_eachbit(flv, lrfuseiter, (void *)hpos);
     }
     DEBUG(i_li_unparse());
}

/* $Id: reg-gc.c,v 1.6 1998/06/24 18:09:07 maxp Exp $ */

#include <stdlib.h>
#include "icode.h"
#include "icode-internal.h"

				/* Graph coloring interference graph */
typedef struct {
     i_local_t n;		/* Size of graph (no. of matrix rows/cols) */
     unsigned int w;		/* Word width (no.cols/(bits/word)) */
     i_puint32 d;		/* Data */
} * ig_t;

static ig_t iig;		/* Integer interference graph */
static ig_t fig;		/* Floating point interference graph */

				/* Row macro: i=index,m=matrix */
#define row(i, m) ((bvt)((m)->d+(m)->w*(i)))

static int *removed;		/* Flag whether node is removed from graph */

/*
 * Basic ops on interference graph (IG) adjacency matrix
 */

/* i_igcreate: create a new interference graph for k variables */
/* Note: for performance purposes, this function violates the bvt
   Abstraction.  It may need to be changed if bvt changes. */
static ig_t i_igcreate (i_local_t k) {
     ig_t g;
     unsigned int i, w = bv_nwords(k)+bv_dx;
     NEW(g, 1);
     g->n = k;
     g->w = w;
     NEW0(g->d, k*w);
     for (i = 0; i < k; i++)	/* Initialize bit vector metadata */
	  g->d[g->w*i] = k;
     return g;
}

/* i_igcopy: create a copy of interference graph g */
static ig_t i_igcopy (ig_t g) {
     ig_t g2;
     i_puint32 l, p, q;
     unsigned int s;
     
     NEW(g2, 1);		/* Allocate new graph */
     g2->n = g->n;
     g2->w = g->w;
     s = g->n*g->w;		/* Size of data of old graph */
     NEW(g2->d, s);		/* Allocate data of new graph */
     p = g2->d;
     q = g->d;			/* Data of old graph */
     l = q+s;			/* Limit of data old graph */
     assert(s);
     switch(s&0xF) {		/* s%16 */
	  do {			/* Copy */
	  case 0:   *p++ = *q++; case 15:  *p++ = *q++;
	  case 14:  *p++ = *q++; case 13:  *p++ = *q++;
	  case 12:  *p++ = *q++; case 11:  *p++ = *q++;
	  case 10:  *p++ = *q++; case  9:  *p++ = *q++;
	  case 8:   *p++ = *q++; case  7:  *p++ = *q++;
	  case 6:   *p++ = *q++; case  5:  *p++ = *q++;
	  case 4:   *p++ = *q++; case  3:  *p++ = *q++;
	  case 2:   *p++ = *q++; case  1:  *p++ = *q++;
	  } while (q < l);
	  break;
     default:
	  assert(0);
     }

     return g2;
}

#ifndef NDEBUG
/* i_igunparse: unparse the interference graph g of size k */
static void i_igunparse (ig_t g) {
     unsigned int i;
     i_local_t n = g->n;

     printf("%dx%d matrix:\n", n, n);
     for (i = 0; i < n; i++) {	/* Unparse each row */
	  printf("   %2d: ", i);
	  bv_unparse(row(i, g));
     }
}
#endif

/*
 * Adding nodes to interference graph
 */

/* i_igcliqueinternal: set row n of matrix for IG x to union of row n and v */
static inline void i_igcliqueinternal (unsigned int n, bvt v, void *x) {
     ig_t g = (ig_t)x;
     bv_union(row(n,g), v);
}

/* i_igclique: given a bit vector v of simultaneously live variables, create
   the corresponding clique in interference graph g */
static inline void i_igclique (ig_t g, bvt v) {
				/* For each bit i in v, set row i of IG matrix
				   to v. (This is a call to inline fn, passing
				   another inline fn as argument: gcc -O3
				   recursively inlines both.) */
     bv_eachbit(v, i_igcliqueinternal, (void *)g);	
}

/*
 * Removing nodes from interference graph
 */

/* i_igrmnode: remove bit rmn from row n of adjacency matrix x
   (i.e. remove edge n->rmn in IG) */
static i_local_t rmn;
static inline void i_igrmnode (unsigned int n, bvt v, void *x) {
     ig_t g = (ig_t)x;
     assert(rmn < g->n); bv_unset(row(n, g), rmn);
}

/* i_igremove: remove node i from IG g */
static inline void i_igremove (i_local_t i, ig_t g) {
     rmn = i;			/* Remove i from each of its neighbors */
     bv_eachbit(row(i, g), i_igrmnode, (void *)g);
     bv_clear(row(i, g));	/* Clear row i */
     removed[i] = 1;
}

/* i_igspill: spill node i, removing it from index j in interference graph g */
static void i_igspill (i_local_t i, i_local_t j, ig_t g) {
     SCLASS(i) = STACK;		/* Update storage location */
     if (TYPE(i) == I_B)
	  ADDR(i) = v_localb(ADDR(i));
     else ADDR(i) = v_local(TYPE(i));
     i_igremove(j, g);		/* Remove node i from integer IG */
}

/* i_rmreg: make unavailable the registers of all nodes adjacent to node i */
static inline void i_rmreg (i_local_t i, bvt v, void *x) {
     i_local_t j = i + (i_local_t)x;
     if (SCLASS(j) == REGISTER && !PARAMP(j))
	  i_reg_use(i_ty2sc[TYPE(j)], ADDR(j));
}

/*
 * Graph coloring
 */

#define push(obj,stk,cnt) do { (stk)[(cnt)++] = (obj); } while (0)
#define pop(obj,stk,cnt) do { (obj) = (stk)[--(cnt)]; } while (0)

/* i_ig_color: color the interference graph g for storage class (INT/FP) sc */
void i_ig_color (int sc) {
     ig_t g, g2;		/* Backup of interference graph */

     i_local_t *nstk;		/* Node stack: when removing a node from IG,
				   push it onto this stack */
     unsigned int ncnt = 0;	/* Counter for node stack */

     unsigned int nnodes;	/* No. of nodes left in IG when coloring */
     unsigned int nremoved;	/* No. of nodes removed in 1 coloring pass */

     i_local_t i, j;		/* Dummy variable indices / counters */
     unsigned int k;
     unsigned int reg;		/* Temporary color variable */

     unsigned int ncolors;	/* Number of available registers */

				/* Data for prioritized spilling */
     i_ref_t threshref = 0.;	/* Threshold refcnt (see below) */
     i_local_t *deferspill = 0;	/* Maintains info of which spills have been 
				   deferred */
     i_local_t Vnum, Vbase;	/* Number of vars of sclass sc, and their
				   base offset in i_locals */
#ifndef NDEBUG
     int dcnt = 0;
#endif

     if (sc == FP) {
	  g = fig; Vnum = num_f; Vbase = i_loc_ilim;
     } else {
	  g = iig; Vnum = num_i; Vbase = 0;
     }
     ncolors = i_reg_navail(sc);/* Find number of available registers */

     NEW(nstk, Vnum);		/* Allocate register stack */
     g2 = i_igcopy(g);		/* Make backup copy of IG */

     NEW0(removed, Vnum);	/* Mark whether a node has been removed */

     DEBUG(i_igunparse(g));

     if (i_dorefcnt) {		/* Find maxref */
	  i_ref_t t = 0.;
	  unsigned n = 0;
	  NEW0(deferspill, Vnum);
	  for (i = 0; i < Vnum; i++) {
	       j = i + Vbase;	/* Get real index of this variable */
	       if (! (PARAMP(j) || SCLASS(j) == STACK)) {
		    t += REFCNT(j); n++;
	       }
	  }
	  threshref = t/(n*2);	/* For any clique (V,E) where |V| > # regs,
				   spill first all v with refcnt < threshref;
				   this is a little bogus (the right way would
				   be to sort the v \in V and spill in order
				   of increasing refcnt), but fast, and it
				   works pretty well. */
     }
				/* 
				 * Dismantle interference graph
				 */
     do {
	  nremoved = 0;		/* Reset number of nodes removed */
	  nnodes = 0;		/* And number of nodes remaining */
				/* Loop over all variables (ie. nodes in IG) */
	  for (i = 0; i < Vnum; i++) {
	       if (removed[i])
		    continue;
	       j = i + Vbase;	
				/* Skip params: already allocated */
	       if (PARAMP(j)) {
		    i_igremove(i, g);
		    continue;
	       }
				/* Remove non-register candidates */
	       if (SCLASS(j) == STACK) {
		    i_igspill(j, i, g);
		    continue;
	       }
				/* Find valence of node i */
	       k = bv_num(row(i, g));
	       if (k == 0) {
		    removed[i] = 1;
		    continue;
	       }
	       if (k <= ncolors) {
				/* Valence <= number of colors: can color.
				   Store index of node i for later use  */
		    push(i, nstk, ncnt);
				/* Remove node i from graph */
		    i_igremove(i, g);
		    ++nremoved;	/* Increase the counter of removed nodes */
	       } else
		    ++nnodes;	/* Can't color: increase number of nodes
				   left in graph */
	  }
     } while (nnodes > 0 && nremoved > 0);

     while (nnodes > 0) {	/* Try nodes with high valence/low refcount */
	  i_local_t target = i_zero;
	  unsigned int mc = 0;
#ifndef NDEBUG
	  if (i_debug) {
	       printf("Initial spill loop, #%d (nnodes=%d,dorefcnt=%d)\n", 
		      dcnt++, nnodes, i_dorefcnt);
	  }
#endif
	  for (i = 0; i < Vnum; i++) {
	       if (removed[i])
		    continue;
	       j = i + Vbase;
				/* Find valence of node i */
	       k = bv_num(row(i, g));
	       assert(k);
	       if (k <= ncolors) {
				/* Can color: evidently we removed some of 
				   this node's neighbors on a previous 
				   iteration of this inner loop */
				/* Store index of node i for later use */
		    push(i, nstk, ncnt);
				/* Remove node i from graph */
		    i_igremove(i, g);
		    --nnodes;	/* We have removed a node */
		    if (!nnodes)
			 break;
	       } else if (REFCNT(j) < 2*threshref && k > mc) {
		    mc = k;
		    target = i;
	       } 
	  }
	  if (nnodes)
	       if (target != i_zero) {
		    DEBUG(printf("Spilling(1) %d[n=%d,d=%d,k=%d,c=%f,t=%f]\n",\
				 target, nnodes, deferspill[target], mc, \
				 REFCNT(target+Vbase), threshref));
		    i_igspill(target+Vbase, target, g);
		    DEBUG(i_igunparse(g));
		    nnodes--;
	       } else {		/* Found nothing to spill here */
		    assert(target == i_zero);
		    break;
	       }
     }
     DEBUG(dcnt = 0);
     while (nnodes > 0) {	/* Final, aggressive spill loop */
#ifndef NDEBUG
	  if (i_debug) {
	       printf("Final spill loop, #%d (nnodes=%d)\n", dcnt++, nnodes);
	  }
#endif	  
	  for (i = 0; i < Vnum; i++) {
	       if (removed[i])
		    continue;
	       j = i + Vbase;
				/* Find valence of node i */
	       k = bv_num(row(i, g));
	       assert(k);
	       if (k <= ncolors) {
				/* Can color: evidently we removed some of 
				   this node's neighbors on a previous 
				   iteration of this inner loop */
				/* Store index of node i for later use */
			 push(i, nstk, ncnt);
				/* Remove node i from graph */
			 i_igremove(i, g);
	       } else if (!i_dorefcnt || deferspill[i] > 1 
			  || REFCNT(j) < 3*threshref) {
				/* Small refcnt or already deferred: spill. */
		    DEBUG(printf("Spilling(2) %d[n=%d,d=%d,k=%d,c=%f,t=%f]\n",\
				 i, nnodes, deferspill[i], k, \
				 REFCNT(j), threshref));
		    i_igspill(j, i, g);
		    DEBUG(i_igunparse(g));
	       } else {		/* This node is referenced a lot; try deferring
				   the spill. */
		    ++deferspill[i];
		    continue;	/* Do not remove this node, yet */
	       }
	       --nnodes;	/* We have removed a node */
	  }
     }
				/*
				 * Rebuild interference graph, coloring nodes
				 */
     while (ncnt) {
				/* Find a new node to add back to the IG */
	  pop(i, nstk, ncnt);
				/* Make unavailable the registers of all nodes
				   adjacent to node i in the IG */
	  bv_eachbit(row(i, g2), i_rmreg, (void *)Vbase);
	  reg = i_reg_get(sc);	/* Get an int register */
	  j = i + Vbase;
	  ADDR(j) = reg;
	  SCLASS(j) = REGISTER;	/* Set storage location info */
	  i_reg_mask(sc);	/* Reset the set of available registers */
     }
     i_reg_resetmask(sc);
}
#undef push
#undef pop

/*
 * Building the interference graph
 */

#define gen(val) do {					\
     i_local_t j = (val);				\
     if (sc_var(j)) {					\
	  if (sc_fp(j)) nfi |= bv_setf(fv, fp_id(j));	\
     	  else nii |= bv_setf(iv, j);			\
     }							\
} while (0)

#define kill(val) (j = (val), 					\
		   sc_var(j) ? (sc_fp(j) ? 			\
				bv_unsetf(fv, fp_id(j)) :	\
				bv_unsetf(iv, j)) :		\
		   0)

#define updateig() do {					\
     if (nii) { i_igclique(iig, iv); nii = 0; }		\
     if (nfi) { i_igclique(fig, fv); nfi = 0; }	\
} while (0)

/* i_ig_build: collect live variable information from the flow graph, and
   build up the interference graph */
void i_ig_build (void) {
     bvt iv=0, fv=0;		/* Temp bit vector */
     i_bb_t b;			/* Temp bblock */
     i_puint32 cp;		/* Code pointer */
     unsigned int op;		/* Current opcode */
     unsigned int nii, nfi;	/* Marks new int/float liveness info */
     i_local_t j;		/* Temporary local for use in macros */

     if (num_i)		/* Create new integer IG */
	  iig = i_igcreate(num_i);
     if (num_f)		/* Create new float IG */
	  fig = i_igcreate(num_f);
     b = i_fg_root;
     while (b) {
	  iv = b->ilv_out;	/* We will modify b->lv_[f]out, since we no */
	  fv = b->flv_out;	/* longer need them */
	  assert(b->t >= b->h);
	  for (cp = b->t; cp >= b->h; cp -= i_isize) {
	       op = get_op(cp);
	       nii = nfi = 0;
	       switch(i_op2class[op]) {
	       case I_MOPW:	case I_MOPWF:
		    gen(get_rd(cp)); gen(get_rs(cp));
		    gen(get_rs2(cp));
                    updateig();
		    break;
	       case I_MOPWI:	case I_MOPWIF:
		    gen(get_rd(cp)); gen(get_rs(cp));
                    updateig();
		    break;
	       case I_MOPR:	case I_BOP:
	       case I_MOPRF:	case I_BOPF:
	       case I_MOPRI:	case I_BOPI:	case I_MOPRIF:
				/* Substract def */
		    if (! kill(get_rd(cp))) { mk_nop(cp); break; }
				/* Fall through */
	       case I_BR:	case I_BRI:	case I_BRF:
		    gen(get_rs(cp));
		    if (!isimmed(op)) gen(get_rs2(cp));
		    updateig();
		    break;
	       case I_UOP:	case I_UOPI:	case I_UOPF:
	       case I_LEA:	case I_LEAF:
				/* Substract def */
		    if (!kill(get_rd(cp))) { mk_nop(cp); break; }
				/* Add use */
		    if (!isimmed(op)) gen(get_rs(cp));
		    updateig();
		    break;
	       case I_SET:	case I_SETF:
				/* Subtract def */
		    if (!kill(get_rd(cp)))
			 mk_nop(cp);
		    break;
	       case I_ARG:	case I_ARGF:
	       case I_RET:	case I_RETI:	case I_RETF:	case I_JMP:
				/* Add use */
		    if (op != i_op_retv && !isimmed(op)) {
			 gen(get_rd(cp));
			 updateig();
		    }
		    break;
	       case I_CALL:	case I_CALLF:
	       case I_CALLI:	case I_CALLIF:
				/* Substract def */
		    if (op != i_op_callv && op != i_op_callvi)
			 kill(get_rd(cp));
				/* Handle caller-saved registers */
		    if (num_f) fcallsav(cp, bv_cp(fv));
		    if (num_i) icallsav(cp, bv_cp(iv));
				/* Add use of callee if not immediate */
		    if (!isimmed(op)) { gen(get_rs(cp)); updateig(); }
		    break;
	       case I_MISC:	case I_JMPI:
		    continue;
	       default: assert(0);
	       }
	  }
	  b = b->lnext;
     }
}

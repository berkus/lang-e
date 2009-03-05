#include "icode.h"
#include "icode-internal.h"

/*
 * Basic block code
 */

i_cnt_t i_nbb, num_bb;

/* bb_alloc: return a new, empty basic block */
static inline i_bb_t bb_alloc (void) {
     i_bb_t b;
     num_bb++;
     NEW0(b, 1);
     if (num_i) {
	  b->iv_def = bv_new(num_i);
	  b->iv_use = bv_new(num_i);
	  b->ilv_out = bv_new(num_i);
     }
     if (num_f) {
	  b->fv_def = bv_new(num_f);
	  b->fv_use = bv_new(num_f);
	  b->flv_out = bv_new(num_f);
     }
     return b;
}

i_bb_t i_bb_new (unsigned maxlen, i_bb_t lprev) {
     i_bb_t b = bb_alloc();
     
     if (lprev) {
	  lprev->lnext = b;
	  b->lprev = lprev;
	  if (lprev == i_fg_tail)
	       i_fg_tail = b;
     } else {
	  b->lnext = i_fg_root;
	  i_fg_root->lprev = b;
	  i_fg_root = b;
     }

     NEW(i_ip, maxlen);
     i_buf = i_ip;
     i_lim = i_buf+maxlen;

     return b;
}

void i_bb_update (i_bb_t b) {
     assert(0);
}

void i_bb_insertafter (i_bb_t b, i_puint32 pos, unsigned int len) {
     assert(0);
}

void i_bb_insertbefore (i_bb_t b, i_puint32 pos, unsigned int len) {
     assert(0);
}

void i_bb_addpred (i_bb_t b, i_bb_t pred) {
     assert(0);
}

void i_bb_delpred (i_bb_t b, i_bb_t pred) {
     assert(0);
}

/*
 * Basic block routines to support building the flow graph
 */

typedef struct {		/* Data structure to resolve forward refs */
     i_bb_t src;
     i_label_t dst;
} fwdref_t;

static fwdref_t *fwdrefs;
static i_cnt_t fwdref_cur;
static i_bb_t *lbl2bb;		/* Map from label to corresponding bblock */


/* label2bb: given a label l, return the bb that it labels */
static inline i_bb_t label2bb (i_label_t l) {
     assert(l >= min_lab && l < i_lab_cur);
     return lbl2bb[l];
}

/* bb_linkbb: make p the predecessor of s and s the successor of p */
static inline void bb_linkbb (i_bb_t p, i_bb_t s) {
     assert(p && s);
     addtolist(i_bb_t, p->n, s, p->nn, p->ln, 4);
     addtolist(i_bb_t, s->p, p, s->np, s->lp, 4);
}

/* bb_linklbl: make p the predecessor of the basic block starting with 
   label l: if the block does not exist, create a forward reference that
   will be resolved later. */
static inline void bb_linklbl (i_bb_t p, i_label_t l) {
     i_bb_t s = label2bb(l);
     assert(p);
     if (!s) {
	  fwdrefs[fwdref_cur].src = p;
	  fwdrefs[fwdref_cur].dst = l;
	  fwdref_cur++;
     } else {
	  addtolist(i_bb_t, p->n, s, p->nn, p->ln, 4);
	  addtolist(i_bb_t, s->p, p, s->np, s->lp, 4);
     }
}

/* bb_finalize: finish the basic block b ending at cp.
   If there is more code (cp+step < lp), return a new basic block to hold it.
   If link is true, make b a predecessor of the new basic block. */
static inline i_bb_t bb_finalize (i_bb_t b, icode_t cp, icode_t lp, 
				  unsigned step, unsigned link) {
     assert(!b->init);
     b->init = 1;
     b->t = cp;
     if (cp+step < lp) {
	  i_bb_t newbb = bb_alloc();
	  newbb->h = cp + step;
	  b->lnext = newbb;
	  newbb->lprev = b;
	  if (link)
	       bb_linkbb(b, newbb);
	  return newbb;
     }
     return b;
}

/*
 * Building the flow graph
 */

i_bb_t i_fg_root, i_fg_tail;

static inline void markuse (i_bb_t b, i_local_t var) {
     i_local_t j = var;
     if (!sc_var(j)) return;
     if (sc_fp(j)) 		
	  bv_cset(b->fv_use, b->fv_def, fp_id(j));
     else
	  bv_cset(b->iv_use, b->iv_def, j);
}

static inline void markdef (i_bb_t b, i_local_t var) {
     i_local_t j = var;     
     if (!sc_var(j)) return;
     if (sc_fp(j)) 		
	  bv_cset(b->fv_def, b->fv_use, fp_id(j));
     else
	  bv_cset(b->iv_def, b->iv_use, j);
}

static inline void markvar (i_bb_t b, i_local_t var) {
     if (!sc_var(var)) return;
     if (sc_fp(var))
	  bv_set(b->fv_use, fp_id(var));
     else
	  bv_set(b->iv_use, var);
}

i_puint32 *i_calls;
i_cnt_t i_calls_cur, i_calls_lim;

static inline void markcall (i_puint32 cp) {
     growlist(i_puint32, i_calls, i_calls_cur, i_calls_lim, dn_calls);
     i_calls[i_calls_cur++] = cp;
}

void i_fg_build (void) {
     unsigned i;
     i_puint32 cp = i_buf;	/* Start of code to analyze */
     i_puint32 lp = i_lim;	/* End of code to analyze */
     i_uint32 op;		/* Current opcode */
     i_bb_t b;			/* Current basic block */

     num_bb = 0;
     b = bb_alloc();
     b->h = cp;

     if (i_ralloctype == RA_EZ) {
	  i_fg_root = i_fg_tail = b;
	  b->t = lp - i_isize;
	  return;
     }

     i_calls_cur = i_calls_lim = 0;
     NEW0(lbl2bb, i_lab_cur); 
     NEW(fwdrefs, i_nbb); fwdref_cur = 0;

     i_fg_root = b;
     do {
	  assert(b && !b->init);
	  op = get_op(cp);
	  switch(i_op2class[op]) {
	  case I_BOP:	  case I_BOPF:
	  case I_MOPR:	  case I_MOPRF:
	       assert(!isimmed(op));
	       markuse(b, get_rs(cp));
	       markuse(b, get_rs2(cp));
	       markdef(b, get_rd(cp));
	       break;
	  case I_BOPI:	  case I_MOPRI:	case I_MOPRIF:
	       assert(isimmed(op));
	       markuse(b, get_rs(cp));
	       markdef(b, get_rd(cp));
	       break;
	  case I_MOPW:	  case I_MOPWF:
	       assert(!isimmed(op));
	       markuse(b, get_rd(cp));
	       markuse(b, get_rs(cp));
	       markuse(b, get_rs2(cp));
	       break;
	  case I_MOPWI:	  case I_MOPWIF:
	       assert(isimmed(op));
	       markuse(b, get_rd(cp));
	       markuse(b, get_rs(cp));
	       break;
	  case I_UOP:     case I_UOPF:
	       assert(!isimmed(op));
	       markuse(b, get_rs(cp));
	       markdef(b, get_rd(cp));
	       break;
	  case I_UOPI:
	       assert(isimmed(op));
	       markdef(b, get_rd(cp));
	       break;
	  case I_SET:	   case I_SETF:
	       markdef(b, get_rd(cp));
	       break;
	  case I_LEA:	   case I_LEAF:
	       SCLASS(get_rs(cp)) = STACK;
	       markuse(b, get_rs(cp));
	       markdef(b, get_rd(cp));
	       break;
	  case I_RET:	   case I_RETF:
	       if (op != i_op_retv)
		    markuse(b, get_rd(cp));
	  case I_RETI:
	       b = bb_finalize(b, cp, lp, i_isize, false);
	       break;
	  case I_BR:	  case I_BRF:
	       markuse(b, get_rs2(cp));
	  case I_BRI:
	       markuse(b, get_rs(cp));
	       bb_linklbl(b, get_rd(cp));
	       b = bb_finalize(b, cp, lp, i_isize, true);
	       break;
	  case I_CALL:	  case I_CALLF:
	       markuse(b, get_rs(cp));
	  case I_CALLI:	  case I_CALLIF:
	       if (op != i_op_callv && op != i_op_callvi)
		    markdef(b, get_rd(cp));
	       markcall(cp);
	       b = bb_finalize(b, cp, lp, 2*i_isize, true);
	       cp += i_isize;	/* Calls are 2x as long as other insns */
	       break;
	  case I_ARG:	  case I_ARGF:
	       markuse(b, get_rd(cp));
	       break;
	  case I_JMP:
	       assert(get_rd(cp) < num_i);
	       markuse(b, get_rd(cp));
	       b = bb_finalize(b, cp, lp, i_isize, false);
	       break;
	  case I_JMPI:
	       bb_linklbl(b, get_imm(cp));
	       b = bb_finalize(b, cp, lp, i_isize, false);
	       break;
	  case I_MISC:
	       switch (op) {
	       case i_op_lbl:
		    if (cp > b->h) /* If not at head of current block ... */
				/* ... make this the head of a new block */
			 b = bb_finalize(b, cp-i_isize, lp, i_isize, true);
		    lbl2bb[get_rd(cp)] = b;
		    break;
	       default:		/* refmul, refdiv, self, nop */
		    break;
	       }
	       break;
	  default:
	       assert(0);
	  }
     } while ((cp += i_isize) < lp);
     i_fg_tail = b;
     for (i = 0; i < fwdref_cur; i++)
	  bb_linkbb(fwdrefs[i].src, lbl2bb[fwdrefs[i].dst]);
#ifndef NDEBUG
     for (i = 0, b = i_fg_root; b; b = b->lnext) i++;
     assert(i == num_bb);
#endif
}

/*
 * Live variables
 */

/* 
   The idea of reducing work when doing a linear ("flat") scan of basic blocks
   (rather than reverse dfs through the flow graph) by keeping counters of what
   work remains to be done is due to Eddie Kohler.
*/

static int iupdates;	/* Updates to flow info still outstanding */
static int fupdates;

/* lvrelax: compute live variable info for block b if necessary (if
   updates have been performed by any of its successors), or in any case if
   force is true. */
static void lvrelax (i_bb_t b, int force) {
     unsigned int i;
     bvt out, fout;		/* "Out" bit vectors */
     bvt in, fin;		/* "In" bit vector */
     i_bb_t *n = b->n;		/* List of successors */
     i_bb_t *p = b->p;		/* List of predecessors */
     unsigned int nn = b->nn;	/* Numbers of predecessors and successors */
     unsigned int np = b->np;

     assert(b);
				/* Either this is the first round or we are
				   here because we need to update something */
     assert(force || b->iupdates+b->fupdates > 0);
     if (nn == 0) {		/* Sinks: in = use */
	  assert(force);	/* Sinks are touched only once */
	  if (num_i) {	/* Handle ints */
	       b->ilv_in = bv_cp(b->iv_use);
				/* Increase updates for each predecessor */
	       for (i = 0; i < np; i++)
		    ++p[i]->iupdates;
	       iupdates += np;	/* Increase updates count */
	  }
	  if (num_f) {	/* Handle floats */
	       b->flv_in = bv_cp(b->fv_use);
	       for (i = 0; i < np; i++)
		    ++p[i]->fupdates;
	       fupdates += np;
	  }
	  return;		/* Nothing more to be done for sinks */
     }
				/* Handle integers */
     if (num_i && (b->iupdates || force)) {
	  out = bv_cp(b->ilv_out);
				/* out = \sum_{s\in succ}{in(s)}  */
	  for (i = 0; i < nn; i++) {
	       assert(n[i]);
	       if (n[i]->ilv_in)
		    bv_union(out, n[i]->ilv_in);
	  }
				/* in = use + (out - def) */
	  if (!bv_eq(b->ilv_out, out)) {
	       b->ilv_out = out;
	       in = bv_cp(out);
	  } else
	       in = out;	/* We don't use 'out': optimize away bv_cp */
	  bv_diff(in, b->iv_def);
	  bv_union(in, b->iv_use);

	  iupdates -= b->iupdates;/* This node has just been updates */
	  assert(iupdates >= 0);
	  b->iupdates = 0;

	  if (!b->ilv_in || !bv_eq(b->ilv_in, in)) {
	       b->ilv_in = in;
	       for (i = 0; i < np; i++)
		    ++p[i]->iupdates;
	       iupdates += np;	/* Increase updates count; always >= 0 */
	  }
     }
				/* Handle floats */
     if (num_f && (b->fupdates || force)) {
	  fout = bv_cp(b->flv_out);
				/* out = \sum_{s\in succ}{in(s)}  */
	  for (i = 0; i < nn; i++) {
	       assert(n[i]);	
	       if (n[i]->flv_in)
		    bv_union(fout, n[i]->flv_in);
	  }
				/* in = use + (out - def) */
	  if (!bv_eq(b->flv_out, fout)) {
	       b->flv_out = fout;
	       fin = bv_cp(fout);
	  } else
	       fin = fout;	/* We don't use 'out': optimize away bv_cp */
	  bv_diff(fin, b->fv_def);
	  bv_union(fin, b->fv_use);

	  fupdates -= b->fupdates;
	  assert(fupdates >= 0);
	  b->fupdates = 0;

	  if (!b->flv_in || !bv_eq(b->flv_in, fin)) {
	       b->flv_in = fin;
	       for (i = 0; i < np; i++)
		    ++p[i]->fupdates;
				/* Increase updates count; always >= 0 */
	       fupdates += np;
	  }
     }
}

/* i_livevars: do live variable analysis.
   Requires that i_fg_build have been called. */
void i_fg_lv (void) {
     i_bb_t b = i_fg_tail;
     iupdates = fupdates = 0;
     while (b) {
	  lvrelax(b, true);
	  b = b->lprev;
     }
     do {			/* Iterative relaxation over predecessors */
	  b = i_fg_tail;
	  while (b) {
	       if (b->iupdates || b->fupdates)
		    lvrelax(b, false);
	       b = b->lprev;
	  }
     } while (iupdates || fupdates);
}

/*
 * Flow graph traversal
 */

void i_fg_bfs (i_bbf_t f) {
     assert(0);
}

static void fg_dfs_visit (i_bb_t b, i_bbf_t before, i_bbf_t after) {
     i_bb_t *n = b->n;		/* List of successors */
     unsigned int nn = b->nn;	/* Number of successors */
     unsigned int i;

     b->color = BLACK;
     if (before) before(b);
     for (i = 0; i < nn; i++)	/* Recurse on successors */
	  if (n[i]->color == WHITE)
	       fg_dfs_visit(n[i], before, after);
     if (after) after(b);
}

void i_fg_dfs (i_bbf_t before, i_bbf_t after) {
     i_bb_t b;
     for (b = i_fg_root; b; b = b->lnext)
	  b->color = WHITE;
     fg_dfs_visit(i_fg_root, before, after);
}

void i_fg_rdfs (i_bbf_t f) {
     assert(0);
}

void i_fg_fwd (i_bbf_t f) {
     i_bb_t b = i_fg_root;
     while (b) {
	  f(b);
	  b = b->lnext;
     }
}

void i_fg_bwd (i_bbf_t f) {
     i_bb_t b = i_fg_tail;
     while (b) {
	  f(b);
	  b = b->lprev;
     }
}

/*
 * Unparsing
 */

/* unparsebit: print out the number of bit n */
static inline void unparsebit (unsigned int n, bvt v, void *x) {
     printf("%d ", n);
}

/* i_bb_unparse: unparse the basic block b */
void i_bb_unparse (i_bb_t b) {
     i_puint32 cp;
     unsigned int i;

     printf("BB dfn=%d addr=%p\n\tPredecessors:\n\t\t", b->dfn, b->h);
     for (i = 0; i < b->np; i++) {
	  assert(b->p[i]);
	  printf("%p ", b->p[i]->h);
     }
     printf("\n\tSuccessors:\n\t\t");
     for (i = 0; i < b->nn; i++) {
	  assert(b->n[i]);
	  printf("%p ", b->n[i]->h);
     }
     if (num_i) {
	  printf("\n\tInteger def: ");
	  bv_eachbit(b->iv_def, unparsebit, 0);
	  printf("\n\tInteger use: ");
	  bv_eachbit(b->iv_use, unparsebit, 0);
	  printf("\n\tInteger live out: ");
	  bv_eachbit(b->ilv_out, unparsebit, 0);
	  if (b->ilv_in) {
	       printf("\n\tInteger live in: ");
	       bv_eachbit(b->ilv_in, unparsebit, 0);
	  }
     }
     if (num_f) {
	  printf("\n\tFloat def: ");
	  bv_eachbit(b->fv_def, unparsebit, 0);
	  printf("\n\tFloat use: ");
	  bv_eachbit(b->fv_use, unparsebit, 0);
	  printf("\n\tFloat live out: ");
	  bv_eachbit(b->flv_out, unparsebit, 0);
	  if (b->flv_in) {
	       printf("\n\tFloat live in: ");
	       bv_eachbit(b->flv_in, unparsebit, 0);
	  }
     }
     printf("\n\tCode:\n");
     for (cp = b->h; cp <= b->t; cp += i_isize)
	  i_unparseinsn(stdout, cp);
}

static i_cnt_t dfn;
static inline void fg_dfn (i_bb_t b) {
     b->dfn = dfn --;
}

/* i_fg_unparse: unparse the whole flow graph */
void i_fg_unparse (void) {
     i_bb_t b;
     dfn = num_bb;
     i_fg_dfs(0 /* before */, fg_dfn /* after */);
     for (b = i_fg_root; b; b = b->lnext) 
	  i_bb_unparse(b);
}

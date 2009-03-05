/* $Id: li-scc.c,v 1.6 1998/06/24 18:09:06 maxp Exp $ */

/* Fast live interval analysis */

#include <assert.h>
#include "icode.h"
#include "icode-internal.h"
#include "mem.h"

/*
 * Depth-first numbering
 */

static i_cnt_t dfn;		/* Depth first number */
static i_bb_t *nodes;		/* Vector of bblocks in reverse dfn order */

static void dfs_visit (i_bb_t b) {
     i_bb_t *n = b->n;		/* List of successors */
     unsigned int nn = b->nn;	/* Number of successors */
     unsigned int i;

     b->color = BLACK;
     for (i = 0; i < nn; i++)	/* Recurse on successors */
	  if (n[i]->color == WHITE)
	       dfs_visit(n[i]);
     nodes[--dfn] = b;
#ifndef NDEBUG
     b->dfn = dfn;
#endif
}

static void dfs (void) {
     i_bb_t b;
     for (b = i_fg_root; b; b = b->lnext)
	  b->color = WHITE;
     dfn = num_bb+1;
     NEW(nodes, dfn); 
     dfs_visit(i_fg_root);	/* nodes[dfn..num_bb] stores visited nodes */
     assert(dfn);
}

/*
 * Strongly-connected components
 */

static i_cnt_t scn;		/* SCC number */
scc_t i_sccs;

static void scc_visit (i_bb_t b) {
     i_bb_t *p = b->p;		/* List of predecessors */
     unsigned int i, np = b->np;/* Number of predecessors */

     b->color = BLACK;
     b->scn = scn;

     if (num_i) bv_union2s(i_sccs[scn].iv, b->iv_use, b->iv_def);
     if (num_f) bv_union2s(i_sccs[scn].fv, b->fv_use, b->fv_def);

     for (i = 0; i < np; i++)	/* Recurse on predecessors */
	  if (p[i]->color == WHITE)
	       scc_visit(p[i]);
}

static void scc_mark (void) {
     unsigned int i;
     i_bb_t b;
     for (b = i_fg_root; b; b = b->lnext)
	  b->color = WHITE;
     scn = 0;
     NEW(i_sccs, num_bb+1);
     for (i = dfn; i <= num_bb; i++)
	  if (nodes[i]->color == WHITE) {
	       ++scn;
	       if (num_i) i_sccs[scn].iv = bv_new(num_i);
	       if (num_f) i_sccs[scn].fv = bv_new(num_f);
	       scc_visit(nodes[i]);
	  }
}

#ifndef NDEBUG
static inline void unparsebit (unsigned int n, bvt v, void *x) {
     printf("%d ", n);
}
static void scc_unparse (void) {
     i_cnt_t i, k;
     i_bb_t b;
     printf("SCC: (total %d SCCs covering %d BBs)\n", scn, num_bb);
     for (i = 1; i <= scn; i++) {
	  printf("\t%d. Basic blocks: ", i);
	  k = 0;
	  for (b = i_fg_root; b; b = b->lnext)
	       if (b->scn == i) {
		    printf("%d ([0x%p,0x%p]), ", b->dfn, b->h, b->t);
		    k++;
	       }
	  if (!k) printf("\n\t\tERROR: no basic block covered by SCC.");
	  printf("\n");
	  if (num_i) {
	       printf("\t\tInts: "); 
	       bv_eachbit(i_sccs[i].iv, unparsebit, 0);
	       printf("\n"); 
	  }
	  if (num_f) {
	       printf("\t\tFloats: "); 
	       bv_eachbit(i_sccs[i].fv, unparsebit, 0);
	       printf("\n"); 
	  }
     }
     printf("BBs not covered by any SCC: ");
     for (i = 0, b = i_fg_root; b; b = b->lnext)
	  if (b->scn == 0) {
	       printf("%d ([0x%p,0x%p]), ", b->dfn, b->h, b->t);
	       i++;
	  }
     if (i == 0) printf("none\n");
     else printf("\n");
}
#endif

/*
 * Building live intervals
 */

static inline void lrinit (i_local_t l, i_cnt_t pos, int sc) {
     i_cnt_t tlr;
     i_lr_t lrs = sc == FP ? i_flrs : i_ilrs;

     if (LRANGE(l) == NOLRINFO) {
	  tlr = sc == FP ? i_flr_cur-- : i_ilr_cur--;
	  LRANGE(l) = tlr;
	  lrs[tlr].n = l;
	  lrs[tlr].beg = lrs[tlr].end = pos;
     } else {
	  tlr = LRANGE(l);
	  assert(lrs[tlr].beg > pos && lrs[tlr].end > pos);
	  lrs[tlr].beg = pos;
     }
}

static inline void ilrinit (i_local_t l, bvt vec, void *v) {
     i_cnt_t pos = (i_cnt_t)v;
     lrinit(l, pos, INT);
}

static inline void flrinit (i_local_t l, bvt vec, void *v) { 
     i_cnt_t pos = (i_cnt_t)v;
     lrinit(fp_mk(l), pos, FP);
}

static void scan_intervals (void) {
     i_cnt_t i;
     if (num_i) {		/* Allocate live ranges */
	  NEW0(i_ilrs, num_i);
	  i_ilr_cur = num_i-1; 
     }
     if (num_f) {
	  NEW0(i_flrs, num_f);
	  i_flr_cur = num_f-1;
     }
     
     if (num_i)
	  for (i = scn; i >= 1; i--)
	       bv_eachbit(i_sccs[i].iv, ilrinit, (void *)i);
     if (num_f) 
	  for (i = scn; i >= 1; i--)
	       bv_eachbit(i_sccs[i].fv, flrinit, (void *)i);
}

/*
 * Top-level function
 */

/* i_li_scc: do fast (scc-based) live interval analysis.
   Requires that i_fg_build have been called. */
void i_li_scc (void) {
     dfs();
     scc_mark();
     DEBUG(scc_unparse());
     scan_intervals();
     DEBUG(i_li_unparse());
}

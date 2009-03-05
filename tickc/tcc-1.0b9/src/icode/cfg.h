/* $Id: cfg.h,v 1.7 1998/05/17 20:38:31 maxp Exp $ */

#ifndef __CFG_H__
#define __CFG_H__

#include <assert.h>

/*
 * Basic blocks
 */

typedef struct i_bb_obj {	/* Basic block type */
     i_puint32 h;		/* Head */
     i_puint32 t;		/* Tail */

     struct i_bb_obj *lprev;	/* Layout successor */
     struct i_bb_obj *lnext;	/* Layout predecessor */
     i_cnt_t np;			/* Number of predecessors */
     i_cnt_t lp;			/* Limit of predecessor list */
     struct i_bb_obj **p;	/* Predecessors */
     i_cnt_t nn;			/* Number of successors */
     i_cnt_t ln;			/* Limit of successor list */
     struct i_bb_obj **n;	/* Successors */

     bvt ilv_in;		/* Int live variables in */
     bvt ilv_out;		/* Int live variables out */
     bvt iv_use;		/* Int variables used */
     bvt iv_def;		/* Int variables defined */

     bvt flv_in;		/* FP live variables in */
     bvt flv_out;		/* FP live variables out */
     bvt fv_use;		/* FP variables used */
     bvt fv_def;		/* FP variables defined */

     i_cnt_t iupdates;		/* Number of successors of this bb that have */
     i_cnt_t fupdates;		/*   changed their int and fp lv info */

     i_flag_t init;		/* 1 after bblock is initialized */
     i_flag_t color;		/* Status during searches */
     i_cnt_t scn;			/* Strongly connected component number */
     i_cnt_t dfn;			/* Depth-first number */
} *i_bb_t;

extern i_bb_t i_fg_root;	/* Flow graph entry point */
extern i_bb_t i_fg_tail;	/* End of flow graph in layout order */

static inline unsigned int i_bb_len (i_bb_t b) {
     return (b->t - b->h) / i_isize;
}

extern void i_bb_insertafter (i_bb_t b,  i_puint32 pos, unsigned int len);
extern void i_bb_insertbefore (i_bb_t b, i_puint32 pos, unsigned int len);
extern i_bb_t i_bb_new (unsigned maxlen, i_bb_t lprev);
extern void i_bb_update (i_bb_t b);
extern void i_bb_addpred (i_bb_t b, i_bb_t pred);
extern void i_bb_delpred (i_bb_t b, i_bb_t pred);
extern void i_bb_unparse (i_bb_t b);

/*
 * Flow graph and analyses
 */

typedef void (*i_bbf_t)(i_bb_t b); /* Type of func applied to BBs */

extern void i_fg_build (void);	/* Build flow graph */
extern void i_fg_lv (void);	/* Collect live var info */
extern void i_fg_unparse (void);/* Unparse the flow graph */

				/* Flow graph iterators */
extern void i_fg_bfs (i_bbf_t f);/* BFS */
extern void i_fg_dfs (i_bbf_t before, i_bbf_t after); /* DFS */
extern void i_fg_rdfs (i_bbf_t f); /* Reverse DFS */
extern void i_fg_fwd (i_bbf_t f); /* Forward in layout order */
extern void i_fg_bwd (i_bbf_t f); /* Back in layout order */

/*
 * Tracking calls
 */

extern unsigned i_calls_cur;
extern i_puint32 * i_calls;	/* Array of call addresses for FLR ralloc */

				/* Store caller-saved reg info at call sites */
static inline void icallsav (icode_t cp, bvt v) {
     *(cp+i_isize+0) = (unsigned)(v);
}
static inline void fcallsav (icode_t cp, bvt v) {
     *(cp+i_isize+1) = (unsigned)(v);
}

#endif

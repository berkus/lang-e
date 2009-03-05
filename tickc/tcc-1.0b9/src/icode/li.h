/* $Id: li.h,v 1.6 1998/05/17 20:38:37 maxp Exp $ */

#ifndef __LIVE_INTERVALS_H__
#define __LIVE_INTERVALS_H__

typedef struct {
     i_local_t n;		/* Local having this live range */
     i_cnt_t beg;			/* Begin and end of live range */
     i_cnt_t end;
} * i_lr_t;

extern i_lr_t i_ilrs;		/* Int and float register live ranges */
extern i_lr_t i_flrs;
extern i_cnt_t i_ilr_cur;		/* Live range counters */
extern i_cnt_t i_flr_cur;

typedef struct {		/* List of strongly connected components */
     bvt iv;
     bvt fv;
} * scc_t;

extern scc_t i_sccs;

extern void i_li_unparse (void);

#endif

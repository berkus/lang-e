/* $Id: reg.h,v 1.6 1998/05/08 15:59:42 maxp Exp $ */

#ifndef __REG_H__
#define __REG_H__

#include <vcode.h>
#include "cfg.h"

extern void i_regalloc (i_pmap_t params[]);
#define R(x) (1<<(x))

#include "icode-arch.h"

/* Graph coloring */
extern void i_ig_build (void);
extern void i_ig_color (int sc);

/* Linear scan */
extern void i_li_full (void);
extern void i_li_scc (void);
extern void i_ra_ls (void);

#include "reg-manager.h"

#endif

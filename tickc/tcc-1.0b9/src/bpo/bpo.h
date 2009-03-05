/* $Id: bpo.h,v 1.1.1.1 1997/12/05 01:25:42 maxp Exp $ */ 

#ifndef __BPO_H__
#define __BPO_H__

#include "btype.h"

typedef struct {		/* Variable storage */
     int ver;			/* Version counter */
     T val;			/* Value */
} var_t;

typedef struct {		/* Variable info for input words */
     int id;			/* Indices of variables */
     int sh;			/* Position of rightmost bit of each var */
     T msk;			/* Mask for extracting each var in v_ids */
} iv_t;

typedef struct {		/* One input word (T) for a rule */
     T f_msk;			/* Mask for fixed part */
     T f_val;			/* Value for fixed part */
     int nv;			/* Number of variables to match */
     iv_t *v;			/* Variables */
} iw_t;

typedef struct {		/* Variable info for output words */
     int id;			/* Indices of variables to emit */
     int sh;			/* Amount by which to shift each var */
} ov_t;

typedef struct {		/* One output word (T) for a rule */
     T f_val;			/* Fixed value to emit */
     int nv;			/* Number of variables to emit */
     ov_t *v;			/* Variables */
} ow_t;

typedef struct {
     int ni;			/* Number of input words */
     iw_t *ins;			/* Input words */
     int no;			/* Number of output words */
     ow_t *outs;		/* Output words */
     int *rd;
} rule_t;

typedef W (*nft)(W);		/* Type of buffer pointer increment function */

extern var_t bpoV[];		/* Variables */
extern int bpoNR;		/* Number of rules */
extern rule_t bpoR[];		/* Rules */

extern void bpoinit (void);
extern W brulematch (rule_t *r, W oc, W oh);
extern W brulerepl (rule_t *r, W ih, W il);
extern W bpo (W ih, W it, W il, 
	      W oh, W ot, W ol, 
	      nft nf, 
#ifndef __bpo_no_relo__
	      int *fdb, int *opb, 
#endif
	      int dp);

#endif

/* $Id: bpo-dummy.c,v 1.1 1997/12/11 03:58:35 maxp Exp $ */

#include "bpo.h"

W bpo (W ih, W it, W il, W oh, W ot, W ol, nft nf, 
#ifndef __bpo_no_relo__
       int *fdb, int *opb, 
#endif
       int dp) {
     return (W)0;
}

/* $Id: bpo.c,v 1.2 1997/12/11 01:26:11 maxp Exp $ */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "bpo.h"

#if defined(__sparc__) && !defined(__LCC__)
extern int printf();
#endif

#ifdef NDEBUG
#define DEBUG(x)
#else
#define DEBUG(x) x
#endif
#define PDEBUG(x)  DEBUG(if (debugp) printf x;)
#define PDEBUG2(x) DEBUG(if (debugp>1) printf x;)

#if !defined(__GNUC__) || defined(__STRICT_ANSI__)
#define inline
#endif

/* This file implements bpo, a binary peephole optimizer.
   See the function 'bpo' for details.

   Bpo requires pointers to an input buffer and a (sufficiently large)
   output buffer.  As it works, it pops items off the input buffer head (ih),
   appending the optimized code to the output buffer tail (ot).

   Some notational conventions:

     Output buffer
     ---------------------
     | a | b | . . .
     ---------------------
     ^               ^                   ^
     |-- oh (fixed)  |-- ot (moves)      |-- ol (fixed)

     Input buffer
                   ---------------------
                       . . . c | d | e |
                   --------------------- 
     ^                 ^               ^
     |-- il (fixed)    |-- ih (moves)  |-- it (fixed)

     i/o = input/output, h/t/l = head/tail/limit

   Replacing a pattern may grow ot up towards the output limit (ol) or
   ih down towards the input limit (il).  Exceeding the limit in either
   direction causes the program to abort.  The user is responsible for
   ensuring that the supplied limit is adequate, given the input and rule
   set.
 */

enum { NV=26 };			/* Max number of variables */
var_t bpoV[NV];			/* Array storing values of variables */

#define BSZ 512			/* Max rule output size (# of rewrite elts) */
#define HSZ 1024		/* Max history buffer size */

static int ntry;		/* Number of match attempts */
DEBUG(static int debugp;)	/* Debug flag */

/* bpoinit: initialize some bpo state
     Used internally by bpo.
     Must also be used by any direct client of brulematch and brulerepl.
 */
inline void bpoinit () {
     int i;			/* Reset variable version info */
     for (i = 0; i < NV; i++) bpoV[i].ver = 0;
     ntry = 1;
}

/* blengthmatch: check that a rule of length rl can be applied, given the
   current length history buffer (denoted by lhl and lhp).  This is useful when
   using variable-width instructions.  The length history buffer stores the
   lengths of all instructions recognized so far.  Running blengthmatch
   guarantees that matches/replacements will occur only on real instruction
   boundaries, even when binary substrings of one instruction are themselves
   valid instructions.

   Returns 1 if the rule may be applied, 0 otherwise.  */
static inline int blengthmatch (int *lhl, int *lhp, int rl) {
     int i = 0;
     while (--lhp >= lhl) {
	  i += *lhp;
	  if (i == rl) return 1;
     }
     return 0;
}

/* brulematch: try to match a binary rule
     r:  pointer to rule to be matched
     oc: current cursor in output buffer (start point for match attempt)
     oh: low limit of output buffer

   If r matches at oc (not exceeding oh), returns the final value of oc
   after the match.  If r does not match, returns 0L.
 */     
inline W brulematch (rule_t *r, W oc, W oh) {
#define __bpo_filter_const__
     int j, k;
     T v;			/* Value of current peephole window */
     iw_t *w;
     
#ifdef __bpo_filter_const__
     {
	  W p = oc;
	  for (j = r->ni-1; j >= 0; j--) {
	       if (GT(oh, p)) return 0L;
	       v = LOAD(p);
	       w = &r->ins[j];
	       PDEBUG2(("\tline %d [addr=0x%p, val=0x%x]\n", \
			j, p, (unsigned)v));
	       if ((v&w->f_msk) != w->f_val) {
		    PDEBUG2(("\t\tgave up on value 0x%x\n", \
			     (unsigned)w->f_val));
		    return 0L;
	       }
	       DECR(p);
	  }
     }
#endif
     for (j = r->ni-1; j >= 0; j--) {
#ifndef __bpo_filter_const__
	  if (GT(oh, oc)) return 0L;
#endif
	  w = &r->ins[j];
	  v = LOAD(oc);
#ifndef __bpo_filter_const__
				/* 1.1. Match the fixed part */
	  if ((v&w->f_msk) == w->f_val) {
#endif
				/* 1.2. Now try to match the variables */
	       for (k = 0; k < w->nv; k++) {
		    iv_t *iv = &w->v[k];
		    int id = iv->id;
		    if (bpoV[id].ver == ntry) {
			 if (bpoV[id].val != (v&iv->msk) >> iv->sh) {
			      PDEBUG2(("\t\tgave up on variable %d\n", id));
			      return 0L;
			 }
		    } else {
			 assert(bpoV[id].ver < ntry);
			 bpoV[id].ver = ntry;
			 bpoV[id].val = (v&iv->msk) >> iv->sh;
		    }
	       }
#ifndef __bpo_filter_const__
	  } else {
	       PDEBUG2(("\t\tgave up on value 0x%x\n", (unsigned)w->f_val));
	       return 0L;
	  }
#endif
	  DECR(oc);
     }
     INCR(oc);
     return oc;
}

/* brulerepl: replace a binary rule
     r:  pointer to rule to be replaced
     ih: cursor at head of input buffer (start point for replacement)
     il: low limit of input buffer

   Performs replacement and returns new value of ih.
 */
inline W brulerepl (rule_t *r, W ih, W il) {
     T v;			/* Value of current peephole window */
     int j, k;
     for (j = r->no-1; j >= 0; j--) {
	  ow_t *w = &r->outs[j];
	  assert(!GT(il, ih));
	  for (v = w->f_val, k = 0; k < w->nv; k++)
	       v |= (bpoV[w->v[k].id].val << w->v[k].sh);
	  PDEBUG2(("\tstoring repl %d (0x%x) at 0x%p\n", j, (unsigned)v, ih));
	  STORE(ih, v); DECR(ih);
     }
     INCR(ih);
     return ih;
}

/* bpo: binary peephole optimizer
     ih/it/il: head/tail/low limit of input buffer
     oh/ot/ol: head/tail/high limit of output buffer
     nf: pointer to "next function"
     fdb: fixed data buffer
     opb: output permutation buffer
     dp: debug flag (1=="debug info on")

   Given a pointer into the input buffer, nf should return a pointer
   to the next location where optimization should be attempted.  It
   need not do anything special when the input cursor reaches the
   input tail: bpo checks this itself.  nf==0 implies the trivial
   next function: nf(p) = p+1.
   
   fdb is a buffer of size (it-ih): it contains a '1' wherever the buffer
   beginning at ih contains "fixed data" (data that should not be changed
   by bpo), and a '0' elsewhere.

   opb is the output permutation buffer, of size at least (ot-oh).
 
   Compiling with -D__bpo_no_relo__ removes fdb and opb from the bpo
   signature, and suppresses all code that deals with fixed data or
   output code permutation. */
W bpo (W ih, W it, W il, W oh, W ot, W ol, nft nf, 
#ifndef __bpo_no_relo__
       int *fdb, int *opb, 
#endif
       int dp) {
#ifndef __bpo_no_relo__
     int *pot = opb, *pih = 0;	/* "Permutation" output tail / input head */
     int *fot = 0, *fih = fdb;	/* "Fixed data" output tail / input head */
     int *fol = 0, *pil = 0;	/* "Fixed data" out low; "perm" in low */
#endif
     int *lhl = 0, *lhp = 0;	/* Low limit / current pointer to insn
				   length history buffer (necessary when
				   doing opts on variable-width insns) */
     int tnf = nf == (nft)0;	/* True if using trivial next function */
     int i, n = 0;
     DEBUG(int npass = 0; int nrepl = 0; debugp = dp;)
     assert(bpoNR >= 0 && oh < ol && il <= ih && il < it);
     bpoinit();

#ifndef __bpo_no_relo__
     if (opb) {			/* Initialize output permutation vector */
	  int k = it-ih;
	  int *p, *pit;
	  pil = (int *)malloc((2*k+1)*sizeof(int));
	  pih = pil+k; pit = pih+k;
	  for (p = pih, i = 0; p < pit; *p++ = i++);
     }
     if (fdb)
	  fol = fot = (int *)malloc((ol-oh)*sizeof(int));
#endif
     if (!tnf)
	  lhp = lhl = (int *)malloc(HSZ*sizeof(int));

nextpass:
     while (GT(it, ih)) {
	  assert(GT(ol, ot)); DEBUG(++npass);
				/* Head(output) <-- head(input) */
	  if (!tnf) {
	       for (n = nf(ih)-ih-1, i = 0; i < n; i++, INCR(ih), INCR(ot)) 
		    ASGN(ot, ih);
	       assert(lhp < &lhl[HSZ]);
	       *lhp++ = n+1;
	  }
	  ASGN(ot, ih);		/* Unroll last ASGN: avoid DECR(ot),DECR(ih) */

	  PDEBUG2(("ih=0x%p, it=0x%p, il=0x%p, oh=0x%p, ot=0x%p, ol=0x%p\n", \
		  ih, it, il, oh, ot, ol)); 
#ifndef __bpo_no_relo__
	  if (opb) {
	       for (i = 0; i < n; i++) *pot++ = *pih++;
	       *pot = *pih;
	  }
	  if (fdb) {
	       for (i = 0; i < n; i++) *fot++ = *fih++;
	       *fot = *fih;
	  }
#endif
	  for (i = 0; i < bpoNR; i++, ntry++) {
	       W tp;
	       PDEBUG2(("trying rule %d\n", i));
#ifndef __bpo_no_relo__
	       if (fdb) {
		    int *p, j;
		    for (p = fot, j = bpoR[i].ni; j > 0 && !(*p); j--, p--);
		    if (j)	/* Found non-0 fot entry: can't use this rule */
			 continue;
	       }
#endif
	       if ((tnf || blengthmatch(lhl, lhp, bpoR[i].ni))
		   && (tp = brulematch(bpoR+i, ot, oh)) != 0L) {
		    ot = tp;	/* Everything matched! Now replace */
		    DEBUG(++nrepl); PDEBUG(("applying rule %d\n", i));
		    ih = brulerepl(bpoR+i, ih, il);
#ifndef __bpo_no_relo__
		    if (opb) {
			 int j, k, tmp[BSZ];
			 assert(bpoR[i].no <= BSZ);
			 pot = pot - bpoR[i].ni + 1;
				/* Compute permutation */
			 for (j = 0; j < bpoR[i].no; j++)
			      if ((k = bpoR[i].rd[j]) >= 0) 
				   tmp[j] = pot[k];
			      else tmp[j] = -1;
				/* Move on */
			 pih = pih - bpoR[i].no + 1;
			 for (j = 0; j < bpoR[i].no; j++)
			      pih[j] = tmp[j];
		    }
		    if (fdb) {	/* New code can be rewritten */
			 int j;
			 fot = fot - bpoR[i].ni + 1;
			 fih = fih - bpoR[i].no + 1;
			 for (j = 0; j < bpoR[i].no; j++)
			      fih[i] = 0;
		    }
#endif
		    goto nextpass;
	       }
	  }
	  INCR(ih); INCR(ot);
#ifndef __bpo_no_relo__
	  if (opb) { pot++; pih++; }
	  if (fdb) { fot++; fih++; }
#endif
     }
#ifndef __bpo_no_relo__
     if (opb) free(pil);
     if (fdb) free(fol);
#endif
     if (!tnf) free(lhl);
     PDEBUG(("BPO: made %d replacements in %d tries over %d passes\n", \
	     nrepl, ntry, npass));
     PDEBUG(("\tOutput ends at 0x%p\n", ot));
     return ot;
}

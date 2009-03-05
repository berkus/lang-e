/* $Id: bv.h,v 1.4 1998/05/08 04:00:01 maxp Exp $ */

#ifndef __BV_H__
#define __BV_H__

#include <assert.h>
#include <stdio.h>
#include "mem.h"

#if defined(__sparc__) && !defined(__LCC__) && !defined(__SVR4)
extern int printf (char *, ...);
#endif

/*
 * Bit vectors
 */

typedef unsigned long bvdt;
typedef bvdt * bvt;		/* Bit vector type */
#define bv_dx 1			/* Offset into b.v. where real data begins */

/* A bit vector looks like this:

   	---------------------------------
	| metadata	| data .... 	|
	---------------------------------
	^		^
	|- (bvt)p	|- (bvt)(p + bv_dx)

   Currently the metadata is just the number of bits that comprise the data,
   and bv_dx is "1".  We may want to also store (rather than recomputing on
   demand) the number of words that these bits occupy.  Then we just need to
   set bv_dx=2 and change the bit vector creation procedures.  Similarly for
   other changes.
*/
   
/* Bounds check */
#define cb(val, lim, action) do { if ((val) >= (lim)) { action; } } while (0)

/* bv_nwords: returns the number of longs necessary to store sz bits */
static inline unsigned int bv_nwords (unsigned int sz) {
     unsigned int nwords;		/* Words required to hold sz bits */
     unsigned int nbytes = (sz+7)>>3;	/* Bytes required to hold sz bits */
     switch (sizeof(bvdt)) {	/* Long words needed to hold sz bits */
     case 4: nwords = (nbytes+3)>>2; break;
     case 8: nwords = (nbytes+7)>>3; break;
     default: assert(0);
     }
     return nwords;
}

/* bv_new: create a new bitvector with l bits */
static inline bvt bv_new (unsigned int l) {
     bvt p;
     unsigned int wl = bv_nwords(l)+bv_dx;
     assert(l);
     NEW0(p, wl);		/* Allocate and clear */
     p[0] = l;			/* Store the length */
     return p;
}

/* bv_pnew: create a new bitvector with l bits in permanent arena */
static inline bvt bv_pnew (unsigned int l) {
     bvt p;
     unsigned int wl = bv_nwords(l)+bv_dx;
     assert(l);
     PNEW0(p, wl);		/* Allocate and clear */
     p[0] = l;			/* Store the length */
     return p;
}

/* bv_init: create a new bit vector of n*8*(sizeof bvdt) bits,
   initialized to the values in a */
static inline bvt bv_init (bvdt *a, unsigned int n) {
     unsigned int s;
     bvt p, q;
     NEW(p, n+bv_dx);		/* Allocate */
     p[0] = n*8*sizeof(bvdt);	/* Store the length */
				/* Initialize from a */
     for (q = p+bv_dx, s = 0; s < n; s++) q[s] = a[s];
     return p;
}

/* bv_pinit: create a new bit vector of n*8*(sizeof bvdt) bits,
   initialized to the values in a, in the permanent arena */
static inline bvt bv_pinit (bvdt *a, unsigned int n) {
     unsigned int s;
     bvt p, q;
     PNEW(p, n+bv_dx);		/* Allocate */
     p[0] = n*8*sizeof(bvdt);	/* Store the length */
				/* Initialize from a */
     for (q = p+bv_dx, s = 0; s < n; s++) q[s] = a[s];
     return p;
}

/* bv_reinit: initialize bv according to the data in a.
   Warning: the length of bv should be <= the length of a. */
static inline void bv_reinit (bvt bv, bvdt *a) {
     unsigned n = bv[0] / (8*sizeof(bvdt)), s;
     bvt t;
     for (t = bv+bv_dx, s = 0; s < n; s++) t[s] = a[s];
}

/* bv_clear: set all bits in bit vector v to 0 */
static inline void bv_clear (bvt v) {
     unsigned int s, l = bv_nwords(v[0])+bv_dx;
     for (s = bv_dx; s < l; s++) v[s] = 0;
}

/* bv_set: turn on bit i in bit vector bv. Bit vector bound is l. */
static inline void bv_set (bvt bv, unsigned int i) {
     unsigned int wordi;	/* Word index */
     unsigned int biti;		/* Bit index */
     cb(i, bv[0], return);
     switch (sizeof(bvdt)) {
     case 4: wordi = (i>>5)+bv_dx; biti = (i&0x1F); break;
     case 8: wordi = (i>>6)+bv_dx; biti = (i&0x3F); break;
     default: assert(0);
     }
     bv[wordi] |= (1<<biti);
}

/* bv_setf: turn on bit i in bit vector bv if it was not already set. 
   Return 1 if we changed bit i, 0 otherwise. */
static inline unsigned int bv_setf (bvt bv, unsigned int i) {
     unsigned int wordi;	/* Word index */
     unsigned int biti;		/* Bit index */
     cb(i, bv[0], return 0);
     switch (sizeof(bvdt)) {
     case 4: wordi = (i>>5)+bv_dx; biti = (i&0x1F); break;
     case 8: wordi = (i>>6)+bv_dx; biti = (i&0x3F); break;
     default: assert(0);
     }
     if (bv[wordi] & (1<<biti))
	  return 0;
     bv[wordi] |= (1<<biti);
     return 1;
}

/* bv_cset: conditional set: turn on bit i in bit vector v1 iff it is not
   already on in bit vector v2. */
static inline void bv_cset (bvt v1, bvt v2, unsigned int i) {
     unsigned int wordi;	/* Word index */
     unsigned int biti;		/* Bit index */
     cb(i, v1[0], return);
     cb(i, v2[0], return);
     switch (sizeof(bvdt)) {
     case 4: wordi = (i>>5)+bv_dx; biti = (i&0x1F); break;
     case 8: wordi = (i>>6)+bv_dx; biti = (i&0x3F); break;
     default: assert(0);
     }
     if (! (v2[wordi] & (1<<biti)))
	  v1[wordi] |= (1<<biti);
}

/* bv_setp: returns non-0 if bit i in bit vector bv is set, 0 otherwise. */
static inline unsigned int bv_setp (bvt bv, unsigned int i) {
     unsigned int wordi;	/* Word index */
     unsigned int biti;		/* Bit index */
     cb(i, bv[0], return 0);
     switch (sizeof(bvdt)) {
     case 4: wordi = (i>>5)+bv_dx; biti = (i&0x1F); break;
     case 8: wordi = (i>>6)+bv_dx; biti = (i&0x3F); break;
     default: assert(0);
     }
     return (unsigned int)(bv[wordi] & (1<<biti));
}

/* bv_unset: turn off bit i in bit vector bv. */
static inline void bv_unset (bvt bv, unsigned int i) {
     unsigned int wordi;	/* Word index */
     unsigned int biti;		/* Bit index */
     cb(i, bv[0], return);
     switch (sizeof(bvdt)) {
     case 4: wordi = (i>>5)+bv_dx; biti = (i&0x1F); break;
     case 8: wordi = (i>>6)+bv_dx; biti = (i&0x3F); break;
     default: assert(0);
     }
     bv[wordi] &= ~(1<<biti);
}

/* bv_unsetf: turn off bit i in bit vector bv if it was not already off;
   return 1 if we changed bit i, 0 otherwise. */
static inline unsigned int bv_unsetf (bvt bv, unsigned int i) {
     unsigned int wordi;	/* Word index */
     unsigned int biti;		/* Bit index */
     cb(i, bv[0], return 0);
     switch (sizeof(bvdt)) {
     case 4: wordi = (i>>5)+bv_dx; biti = (i&0x1F); break;
     case 8: wordi = (i>>6)+bv_dx; biti = (i&0x3F); break;
     default: assert(0);
     }
     if ((bv[wordi] & (1<<biti)) == 0)
	  return 0;
     bv[wordi] &= ~(1<<biti);
     return 1;
}

/* bv_cp: return a copy of bit vector v. */
static inline bvt bv_cp (bvt v) {
     unsigned int s, l = bv_nwords(v[0])+bv_dx;
     bvt v2;
     NEW(v2, l);
     for (s = 0; s < l; s++) v2[s] = v[s];
     return v2;
}

/* bv_pcp: return a copy of bit vector v from the permanent arena. */
static inline bvt bv_pcp (bvt v) {
     unsigned int s, l = bv_nwords(v[0])+bv_dx;
     bvt v2;
     PNEW(v2, l);
     for (s = 0; s < l; s++) v2[s] = v[s];
     return v2;
}

/* bv_cpto: copy from from to to.  Both must have the same length. */
static inline void bv_cpto (bvt to, bvt from) {
     unsigned int s, l = bv_nwords(to[0])+bv_dx;

     assert(to[0] == from[0]);
     for (s = bv_dx; s < l; s++) to[s] = from[s];
}

/* bv_eq: return truth value v1 == v2. */
static inline unsigned int bv_eq (bvt v1, bvt v2) {
     unsigned int s, l = bv_nwords(v1[0])+bv_dx;

     for (s = 0; s < l; s++)
	  if (v1[s] != v2[s]) return 0;
     return 1;
}

/* bv_subset: return 1 if v1 is a subset of v2, 0 otherwise. */
static inline unsigned int bv_subset (bvt v1, bvt v2) {
     unsigned int s, l = bv_nwords(v1[0])+bv_dx;

     if (v1[0] > v2[0])
	  return 0;
				/* a \subset b  <==> (a&~b) == 0 */
     for (s = bv_dx; s < l; s++)
	  if (v1[s] & ~v2[s]) return 0;
     return 1;
}

/* bv_union: v1 = v1 \cup v2 */
static inline void bv_union (bvt v1, bvt v2) {
     unsigned int s, l = bv_nwords(v1[0])+bv_dx;
     assert(v1[0] == v2[0]);

     for (s = bv_dx; s < l; s++) v1[s] |= v2[s];
}

/* bv_union2: return v1 = v2 \cup v3 */
static inline void bv_union2 (bvt v1, bvt v2, bvt v3) {
     unsigned int s, l = bv_nwords(v1[0])+bv_dx;
     assert(v1[0] == v2[0] && v2[0] == v3[0]);

     for (s = bv_dx; s < l; s++) v1[s] = v2[s] | v3[s];
}

/* bv_union2s: return v1 = v1 \cup v2 \cup v3 */
static inline void bv_union2s (bvt v1, bvt v2, bvt v3) {
     unsigned int s, l = bv_nwords(v1[0])+bv_dx;
     assert(v1[0] == v2[0] && v2[0] == v3[0]);

     for (s = bv_dx; s < l; s++) v1[s] |= (v2[s] | v3[s]);
}

/* bv_runion: returns v1 \cup v2 */
static inline bvt bv_runion (bvt v1, bvt v2) {
     unsigned int s, l = bv_nwords(v1[0])+bv_dx;
     bvt v0;
     assert(v1[0] == v2[0]);

     NEW(v0, l);
     v0[0] = v1[0];
     for (s = bv_dx; s < l; s++) v0[s] = v1[s] | v2[s];
     return v0;
}

/* bv_prunion: returns v1 \cup v2 from the permanent arena */
static inline bvt bv_prunion (bvt v1, bvt v2) {
     unsigned int s, l = bv_nwords(v1[0])+bv_dx;
     bvt v0;
     assert(v1[0] == v2[0]);

     PNEW(v0, l);
     v0[0] = v1[0];
     for (s = bv_dx; s < l; s++) v0[s] = v1[s] | v2[s];
     return v0;
}

/* bv_inter: return v1 = v1 \cap v2 */
static inline void bv_inter (bvt v1, bvt v2) {
     unsigned int s, l = bv_nwords(v1[0])+bv_dx;
     assert(v1[0] == v2[0]);

     for (s = bv_dx; s < l; s++) v1[s] &= v2[s];
}

/* bv_inter2: return v1 = v2 \cap v3 */
static inline void bv_inter2 (bvt v1, bvt v2, bvt v3) {
     unsigned int s, l = bv_nwords(v1[0])+bv_dx;
     assert(v1[0] == v2[0] && v2[0] == v3[0]);

     for (s = bv_dx; s < l; s++) v1[s] = v2[s] & v3[s];
}

/* bv_rinter: returns v1 \cap v2 */
static inline bvt bv_rinter (bvt v1, bvt v2) {
     unsigned int s, l = bv_nwords(v1[0])+bv_dx;
     bvt v0;
     assert(v1[0] == v2[0]);

     NEW(v0, l);
     v0[0] = v1[0];
     for (s = bv_dx; s < l; s++) v0[s] = v1[s] & v2[s];
     return v0;
}

/* bv_xor: v1 = v1 xor v2 */
static inline void bv_xor (bvt v1, bvt v2) {
     unsigned int s, l = bv_nwords(v1[0])+bv_dx;
     assert(v1[0] == v2[0]);

     for (s = bv_dx; s < l; s++) v1[s] ^= v2[s];
}

/* bv_diff: v1 = v1 - v2 */
static inline void bv_diff (bvt v1, bvt v2) {
     unsigned int s, l = bv_nwords(v1[0])+bv_dx;
     assert(v1[0] == v2[0]);

     for (s = bv_dx; s < l; s++) v1[s] &= (~v2[s]);
}

/* bv_diff2: v1 = v2 - v3 */
static inline void bv_diff2 (bvt v1, bvt v2, bvt v3) {
     unsigned int s, l = bv_nwords(v1[0])+bv_dx;
     assert(v1[0] == v2[0] && v2[0] == v3[0]);

     for (s = bv_dx; s < l; s++) v1[s] = v2[s] & (~v3[s]);
}

/* bv_rdiff: return v1 - v2 */
static inline bvt bv_rdiff (bvt v1, bvt v2) {
     unsigned int s, l = bv_nwords(v1[0])+bv_dx;
     bvt v0;
     assert(v1[0] == v2[0]);

     NEW(v0, l);
     v0[0] = v1[0];
     for (s = bv_dx; s < l; s++) v0[s] = v1[s] & ~v2[s];
     return v0;
}

#if defined(__sparc__) || defined(__mips__) || defined(__i386__) || defined(__sslittle__) || defined(__ssbig__)
				/* This code depents on machine's word size */

/* bv_new1: create a new bitvector with l bits, with all bits set to 1 */
static inline bvt bv_new1 (unsigned int l) {
     bvt p;
     unsigned int wl = bv_nwords(l)+bv_dx;
     assert(l);
     NEW1(p, wl);		/* Allocate and set */
     p[0] = l;			/* Store the length */
     if (l&0x1f)		/* Clobber excess 1s */
	  p[wl-1] &= ( (1 << (l&0x1f) ) - 1);
     return p;
}

/* bv_fill: set all bits in bit vector v to 1 */
static inline void bv_fill (bvt v) {
     unsigned int s, bl = v[0], l = bv_nwords(bl)+bv_dx;
     assert(sizeof(bvdt) == 4);
     for (s = bv_dx; s < l; s++) v[s] = 0xffffffff;
     if (bl&0x1f)		/* Clobber excess 1s */
	  v[l-1] &= ( (1 << (bl&0x1f) ) - 1);
}

/* bv_not: v1 = ~v1 */
static inline void bv_not (bvt v) {
     unsigned int s, bl = v[0], l = bv_nwords(bl)+bv_dx;

     for (s = bv_dx; s < l; s++) v[s] = ~v[s];
     if (bl&0x1f)		/* Clobber excess 1s */
	  v[l-1] &= ( (1 << (bl&0x1f) ) - 1);
}

/* bv_eachbit: applies f to each set bit of v */
static inline void bv_eachbit (bvt v, void (*f)(unsigned int, bvt, void *), 
			       void *x) {
     unsigned int i, i0, j, k, lb = v[0], l = bv_nwords(lb);
     bvdt mask;
     bvt vdx = v+bv_dx;
     assert(sizeof(bvdt) == 4);
     for (i = 0; i < l; i++) {
	  i0 = i<<5;		/* Figure out bit offset */
	  for (j = 0; j < 31; j += 4) {	
	       k = j+i0;	/* Loop through current word */
	       if (k >= lb) return;
	       mask = 0xF<<j;
				/* Match on nibbles */
	       switch ((vdx[i]&mask)>>j) {
	       case 0x0: continue;
	       case 0x1: f(0+k,v,x);
		    continue;
	       case 0x2:             f(1+k,v,x);		
		    continue;
	       case 0x3: f(0+k,v,x); f(1+k,v,x);		
		    continue;
	       case 0x4:                         f(2+k,v,x);
		    continue;
	       case 0x5: f(0+k,v,x);             f(2+k,v,x);
		    continue;
	       case 0x6:             f(1+k,v,x); f(2+k,v,x);
		    continue;
	       case 0x7: f(0+k,v,x); f(1+k,v,x); f(2+k,v,x);
		    continue;
	       case 0x8:                                     f(3+k,v,x);
		    continue;
	       case 0x9: f(0+k,v,x);                         f(3+k,v,x);
		    continue;
	       case 0xA:             f(1+k,v,x);             f(3+k,v,x);
		    continue;
	       case 0xB: f(0+k,v,x); f(1+k,v,x);             f(3+k,v,x);
		    continue;
	       case 0xC:                         f(2+k,v,x); f(3+k,v,x);
		    continue;
	       case 0xD: f(0+k,v,x);             f(2+k,v,x); f(3+k,v,x);
		    continue;
	       case 0xE:             f(1+k,v,x); f(2+k,v,x); f(3+k,v,x);
		    continue;
	       case 0xF: f(0+k,v,x); f(1+k,v,x); f(2+k,v,x); f(3+k,v,x); 
		    continue;
	       default: assert(0);
	       }
	  }
     }
}

static char bv_ffb_table[257];
static inline void bv_ffb_init (void) {
     int i;
     for (i = 0; i < 8; i++) 
	  bv_ffb_table[1<<i] = i;
}

/* bv_firstbit: return the first non-zero bit in v, or -1 if all bits are 0 */
static inline int bv_firstbit (bvt v) {
#define byte(x, b) (((x) >> ((b)*8)) & 0xff)
     unsigned int i, i0, l = bv_nwords(v[0]);
     bvdt mask;
     static int ffb_init = 0;
     if (!ffb_init) {
	  bv_ffb_init();
	  ffb_init = 1;
     }
     assert(sizeof(bvdt) == 4);
     v += bv_dx;
     for (i = 0; i < l; i++) {
	  mask = v[i];
	  if (mask == 0) continue;
	  i0 = i<<5;		/* Figure out bit offset */
	  mask &= -mask;
	  if (mask >= (1<<16)) {
	       if (mask >= (1<<24))
		    return bv_ffb_table[byte(mask, 3)] + 24 + i0;
	       else
		    return bv_ffb_table[byte(mask, 2)] + 16 + i0;
	  } else {
	       if (mask >= (1<<8))
		    return bv_ffb_table[byte(mask, 1)] + 8 + i0;
	       else
		    return bv_ffb_table[byte(mask, 0)] + i0;
	  }
     }
     return -1;
}

/* bv_num: return the number of non-zero bits in v */
static inline unsigned int bv_num (bvt v) {
     unsigned int i, i0, j, k = 0, lb = v[0], l = bv_nwords(lb);
     bvdt mask;
     assert(sizeof(bvdt) == 4);
     v += bv_dx;
     for (i = 0; i < l; i++) {
	  if (v[i] == 0)
	       continue;
	  i0 = i<<5;
	  for (j = 0; j < 31; j += 4) {	
				/* Loop through current word */
	       if (j+i0 >= lb) return k;
	       mask = 0xF<<j;	
	       switch ((v[i]&mask)>>j) {
	       case 0x0: continue;
	       case 0x1: case 0x2: case 0x4: case 0x8:
		    k += 1; continue;
	       case 0x3: case 0x5: case 0x6: case 0x9: case 0xA: case 0xC:
		    k += 2; continue;
	       case 0x7: case 0xB: case 0xD: case 0xE:
		    k += 3; continue;
	       case 0xF:
		    k += 4; continue;
	       default: assert(0);
	       }
	  }
     }
     return k;
}

/* bv_unparse: unparses v */
static inline void bv_unparse (bvt v) {
     unsigned int i, i0, j, k;
     unsigned int lb = v[0], l = bv_nwords(lb);
     bvdt mask;
     char *s;
     assert(sizeof(bvdt) == 4);
     v += bv_dx;
     NEWX(s, (l<<5)+1, '0');	/* l<<5+1: max 32 bits/word, +1 for '\0' */
     for (i = 0; i < l; i++) {
	  i0 = i<<5;		/* Figure out bit offset */
	  for (j = 0; j < 31; j += 4) {	
	       k = j+i0;	/* Loop through current word */
	       if (k >= lb) goto done;
	       mask = 0xF<<j;
	       switch ((v[i]&mask)>>j) {
	       case 0x0: 
		    continue;
	       case 0x1: s[0+k]='1'; 				  	
		    continue;
	       case 0x2:             s[1+k]='1';		
		    continue;
	       case 0x3: s[0+k]='1'; s[1+k]='1';		
		    continue;
	       case 0x4:                         s[2+k]='1';
		    continue;
	       case 0x5: s[0+k]='1';             s[2+k]='1';
		    continue;
	       case 0x6:             s[1+k]='1'; s[2+k]='1';
		    continue;
	       case 0x7: s[0+k]='1'; s[1+k]='1'; s[2+k]='1';
		    continue;
	       case 0x8:                                     s[3+k]='1';
		    continue;
	       case 0x9: s[0+k]='1';                         s[3+k]='1';
		    continue;
	       case 0xA:             s[1+k]='1';             s[3+k]='1';
		    continue;
	       case 0xB: s[0+k]='1'; s[1+k]='1';             s[3+k]='1';
		    continue;
	       case 0xC:                         s[2+k]='1'; s[3+k]='1';
		    continue;
	       case 0xD: s[0+k]='1';             s[2+k]='1'; s[3+k]='1';
		    continue;
	       case 0xE:             s[1+k]='1'; s[2+k]='1'; s[3+k]='1';
		    continue;
	       case 0xF: s[0+k]='1'; s[1+k]='1'; s[2+k]='1'; s[3+k]='1'; 
		    continue;
	       default: assert(0);
	       }
	  }
     }
done:
     s[lb] = (char)0;
     printf("%d[%s]\n", lb, s);
}

#else
#error "bv.h: machine-specific bit-vector ops unimplemented on this platform"
#endif

#endif

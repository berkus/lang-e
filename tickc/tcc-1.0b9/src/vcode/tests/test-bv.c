/* $Id: test-bv.c,v 1.1 1998/01/19 20:30:40 maxp Exp $ */

/* Tester for bit vector module:

   % gcc -Wall -I.. -o test-bv test-bv.c ../mem.c -Wno-unused
   % test-bv > out
   % diff out test-bv.ref
*/

#include <assert.h>
#include <stdio.h>
#include "bv.h"

#if defined(__sparc__) && !defined(__LCC__)
extern int printf (const char *, ...);
#endif

void testeachbit(unsigned int i, bvt v, void *x) {
     printf("\tDoing bit %d\n", i);
     if ((i < 13) || (i % 3 == 0)) {
	  bv_unset(v, i); bv_unparse(v);
     }
}

void main (int argc, char **argv) {
     int i;
     bvt p, q;

     unsigned long x[32] = { 0x32 };
     for (i = 0; i < 32; i++) x[i] = i+1;

     printf("Testing bv_nwords:\n");
     for (i = 0; i < 256; i += 8) {
	  printf("\tbv_nwords(%d) = %d\n", i-1, bv_nwords(i-1));
	  printf("\tbv_nwords(%d) = %d\n", i, bv_nwords(i));
	  printf("\tbv_nwords(%d) = %d\n", i+1, bv_nwords(i+1));
     }

     printf("Testing bv_new and bv_new1:\n");
     for (i = 1; i < 256; i += 8) {
	  p = bv_new(i); bv_unparse(p);
	  p = bv_new1(i); bv_unparse(p);
     }

     printf("Testing bv_init:\n");
     for (i = 1; i < 32; i++) {
	  p = bv_init(x, i); bv_unparse(p);
     }

     printf("Testing bv_clear:\n");
     bv_clear(p); bv_unparse(p);

     printf("Testing bv_set:\n");
     p = bv_new(37);
     for (i = 0; i < 20; i++) {
	  bv_set(p, i); bv_unparse(p);
     }

     printf("Testing bv_setf:\n");
     for (i = 15; i < 25; i++) {
	  if (bv_setf(p, i)) bv_unparse(p);
	  else printf("\tBit %d already set\n", i);
     }

     q = bv_new(37);
     printf("Testing bv_cset:\n");
     for (i = 0; i < 38; i++) bv_cset(q, p, i);
     bv_unparse(p); bv_unparse(q);

     printf("Testing bv_unsetf:\n");
     for (i = 20; i < 30; i++) {
	  if (bv_unsetf(p, i)) bv_unparse(p);
	  else printf("\tBit %d already unset\n", i);
     }

     printf("Testing bv_setp:\n");
     bv_unparse(p);
     for (i = 15; i < 20; i++)
	  if (bv_setp(p, i)) printf("\tBit %d set\n", i);

     printf("Testing bv_unset:\n");
     for (i = 0; i<40; i++) bv_unset(q, i);
     bv_unparse(q);

     printf("Testing bv_cp:\n");
     q = bv_cp(p);
     bv_unparse(p); bv_unparse(q);

     printf("Testing bv_cpto:\n");
     bv_set(q,30);
     bv_cpto(p, q);
     bv_unparse(p); bv_unparse(q);

     printf("Testing bv_eq:\n");
     if (bv_eq(p,q)) printf("\tp and q are equal\n");
     bv_unset(q,30);
     bv_unparse(p); bv_unparse(q);
     if (!bv_eq(p,q)) printf("\tp and q are no longer equal\n");

     printf("Testing bv_subset:\n");
     if (bv_subset(q,p)) printf("\t q is a subset of p\n");
     if (bv_subset(p,q)) printf("\t p is a subset of q: bad!\n");


     printf("Testing bv_rdiff:\n");
     q = bv_rdiff(p, q);
     bv_unparse(q);
     q = bv_rdiff(q, p);
     bv_unparse(q);

     printf("Testing bv_fill:\n");
     bv_fill(p);
     bv_unparse(p);

     printf("Testing bv_firstbit: %d\n", bv_firstbit(p));
     printf("Testing bv_num: %d\n", bv_num(p));

     printf("Testing bv_eachbit and a bunch of other stuff:\n");
     bv_eachbit(p, testeachbit, NULL);
     bv_unparse(p);

     printf("Testing bv_firstbit: %d\n", bv_firstbit(p));
     printf("Testing bv_num: %d\n", bv_num(p));

     printf("Testing bv_not:\n");
     bv_not(p);
     bv_unparse(p);
     printf("Testing bv_firstbit: %d\n", bv_firstbit(p));
     printf("Testing bv_num: %d\n", bv_num(p));

     bv_clear(p);
     bv_unparse(p);
     printf("Testing bv_firstbit: %d\n", bv_firstbit(p));
     printf("Testing bv_num: %d\n", bv_num(p));

     printf("Testing bv_subset again:\n");
     p = bv_new(37);
     q = bv_new1(37);
     for (i = 0; i < 37; i++)
	  bv_set(p, i);
     bv_unset(q, 15);
     bv_unparse(p);
     bv_unparse(q);
     if (bv_subset(q,p)) printf("\t q is a subset of p\n");
     if (bv_subset(p,q)) printf("\t p is a subset of q\n");
}

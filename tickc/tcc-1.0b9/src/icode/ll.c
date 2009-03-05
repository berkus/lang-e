/* $Id: ll.c,v 1.3 1998/05/10 03:38:47 maxp Exp $ */

#include "ll.h"

/* i_lleunparse: print out info about linked list element e */
void i_lleunparse (i_ll_t l, i_lle_t e, void *v) {
     if (e == NULL) { 
	  printf("(NULL)\n");
	  return;
     }
     if (e->lr == NULL) {
	  printf("(EMPTY)\n");
     }
     printf("\t\tr%d: [%d, %d]\n", e->lr->n, e->lr->beg, e->lr->end);
}

/* i_llunparse: print out info about linked list l */
void i_llunparse (i_ll_t l) {
     if (l == NULL) { 
	  printf("(NULL)\n");
	  return;
     }
     printf("Linked list\n\tSize: %d\tNum used: %d\tNum free: %d\n",
	    l->sz, l->nu, l->nf);
     printf("\tIsFull: %d\n", i_llisfull(l));
     printf("\tFirst: "); i_lleunparse(l, i_llfirst(l), NULL);
     printf("\tLast: "); i_lleunparse(l, i_lllast(l), NULL);
     i_lleachfwd(l, i_lleunparse, NULL);
}

/* i_lleachfwd: apply f to each element in l, passing v as the third argument.
   Scan forward through l. */
void i_lleachfwd (i_ll_t l, void (*f)(i_ll_t, i_lle_t, void *), void *v) {
     i_lle_t le, next, lim;
     le = l->used;
     if (le == NULL)
	  return;
     lim = le->p;
     while (le != lim) {
	  next = le->n;		/* Find next first, since f may detach le */
	  (*f)(l, le, v);	/* Apply f */
	  le = next;		/* Move on */
     }
     (*f)(l, le, v);
}

/* i_lleachbwd: apply f to each element in l, passing v as the third argument.
   Scan backward through l. */
void i_lleachbwd (i_ll_t l, void (*f)(i_ll_t, i_lle_t, void *), void *v) {
     i_lle_t le, prev, lim;
     lim = l->used;
     if (lim == NULL)
	  return;
     le = lim->p;
     while (le != lim) {
	  prev = le->p;		/* Find prev first, since f may detach le */
	  (*f)(l, le, v);	/* Apply f */
	  le = prev;		/* Move on */
     }
     (*f)(l, le, v);
}

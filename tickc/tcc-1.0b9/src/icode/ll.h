/* $Id: ll.h,v 1.2 1998/05/06 19:03:31 maxp Exp $ */

#ifndef __I_LL_H__
#define __I_LL_H__

/* Linked list */

#include "icode.h"
#include "icode-internal.h"

typedef struct lle_obj {	/* Linked list element type */
     i_lr_t lr;			/* Live range */
     struct lle_obj *p;		/* Previous */
     struct lle_obj *n;		/* Next */
} * i_lle_t;

typedef struct {		/* Linked list type */
     unsigned int sz;		/* Size (max number of elements) */
     i_lle_t used;		/* Used (attached) elements */
     i_lle_t free;		/* Free (detached) elements */
     unsigned int nu;		/* Number used */
     unsigned int nf;		/* Number free */
} * i_ll_t;

static inline i_ll_t i_llcreate (unsigned int l);
extern void i_lleunparse (i_ll_t l, i_lle_t e, void *v);
extern void i_llunparse (i_ll_t l);
static inline unsigned int i_llisfull (i_ll_t l);
static i_lle_t i_llsfwd (i_ll_t l, 
			unsigned int (*f)(i_ll_t, i_lle_t, void *, void *),
			void *v1, void *v2);
static i_lle_t i_llsbwd (i_ll_t l, 
			unsigned int (*f)(i_ll_t, i_lle_t, void *, void *), 
			void *v1, void *v2);
static inline i_lle_t i_llgetfree (i_ll_t l);
static inline i_lle_t i_llifwd (i_ll_t l, i_lle_t e);
static inline i_lle_t i_llibwd (i_ll_t l, i_lle_t e);
static inline void i_llrem (i_ll_t l, i_lle_t e);
static inline void i_llset (i_lle_t e, i_lr_t c);
extern void i_lleachfwd (i_ll_t l, void (*f)(i_ll_t, i_lle_t, void *), void *v);
extern void i_lleachbwd (i_ll_t l, void (*f)(i_ll_t, i_lle_t, void *), void *v);
static inline i_lle_t i_llfirst (i_ll_t l);
static inline i_lle_t i_lllast (i_ll_t  l);


/* i_llcreate: create and return a new linked list of size l */
static inline i_ll_t i_llcreate (unsigned int l) {
     i_ll_t ll;
     i_lle_t le, lim;
     assert(l>0);
     NEW(ll, 1);		/* Allocate & initialize linked list object */
     ll->sz = ll->nf = l;
     ll->nu = 0;
     ll->used = NULL;
     NEW(ll->free, l);
     le = ll->free;
				/* Init the circular-linked free list */
     le->p = le+l-1; le->n = le+1;
     for (lim = &ll->free[l-1], le = &ll->free[1]; le < lim; le++) {
	  le->p = le-1; le->n = le+1;
     }

     le->p = le-1; le->n = ll->free;

     return ll;
}

/* i_llisfull: 1 if linked list is full (number of attached elements equals
   maximum number of elements), 0 otherwise */
static inline unsigned int i_llisfull (i_ll_t l) {
     assert(l && l->nf + l->nu == l->sz);
     return (l->nf == 0);
}

/* i_llsfwd: search fwd from the start of l until f returns 1.  Returns the
   corresponding linked list element, or NULL if f never returns 1. */
static i_lle_t i_llsfwd (i_ll_t l, 
			unsigned int (*f)(i_ll_t, i_lle_t, void *, void *),
			void *v1, void *v2) {
     i_lle_t le, lim, next;

     assert(l);

     le = l->used;
     if (!le)
	  return NULL;
     lim = le->p;

     while (le != lim) {
	  next = le->n;
	  if ((*f)(l, le, v1, v2))
	       return le;
	  le = next;
     }
     if ((*f)(l, le, v1, v2))
	  return le;
     return NULL;
}

/* i_llsbwd: search bwd from the end of l until f returns 1.  Returns the
   corresponding linked list element, or NULL if f never returns 1. */
static i_lle_t i_llsbwd (i_ll_t l, 
			unsigned int (*f)(i_ll_t, i_lle_t, void *, void *), 
			void *v1, void *v2) {
     i_lle_t le, lim, prev;

     assert(l);

     lim = l->used;
     if (!lim)
	  return NULL;
     le = lim->p;

     while (le != lim) {
	  prev = le->p;
	  if ((*f)(l, le, v1, v2))
	       return le;
	  le = prev;
     }
     if ((*f)(l, le, v1, v2))
	  return le;
     return NULL;
}

/* i_llgetfree: return a new free element for list l.
   Requires that there be free elements available. */
static inline i_lle_t i_llgetfree (i_ll_t l) {
     i_lle_t le;

     le = l->free;		/* Obtain a new free list element */
     assert(le);
     le->p->n = le->n;
     le->n->p = le->p;
				/* If removing last element, zero free list */
     l->free = le->n == le ? NULL : le->n;
     assert(l->free || l->nf == 1);
     return le;
}

/* i_llifwd: add a new elements "forwards", into l after e.
   If e is null, create new element at end of list. */
static inline i_lle_t i_llifwd (i_ll_t l, i_lle_t e) {
     i_lle_t le, u;
     assert(l);

     if (l->nf == 0) {
	  printf("icode: out of free list elements. Exiting.\n");
	  exit(1);
     }

     le = i_llgetfree(l);
     				/* Set pt of insertion: e, or head of list */
     if (l->used) {
	  u = e ? e : l->used->p;
	  u->n->p = le;
	  le->n = u->n;
	  le->p = u;
	  u->n = le;
     } else {
	  le->p = le->n = le;
	  l->used = le;
     }

     -- l->nf; ++ l->nu;	/* Set counters */
     return le;
}

/* i_llibwd: add a new element "backwards", into l before e.
   If e is null, create new element at head of list. */
static inline i_lle_t i_llibwd (i_ll_t l, i_lle_t e) {
     i_lle_t le, u;
     assert(l);

     if (l->nf == 0) {
	  printf("icode: out of free list elements. Exiting.\n");
	  exit(1);
     }
     
     le = i_llgetfree(l);

     if (l->used) {
	  u = e ? e : l->used;
	  u->p->n = le;
	  le->p = u->p;
	  le->n = u;
	  u->p = le;
	  if (u == l->used)	/* Reset root if necessary */
	       l->used = le;
     } else {
	  le->p = le->n = le;
	  l->used = le;
     }

     -- l->nf; ++ l->nu;	/* Set counters */
     return le;
}

/* i_llrem: remove e from active elements in l, returning it to free list */
static inline void i_llrem (i_ll_t l, i_lle_t e) {
     i_lle_t u;
     assert(l);

     if (l->nu == 0 || e == NULL) {
	  printf("icode: trying to detach unattached list element. "
		 "Exiting.\n");
	  exit(1);
     }

     e->p->n = e->n;		/* Remove e from used list */
     e->n->p = e->p;
     if (e == e->n) {
	  assert(e == l->used && e == e->p);
	  l->used = NULL;
     } else if (e == l->used) {
	  l->used = e->n;
     }
     assert(l->used || l->nu == 1);

     u = l->free;
     if (u) {			/* Succ and pred */
	  u->n->p = e;
	  e->n = u->n;
	  e->p = u;
	  u->n = e;
     } else {
	  e->p = e->n = e;
     }
     if (l->free == NULL)
	  l->free = e;		/* Set root, if necessary */

     ++ l->nf; -- l->nu;
}


/* i_llset: set contents of e to c */
static inline void i_llset (i_lle_t e, i_lr_t c) {
     e->lr = c;
}

/* i_llfirst: return first element in l. */
static inline i_lle_t i_llfirst (i_ll_t l) {
     return l->used;
}

/* i_lllast: return last element in l. */
static inline i_lle_t i_lllast (i_ll_t  l) {
     if (l->used)
	  return l->used->p;
     return NULL;
}

#endif

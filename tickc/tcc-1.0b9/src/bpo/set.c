/* $Id: set.c,v 1.1.1.1 1997/12/05 01:25:43 maxp Exp $ */

#include <assert.h>
#include "set.h"

#define HASHSIZE NELEMS(((Set)0)->buckets)

Set set (int arena) {
     Set new;
     NEW0(new, 1, arena);
     new->arena = arena;
     return new;
}

void set_add (Set s, Setelt e) {
     if (! set_find(s, e)) {
	  struct setentry *p;
	  unsigned h = (unsigned)e&(HASHSIZE-1);
				/* Allocate */
	  NEW0(p, 1, s->arena);
	  p->link = s->buckets[h];
	  s->buckets[h] = p;

	  p->elt.val = e;
	  if (s->all == 0L) 
	       s->all = &p->elt;
	  else {
	       p->elt.next = s->all;
	       s->all = &p->elt;
	  }
	  ++s->card;
     }
}

int set_find (Set s, Setelt e) {
     struct setentry *p;
     unsigned h = (unsigned)e&(HASHSIZE-1);

     assert(s);
     for (p = s->buckets[h]; p; p = p->link)
	  if (e == p->elt.val)
	       return 1;
     return 0;
}

int set_card (Set s) {
     assert(s);
     return s->card;
}

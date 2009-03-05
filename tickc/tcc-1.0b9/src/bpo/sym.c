/* $Id: sym.c,v 1.1.1.1 1997/12/05 01:25:43 maxp Exp $ */

#include <assert.h>
#include "mem.h"
#include "sym.h"

#define HASHSIZE NELEMS(((Table)0)->buckets)

Table table (int arena) {
     Table new;

     NEW0(new, 1, arena);
     new->arena = arena;
     return new;
}

void foreach (Table tp, void (*f)(Symbol, void*, void*), 
	      void *a1, void *a2) { 
     Symbol p;

     assert(tp);
     for (p = tp->all; p; p = p->next) { (*f)(p, a1, a2); }
}

Symbol install (char *name, Table tp) {
     struct tableentry *p;
     unsigned h = (unsigned)name&(HASHSIZE-1);
				/* Allocate */
     NEW0(p, 1, tp->arena);
     p->link = tp->buckets[h];
     tp->buckets[h] = p;
				/* Initialize */
     assert(name);
     p->sym.name = name;
     if (tp->all == 0L) {
	  tp->all = &p->sym;
     } else {
	  p->sym.next = tp->all;
	  tp->all = &p->sym;
     }
     return &p->sym;
}

Symbol lookup (char *name, Table tp) {
     struct tableentry *p;
     unsigned h = (unsigned)name&(HASHSIZE-1);

     assert(tp);
     for (p = tp->buckets[h]; p; p = p->link)
	  if (name == p->sym.name)
	       return &p->sym;
     return 0L;
}

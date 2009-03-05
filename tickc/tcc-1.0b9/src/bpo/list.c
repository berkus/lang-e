/* $Id: list.c,v 1.1.1.1 1997/12/05 01:25:42 maxp Exp $ */

/* This code is mostly from lcc, by Chris Fraser and David Hanson */

#include <assert.h>
#include "list.h"

/* l_prepend: prepend x to list, return new list */
List l_prepend (void *x, List list, int arena) {
     List new;

     NEW0(new, 1, arena);
     if (list) {
	  new->link = list->link;
	  list->link = new;
     } else {
	  new->link = new;
	  list = new;
     }
     new->x = x;
     return list;
}

/* l_append: append x to list, return new list */
List l_append (void *x, List list, int arena) {
     List new;

     NEW0(new, 1, arena);
     if (list) {
	  new->link = list->link;
	  list->link = new;
     } else
	  new->link = new;
     new->x = x;
     return new;
}

/* l_remove: remove x from list; if x is contained by the list head (*list), 
   modify *list appropriately; if *list is a one element list containing x,
   sets *list = 0 (destroys the list). */
void l_remove (void *x, List *list) {
     List lp, ll;

     if (!list) return;
     lp = *list, ll;
     do {
	  ll = lp->link;
	  assert(ll);
	  if (ll->x == x) {
	       if (ll == *list) {
		    if (lp == *list) { *list = 0L; return; }
		    else { *list = lp; }
	       }
	       lp->link = ll->link;
	  }
     } while ((lp = ll) != *list);
}

/* l_length: # elements in list */
int l_length (List list) {
     int n = 0;

     if (list) {
	  List lp = list;
	  do
	       n++;
	  while ((lp = lp->link) != list);
     }
     return n;
}

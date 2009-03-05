/* $Id: util.h,v 1.2 1998/05/08 21:48:43 maxp Exp $ */

#ifndef __UTIL_H__
#define __UTIL_H__

#include <mem.h>

/* growlist */
#define growlist(elty, list, cur, lim, inc) do {			\
     assert(cur <= lim);						\
     if (cur == lim) {							\
	  elty *nl;		/* New list */				\
	  elty *ol = list;	/* Old list */				\
	  elty *l = ol+cur;	/* Limit of old list */			\
	  lim += inc;		/* Increase limit of old list */	\
	  NEW(nl, lim);		/* Get new list */			\
				/* Copy old to new */			\
	  for (list=nl; ol < l; *nl++ = *ol++);				\
     }									\
} while (0)

/* addtolist: extend homogeneous list (with elements of type elty,
   current length cur and limit length lim) with new element el.
   inc is the increment with which to lengthen list when needed.
   Can be used when list=cur=lim=0 (to initialize a list). */
#define addtolist(elty, list, el, cur, lim, inc) do {			\
     growlist(elty, list, cur, lim, inc);				\
     list[cur++] = el;		/* Add element to list */		\
} while (0)

#endif

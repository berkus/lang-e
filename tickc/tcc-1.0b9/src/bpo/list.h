/* $Id: list.h,v 1.1.1.1 1997/12/05 01:25:42 maxp Exp $ */

#ifndef __LIST_H__
#define __LIST_H__

#include "mem.h"

struct list_t {
     void *x;
     struct list_t *link;
};
typedef struct list_t *List;

extern List l_prepend (void *x, List list, int arena);
extern List l_append (void *x, List list, int arena);
extern void l_remove (void *x, List *list);
extern int l_length (List list);

/* map */
#define l_mapc(l, ty, f, v) do {		\
     if (l) {					\
	  List __lp = l;			\
	  do {					\
               __lp = __lp->link;               \
	       (f)((ty)__lp->x, (v));		\
	  } while (__lp != l);	                \
     }						\
} while (0)

/* filter: create a new list l2 consisting of the elements of l1
   (of type type) matching predicate */
#define l_filter(l2, l1, type, predicate) do {	\
     if (l1) {					\
	  List __lp = l1;			\
	  l2 = 0L;				\
	  do					\
	       if (predicate((type)(__lp->x)))	\
		    l2 = l_append(__lp->x, l2);	\
	  while ((__lp = __lp->link) != l1);	\
     }						\
} while (0)

/* ltov: convert list to a null-terminated array v of type */
#define l_ltov(v, type, list, arena) do {		\
     int __i = 0;					\
     if (list) {					\
          NEW((v), l_length(list)+1, (arena));		\
          if (list) {					\
       	       List __lp = list;			\
       	       do {					\
		    __lp = __lp->link;			\
		    (v)[__i++] = (type)__lp->x;		\
	       } while (__lp != list);			\
          }						\
          (v)[__i] = (type)0;				\
     } else { (v) = 0L; }	        		\
} while (0)

#endif

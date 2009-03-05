/* $Id: mem.h,v 1.1.1.1 1997/12/05 01:25:42 maxp Exp $ */

#ifndef __AMEM_H__
#define __AMEM_H__

#include <memory.h>

#define NELEMS(a) ((int)(sizeof (a)/sizeof ((a)[0])))

/* Arena-based memory management */

extern void *aalloc (unsigned long n, unsigned int a);
extern void adealloc (unsigned int a);

				/* Arenas: '2' is reserved for string */
enum { ARENA0=0, ARENA1=1, ARENA2=2 };
				/* Allocate n adjacent chunks of sizeof *p */
#define NEW(p,n,a) ((p) = aalloc((n)*(sizeof *(p)), (a)))
				/* Allocate and set to all 0s */
#define NEW0(p,n,a) do {			\
     int __k = (n);				\
     memset(NEW((p),__k,(a)), 0, __k*(sizeof *(p)));\
} while (0)
				/* Allocate and set to all 1s */
#define NEW1(p,n,a) do {			        \
     int __k = (n);				        \
     memset(NEW((p),__k,(a)), 0xFF, __k*(sizeof *(p)));	\
} while (0)
				/* Allocate and set to all xs */
#define NEWX(p,n,x,a) do {				\
     int __k = (n);					\
     memset(NEW((p),__k,(a)), (char)(x), __k*(sizeof *(p)));\
} while (0)

#endif

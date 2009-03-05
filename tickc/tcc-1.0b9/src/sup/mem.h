/* $Id: mem.h,v 1.1 1998/01/19 20:30:30 maxp Exp $ */

#ifndef __MEM_H__
#define __MEM_H__

/* Arena-based memory management */

extern void *tc_aalloc (unsigned long n);
extern void *tc_palloc (unsigned long n);
extern void tc_adealloc ();

extern void *memset (void *, int, unsigned);

				/* Allocate n adjacent chunks of sizeof *p */
#define NEW(p,n) ((p) = tc_aalloc((n)*(sizeof *(p))))
				/* Allocate and set to all 0s */
#define NEW0(p,n) do {						\
     int __new0_k = (n);					\
     memset(NEW((p),__new0_k), 0, __new0_k*(sizeof *(p)));	\
} while (0)
				/* Allocate and set to all 1s */
#define NEW1(p,n) do {				\
     int __new1_k = (n);				\
     memset(NEW((p),__new1_k), 0xFF, __new1_k*(sizeof *(p)));	\
} while (0)
				/* Allocate and set to all xs */
#define NEWX(p,n,x) do {						\
     int __newx_k = (n);						\
     memset(NEW((p),__newx_k), (char)(x), __newx_k*(sizeof *(p)));	\
} while (0)

				/* Permanent alloc */
#define PNEW(p,n) ((p) = tc_palloc((n)*(sizeof *(p))))

#define PNEW0(p,n) do {						\
     int __pnew0_k = (n);					\
     memset(PNEW((p),__pnew0_k), 0, __pnew0_k*(sizeof *(p)));	\
} while (0)

#endif

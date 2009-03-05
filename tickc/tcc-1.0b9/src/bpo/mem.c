/* $Id: mem.c,v 1.1.1.1 1997/12/05 01:25:42 maxp Exp $ */

/* Pretty much verbatim from lcc/alloc.c by Chris Fraser and Dave Hanson */

#include <stdio.h>
#include <assert.h>
#include "mem.h"

#if defined(__sparc__) && !defined(__LCC__)
extern int fprintf();
#endif

#define roundup(x,n) (((x)+((n)-1))&(~((n)-1)))
#define CHUNKSIZE (64*1024)

extern void *malloc(unsigned int);
extern int free(void *);
extern void exit(int);

struct block {
     struct block *next;
     char *limit;
     char *avail;
};
union align {
     long l;
     char *p;
     double d;
     int (*f)();
};
union header {
     struct block b;
     union align a;
};

static struct block
	 first[] = {  { 0L },  { 0L }, { 0L} },
	*arena[] = { &first[0], &first[1], &first[2] };
static struct block *freeblocks;

void *aalloc (unsigned long n, unsigned int a) {
     struct block *ap;

     assert(a < NELEMS(arena));
     assert(n > 0);
     ap = arena[a];
     n = roundup(n, sizeof (union align));
     while (ap->avail + n > ap->limit) {
	  if ((ap->next = freeblocks) != NULL) {
	       freeblocks = freeblocks->next;
	       ap = ap->next;
	  } else {
	       unsigned m = sizeof (union header) + n + CHUNKSIZE;
	       ap->next = malloc(m);
	       ap = ap->next;
	       if (ap == NULL) {
		    fprintf(stderr, "Fatal: insufficient memory for aalloc\n");
		    exit(1);
	       }
	       ap->limit = (char *)ap + m;
	  }
	  ap->avail = (char *)((union header *)ap + 1);
	  ap->next = NULL;
	  arena[a] = ap;
     }
     ap->avail += n;
     return ap->avail - n;
}

void adealloc (unsigned int a) {
     assert(a < NELEMS(arena));
     arena[a]->next = freeblocks;
     freeblocks = first[a].next;
     first[a].next = NULL;
     arena[a] = &first[a];
}

/* $Id: mem.c,v 1.2 1998/01/26 19:23:39 maxp Exp $ */

#include <stdio.h>
#include <assert.h>
#include "mem.h"

#if defined(__sparc__) && !defined(__LCC__)
extern int fprintf (FILE *, const char *, ...);
#endif

#define roundup(x,n) (((x)+((n)-1))&(~((n)-1)))
#define CHUNKSIZE (128*1024)

extern void *malloc (unsigned int);
extern void free (void *);

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


#if defined(debug_arena)

union header *arena;

void *tc_aalloc (unsigned long n) {
     union header *new = malloc(sizeof *new + n);

     if (new == NULL) {
	  fprintf(stderr, "Fatal: insufficient memory for debug arena.\n");
	  exit(1);
     }
     new->b.next = (void *)arena;
     arena = new;
     return new + 1;
}

void tc_adealloc (void) {
     union header *p, *q;

     for (p = arena; p; p = q) {
	  q = (void *)p->b.next;
	  free(p);
     }
     arena = NULL;
}


#else

static struct block
	 first = { NULL },
	*arena = &first;
static struct block *freeblocks;

void *tc_aalloc (unsigned long n) {
     struct block *ap;

     assert(n > 0);
     ap = arena;
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
		    fprintf(stderr, "Fatal: insufficient memory for arena.\n");
		    exit(1);
	       }
	       ap->limit = (char *)ap + m;
	  }
	  ap->avail = (char *)((union header *)ap + 1);
	  ap->next = NULL;
	  arena = ap;
     }
     ap->avail += n;
     return ap->avail - n;
}

void tc_adealloc (void) {
     arena->next = freeblocks;
     freeblocks = first.next;
     first.next = NULL;
     arena = &first;
}

#endif


/*
 * tc_palloc: permanent allocation - this stuff never gets deallocated
 */

static struct block pfirst = { NULL }, *parena = &pfirst;

void *tc_palloc (unsigned long n) {
     struct block *ap;

     assert(n > 0);
     ap = parena;
     n = roundup(n, sizeof (union align));
     if (ap->avail + n > ap->limit) {
          unsigned m = sizeof (union header) + n + CHUNKSIZE;
          ap->next = malloc(m);
          ap = ap->next;
          if (ap == NULL) {
               fprintf(stderr,"Fatal: insufficient memory for perm arena.\n");
               exit(1);
          }
          ap->limit = (char *)ap + m;
          ap->avail = (char *)((union header *)ap + 1);
          ap->next = NULL;
          parena = ap;

     }
     ap->avail += n;
     return ap->avail - n;
}

/* $Id: set.h,v 1.1.1.1 1997/12/05 01:25:44 maxp Exp $ */

#ifndef __SET_H__
#define __SET_H__

/* Simple set implementation using a hash table */

				/* Define the set element type here */
#include "btype.h"
typedef T Setelt;

#include "mem.h"

struct setw_t {			/* Wrapper for set elements */
     Setelt val;
     struct setw_t *next;
};

struct set_t {			/* Set object */
     int arena;
     int card;
     struct setentry {
	  struct setw_t elt;
	  struct setentry *link;
     } *buckets[128];
     struct setw_t *all;
};

typedef struct set_t *Set;


/* Prototypes */

extern Set set (int arena);
extern void set_add (Set s, Setelt e);
extern int set_find (Set s, Setelt e);
extern int set_card (Set s);

#endif

/* $Id: sym.h,v 1.1.1.1 1997/12/05 01:25:43 maxp Exp $ */

#ifndef __SYM_H__
#define __SYM_H__

/* Symbols */

struct symbol_t {
     char *name;
     int l;
     int r;
     struct symbol_t *next;
};

typedef struct symbol_t *Symbol;


/* Symbol tables */

struct table_t {
     int arena;
     struct tableentry {
	  struct symbol_t sym;
	  struct tableentry *link;
     } *buckets[128];
     Symbol all;
};

typedef struct table_t *Table;


/* Prototypes */

extern Table table (int arena);
extern void foreach (Table tp, void (*f)(Symbol, void*, void*),
		     void *a1, void *a2);
extern Symbol install (char *name, Table tp);
extern Symbol lookup (char *name, Table tp);
#endif

/* $Id: tog.c,v 1.2 1997/12/11 01:26:16 maxp Exp $ */

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include "list.h"
#include "mem.h"
#include "string.h"

#ifndef __GNUC__
#define inline
#endif

#if defined(__sparc__) && !defined(__LCC__) && !defined(__SVR4)
extern int fprintf (FILE *, char *, ...);
extern int printf (char *, ...);
extern int tolower (int);
extern int fclose (FILE *);
#endif

#ifdef NDEBUG
#define DEBUG(x)
#else
#define DEBUG(x) x
#endif
#define PDEBUG(x)  DEBUG(if (debugp) printf x;)

#define PSZ sizeof(int)
#define HSZ 107			/* Size of the hash table */
#define LSZ 256			/* Maximum line size */
#define VSZ 64			/* Variable table size */
#define MNP 16384		/* Max number of patterns */

static char usage[] = "[-d] -t table file -r rule file {input file}*";
static int debugp;
enum { IN = 0, OUT };

extern void exit(int);
static void error (char *msg) {
     fprintf(stderr, "tog: %s\n", msg);
     exit(1);
}

static void warning (char *msg) {
     fprintf(stderr, "tog: Warning: %s\n", msg);
}

/* bdumpi: dump the integer x as binary, most significant bit first */
inline static char *bdumpi (int x) {
     char *out;
     int i;
     NEW(out, 8*PSZ, ARENA0);
     for (i = 8*PSZ-1; i >= 0; x >>= 1, i--)
	  out[i] = "01"[x&1];
     return out;
}

/* mkwildcard: create a bog wildcard token for a pointer to char */
static char *wildcard;
inline static char *mkwildcard () {
     char *out;
     NEWX(out, 1+8*sizeof out, '-', ARENA0);
     out[8*sizeof out] = '\0';
     return out;
}

/* uniq: */
static int uniq (char *str, char **us, int *id) {
     static struct hnode {
	  char *str;
	  int id;
	  struct hnode *ptr;		  
     } *htab[HSZ] = { 0 };
     static int np = 0;		/* Number of unique patterns */
     struct hnode *p;
     char *p1, *p2, *s;
     int i;
	
     for (s = str, i = 0; *s; i += *s++) ;
     i %= HSZ;

     for (p = htab[i]; p; p = p->ptr) 
	  for (p1=str, p2=p->str; *p1++ == *p2++; )
	       if (p1[-1] == '\0') {
		    *us = p->str; *id = p->id;
		    return 0;	/* Str was not a new pattern */
	       }
     NEW(p, 1, ARENA0);
     NEW(p->str, s-str+1, ARENA0);
     strcpy(p->str, str);
     p->id = np++; p->ptr = htab[i];
     htab[i] = p;
				/* Distinguish pattern table entries */
     assert((int)p->str > MNP && p->id <= MNP);	
     *us = p->str; *id = p->id;
     return 1;			/* Str was a new, unique (so far) pattern */
}

/* unparsebin: generate bog input (binary rewrite specs) */
static int cnt;
inline static void unparsebinline (void *v, FILE *fp) {
     if ((cnt++)&0x1) {
	  char *c = (char *)v;
	  while (*c) { *wildcard = *c++; fprintf(fp, "%s\n", wildcard); }
     } else
	  fprintf(fp, "%s\n", bdumpi((int)v));
}
static void unparsebin (List r, FILE *fp) {
     List *v, *p;
				/* Create a vector of lists */
     l_ltov(v, List, r, ARENA0);
     for (p = v; *p || *(p+1); p++) {
	  cnt = 0; l_mapc(*p, void *, unparsebinline, fp); fprintf(fp, "=\n");
	  p++;
	  cnt = 0; l_mapc(*p, void *, unparsebinline, fp); fprintf(fp, "+\n");
     }
}

/* unparsepat: generate C pattern table for tpo */
inline static void unparsetxtlin (char *v, FILE *fp) {
     fprintf(fp, "\t\"%s\",\n", v);
}
static void unparsepat (List p, FILE *fp) {
     fprintf(fp, "unsigned int tpoNP = %d;\n", l_length(p));
     fprintf(fp, "char *tpoP[] = {\n");
     l_mapc(p, char *, unparsetxtlin, fp);
     fprintf(fp, "\t0\n};\n");
}

/* unparsetxt: generate C rule table for tpo */
static void unparsetxt (List r, FILE *fp) {
     List *v, *p;
     int i;

     l_ltov(v, List, r, ARENA0);
     for (p = v, i = 0; *p; p++, i++) {
	  fprintf(fp, "char *tr%dins[] = {\n", i);
	  l_mapc(*p, char *, unparsetxtlin, fp);
	  fprintf(fp, "\t0\n};\n");
     }
     fprintf(fp, "trule_t tpoR[] = {\n");
     for (p = v, i = 0; *p; p++, i++)
	  fprintf(fp, "\t{ %d, tr%dins },\n", l_length(*p), i);
     fprintf(fp, "\t{ 0 }\n};\n");
}

/* convert: read the text rewrite rules on fp, generating bog rewrite rules (on
   fr) and C tables (on ft) for use with tpo */
static void convert (FILE *fp, FILE *fr, FILE *ft) {
     int phase = IN, lc = 0, id, vused[26], i;
     char c, l[LSZ], *lp, v[VSZ], *vp;
     List trul = 0, tlin = 0, brul = 0, blin = 0, patterns = 0;

     for (i = 0; i < 26; i++) vused[i] = 0;
     while (fgets(l, LSZ, fp)) {
	  ++lc;
	  if (*l == '#' || *l == '\n') continue;
	  if (!strcmp(l, "=\n")) { /* End of input pattern */
	       if (phase != IN)
		    error(stringf("Misplaced '=' at line %d", lc));
	       if (!blin)
		    warning(stringf("Empty input pattern at line %d", lc));
	       trul = l_append((void *)tlin, trul, ARENA0); tlin = 0;
	       brul = l_append((void *)blin, brul, ARENA0); blin = 0;
	       phase = OUT;
	       continue;
	  }
	  if (!strcmp(l, "+\n")) { /* End of output pattern */
	       if (phase != OUT)
		    error(stringf("Misplaced '+' at line %d", lc));
	       brul = l_append((void *)blin, brul, ARENA0); blin = 0;
	       for (i = 0; i < 26; i++) vused[i] = 0;
	       phase = IN;
	       continue;
	  }
				/* Extend text line list */
	  for (lp = l; *lp; lp++) ; if (*--lp == '\n') *lp = '\0';
	  if (phase == IN)
	       tlin = l_append((void*)string(l), tlin, ARENA0);

	  vp = v;		/* "Uniqueify" string */
	  for (lp = l; *lp; lp++)
	       if (*lp == '%')
		    if ((c = tolower(*++lp)) >= 'a' && c <= 'z') {
			 int j = c-'a';
			 *vp++ = c; *lp = ' ';
			 if (phase == IN)
			      vused[j] = 1;
			 else if (!vused[j])
			      error(stringf("Symbol '%%%c' used with no prior "
					    "definition at line %d", c, lc));
		    } else if (c != '%')
			 error(stringf("Illegal expression '%%%c' at line %d",
				       c, lc));
	  *vp++ = 0;
				/* Extend list of uniq'd patterns */
	  if (uniq(l, &lp, &id))
	       patterns = l_append((void *)lp, patterns, ARENA0);
	  PDEBUG(("convert: %s :: %d\n", lp, id));
				/* Extend binary line list (for bog) */
	  blin = l_append((void *)id, blin, ARENA0);
	  blin = l_append((void *)string(v), blin, ARENA0);
     }

     unparsebin(brul, fr);
     fprintf(ft, "#include \"tpo.h\"\n\n");
     unparsepat(patterns, ft);
     unparsetxt(trul, ft);
}

int main (int argc, char **argv) {
     FILE *ft=0, *fr=0, *fp=0;
     int nt = 0, nr = 0, i;

     for (i = 1; i < argc; i++)
	  if (!strcmp(argv[i], "-d"))
	       debugp = 1;
	  else if (!strcmp(argv[i], "-t")) {
	       if (++i >= argc || nt++) error(usage);
	       if ((ft = fopen(argv[i], "w")) == NULL)
		    error(stringf("Can't write to file '%s'", argv[i]));
	  } else if (!strcmp(argv[i], "-r")) {
	       if (++i >= argc || nr++) error(usage);
	       if ((fr = fopen(argv[i], "w")) == NULL)
		    error(stringf("Can't write to file '%s'", argv[i]));
	  } else {
	       if (!nr || !nt) error(usage);
	       if ((fp = fopen(argv[i], "r")) == NULL)
		    error(stringf("Can't read file '%s'", argv[i]));
	       wildcard = mkwildcard();
	       convert(fp, fr, ft);
	       fclose(fp);
	  }

     if (!nr || !nt) error(usage);

     fclose(fr); fclose(ft);
     return 0;
}

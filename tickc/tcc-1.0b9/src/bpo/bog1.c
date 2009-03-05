/* $Id: bog1.c,v 1.2 1997/12/11 01:26:07 maxp Exp $ */

#include "bog.h"

#include <stdio.h>
#if defined(__sparc__) && !defined(__LCC__) && !defined(__SVR4)
extern int tolower(int);
extern int _filbuf(FILE *);
#endif

void error (char *msg) {
     fprintf(stderr, "bog: %s\n", msg);
     exit(1);
}

/* checksame: aborts program if s and a1 refer to the same variable but have
   different bit widths */
void checksame (Symbol s, void *a1, void *a2) {
     Symbol cs = (Symbol)a1;
     if (s->name == cs->name && s->r - s->l != cs->r - cs->l)
	  error(stringf("Symbol '%s' has two different sizes in rule %d",
			cs->name, nr));
}

/* mkvar: adds s to the list l of the current pattern's variables */
List mkvar (Symbol s, List l) {
     var_t *var;

     if (debugp) 
	  fprintf(stderr, "/* mkvar: %s [l=%d,r=%d] */\n", 
		  s->name, s->l, s->r);
     foreach(stab, checksame, s, 0);

     NEW(var, 1, ARENA1);
     var->id = s->name[0]-'a';
     var->sh = SZ-1-s->r;
     var->msk = ((2 << (s->r - s->l)) - 1) << var->sh;
     return l_append((void *)var, l, ARENA1);
}

/* scan: parse the contents of fp, expecting the input side (phase) of a rule
   if ph=='i', or the output side if ph=='o' */
List scan (FILE *fp, int ph) {
     int c = 0;			/* Current char on fp */
     int nb = 0, nw = 0;	/* Position in rule (bit/word) */
     Symbol csym = 0L;		/* Current variable being scanned */
     T f_msk=(T)0, f_val=(T)0;	/* Current constant mask and value */
     List pats = 0L;		/* All the patterns (words) for this phase */
     List vars = 0L;		/* All the vars for a given pattern */

     while ((c=getc(fp)) != EOF) {
	  if (debugp)
	       fprintf(stdout, "/* bit: %c msk: 0x%x val: 0x%x */\n", 
		       c, (unsigned)f_msk, (unsigned)f_val);
				/* Inputs end with '=', outputs with '+' */
	  if ((c == '=' && ph == 'i') || (c == '+' && ph == 'o'))
	       break;
	  switch(c) {
	  case ' ': case '\f': case '\n': case '\r': case '\t': case '\v': 
	       continue;
	  case '#':		/* Comment character */
	       while ((c=getc(fp)) != EOF && c != '\n');
	       if (c == EOF && (nb > 0 || (ph == 'i' && nw > 0)))
		    error(stringf("Unexpected end of file reading patterns at "
				  "rule %d, word %c%d, bit %d", nr,ph,nw,nb));
	       continue;
	  case '-':		/* Denotes body of a variable */
	       if (!csym)	/* We must be defining a variable */
		    error(stringf("'-' appears with no leading symbol "
			 "at rule %d, word %c%d, bit %d", nr, ph, nw, nb));
	       else {		/* Extend the variable's right boundary */
		    ++csym->r; assert(csym->r == nb);
	       }
	       break;
	  case '0': case '1':	/* Constant characters */
	       if (csym) {	/* End any variable definition */
		    vars = mkvar(csym, vars); csym = 0L;
	       }
	       f_msk |= 1; f_val |= (c-'0');
	       break;
	  default:
	       c = tolower(c);	/* Variables named by a-z, case insensitive */
	       if (c >= 'a' && c <= 'z') {
		    char *s = stringf("%c", c);
		    if (csym)
			 vars = mkvar(csym, vars);
		    if (!lookup(s, stab) && ph == 'o')
			 error(stringf("Symbol '%c' used with no prior "
				       "definition at rule %d, word o%d",
				       c, nr, nw));
		    csym = install(s, stab);
		    csym->l = csym->r = nb;
	       } else
		    error(stringf("Illegal character in rule file at rule %d, "
				  "word %c%d, bit %d", nr, ph, nw, nb));
	  }
	  if (nb == SZ-1) {	/* End of a pattern (word): append this info */
	       pat_t *pat;	/*  to list of patterns, and reset current */
	       if (csym) {	/*  pattern state (masks, vars, bit count) */
		    vars = mkvar(csym, vars); csym = 0L;
	       }
	       NEW(pat, 1, ARENA1);
	       pat->f_msk = f_msk; f_msk = (T)0;
	       pat->f_val = f_val; f_val = (T)0;
	       pat->nv = l_length(vars);
	       if (debugp)
		    fprintf(stdout, "/* msk: 0x%x, val: 0x%x, nv: 0x%x */\n",
			    (unsigned)pat->f_msk, (unsigned)pat->f_val, 
			    pat->nv);
	       l_ltov(pat->v, var_t *, vars, ARENA1);

	       pats = l_append(pat, pats, ARENA1);
	       ++nw; nb = 0; vars = 0L;
	  } else {		/* Still more to go: move on to next bit */
	       ++nb; f_msk <<= 1; f_val <<= 1;
	  }
     }
     if (c == EOF)
	  if (ph == 'i' && nb+nw > 0)
	       error(stringf("Unexpected end of file reading patterns "
			     "at rule %d, word i%d, bit %d", nr, nw, nb));
     if (nb != 0)
	  error(stringf("Pattern size must be multiple of %d near "
			"rule %d, word %c%d", SZ, nr, ph, nw));
     return pats;
}

/* unparse_vars: print out info about all the variables in pattern (word) np 
   of rule nr.  The info is an array of iv_t or ov_t objects, depending on
   whether ph=='i' or ph=='o'. */
void unparse_vars (pat_t *p, int ph, int np) {
     int i;
     if (p->nv) {
	  var_t **v = p->v;
	  printf("%cv_t r%d%c%dv[] = {\n", ph, nr, ph, np);
	  for (i = 0; i < p->nv; i++)
	       printf("\t{ 0x%x, %d%s},\n", v[i]->id, v[i]->sh,
		      ph == 'i' ? stringf(", 0x%x", v[i]->msk) : " ");
	  printf("\t{ 0 }\n};\n");
     }
}

/* unparse_pats: unparse all the patterns of the current rule, nr.  This info
   is an array of iw_t or ow_t objects, depending on whether ph=='i' or
   ph=='o'. */
pat_t ** unparse_pats (List pl, int ph) {
     pat_t **pv = 0L;
     int l = l_length(pl), i;

     if (l) {
	  l_ltov(pv, pat_t*, pl, ARENA1);
	  for (i = 0; i < l; i++)
	       unparse_vars(pv[i], ph, i);

	  printf("%cw_t r%d%c[] = {\n", ph, nr, ph);
	  for (i = 0; i < l; i++)
	       printf("\t{%s0x%x, %d, %s },\n",
		      ph == 'i' ? stringf(" 0x%x, ", pv[i]->f_msk) : " ",
		      (unsigned)pv[i]->f_val, pv[i]->nv,
		      pv[i]->nv ? stringf("r%d%c%dv", nr, ph, i) : "0");
	  printf("\t{ 0 }\n};\n");
     }
     return pv;
}

void unparse_perm (pat_t **i, pat_t **o) {
     pat_t **p, **q; int k;
     if (!o) return;
     printf("int r%drd[] = {\n", nr);
     for (p = o; *p; p++) {
	  if ((*p)->nv == 1 && (*p)->v[0]->msk == TMAX && i) {
	       for (q = i, k = 0; *q; q++, k++)
		    if ((*q)->nv == 1 && (*q)->v[0]->id == (*p)->v[0]->id) {
			 printf("\t%d,\n", k);
			 break;
		    }
	       assert(*q);
	  } else printf("\t-1,\n");
     }
     printf("\t0\n};\n");
}

List do_rule (List rl, List ins, List outs) {
     pat_t **iv, **ov;

     if (treep) {
	  rule_t *r;
	  NEW(r, 1, ARENA1);
	  r->nin = l_length(ins);
	  r->nout = l_length(outs);
	  l_ltov(r->in, pat_t *, ins, ARENA1);
	  l_ltov(r->out, pat_t *, outs, ARENA1);
	  rl = l_append(r, rl, ARENA1);
     } else {
	  iv = unparse_pats(ins, 'i'); 
	  ov = unparse_pats(outs, 'o');
	  unparse_perm(iv, ov);
	  rl = l_append((void*)l_length(ins), rl, ARENA1);
	  rl = l_append((void*)l_length(outs), rl, ARENA1);
     }
     return rl;
}

int main (int argc, char **argv) {
     FILE *fp;
     List rl = 0L; 
     int i;

     printf("#include \"bpo.h\"\n\n");
				/* Process one input file at a time */
     for (i = 1; i < argc; i++)
	  if (!strcmp(argv[i], "-d"))
	       debugp = 1;
	  else if (!strcmp(argv[i], "-t"))
	       treep = 1;
	  else if ((fp=fopen(argv[i], "r")) == NULL)
	       error(stringf("Can't open patterns file '%s'", argv[i]));
	  else {
	       List ins = 0, outs = 0;

	       stab = table(ARENA0);
	       ins = scan(fp, 'i'); outs = scan(fp, 'o');
	       if (outs && !ins)
		    error(stringf("Empty input pattern at rule %d", nr));
	       while (ins || outs) {

		    rl = do_rule(rl, ins, outs);

		    adealloc(ARENA0); stab = table(ARENA0);
		    nr++;
		    ins = scan(fp, 'i'); outs = scan(fp, 'o');
	       }
	  }
				/* Generate an array of rule_t to store
				   all info about rules generated above */
     if (nr) {
	  if (treep)
	       treematch(rl);
	  else {
	       int *rv, j;
	       l_ltov(rv, int, rl, ARENA1);
	       printf("rule_t bpoR[] = {\n");
	       for (i = 0, j = 0; i < nr; i++, j += 2) 
		    printf("\t{ %d, %s, %d, %s, %s },\n", 
			   rv[j], rv[j] ? stringf("r%di", i) : "0",
			   rv[j+1], rv[j+1] ? stringf("r%do", i) : "0",
			   rv[j+1] ? stringf("r%drd", i) : "0"
			 );
	       printf("\t{ 0 }\n};\nint bpoNR = %d;\n", nr);
	  }
     }
     adealloc(ARENA1);
     return 0;
}

/* $Id: bog2.c,v 1.2 1997/12/11 01:26:09 maxp Exp $ */

/* check arena ids */

#include "bog.h"

/* emitprologue: emit the bpo function prologue */
void emitprologue (void) {
     char c;
     printf(
"#include <assert.h>\n"
"#include <stdlib.h>\n"
"#if !defined(__GNUC__) || defined(__STRICT_ANSI__)\n"
"#define inline\n"
"#endif\n\n"
"#if defined(__sparc__) && !defined(__LCC__)\n"
"extern int printf();\n"
"#endif\n"
"#ifdef NDEBUG\n"
"#define DEBUG(x)\n"
"#else\n"
"#define DEBUG(x) x\n"
"#endif\n"
"#define PDEBUG(x)  DEBUG(if (dp) printf x;)\n"
"#define PDEBUG2(x) DEBUG(if (dp>1) printf x;)\n"
"#define BSZ 512\n"
"#define HSZ 1024\n\n"
"static inline int blengthmatch (int *lhl, int *lhp, int rl) {\n"
"  int i = 0;\n"
"  while (--lhp >= lhl) {\n"
"  \ti += *lhp;\n"
"  \tif (i == rl) return 1;\n"
"  }\n"
"  return 0;\n"
"}\n\n"
"W bpo (W ih, W it, W il, W oh, W ot, W ol, nft nf,\n"
"#ifndef __bpo_no_relo__\n"
"\t\tint *fdb, int *opb,\n"
"#endif\n"
"\t\tint dp) {\n"
"  int *lhl = 0, *lhp = 0;	/* History buffer pointers */\n"
"  int tnf = nf == (nft)0;	/* True if using trivial next function */\n"
"  int nn;\n"
"  DEBUG(int nrepl = -1;)\n"
"  T "
	  );

     for (c='a'; c < 'z'; c++) printf("%c,", c);
     printf("z;\n");

     printf(
"  assert(oh < ol && il <= ih && il < it);\n\n"
"  if (!tnf)\n"
"  \tlhp = lhl = (int *)malloc(HSZ*sizeof(int));\n"
"\n"
"  nextpass:\n"
"  DEBUG(++nrepl;)\n"
"  while (GT(it, ih)) {\n"
"  \tassert(GT(ol, ot));\n"
"  \t			/* Head(output) <-- head(input) */\n"
"  \tif (!tnf) {\n"
"  \t\tfor (nn = nf(ih)-ih-1, i = 0; i < nn; i++, INCR(ih), INCR(ot)) \n"
"  \t\t\tASGN(ot, ih);\n"
"  \t\tassert(lhp < &lhl[HSZ]);\n"
"  \t\t*lhp++ = nn+1;\n"
"  \t}\n"
"  \tASGN(ot, ih); /* Unroll last ASGN: avoid DECR(ot),DECR(ih) */\n"
"  \tPDEBUG2((\"ih=0x%%p, it=0x%%p, il=0x%%p, oh=0x%%p, ot=0x%%p, "
"ol=0x%%p\\n\",ih, it, il, oh, ot, ol));\n"
	  );
}

/* emitepilogue: emit the bpo function epilogue */
void emitepilogue (void) {
     printf(
"  \tINCR(ih); INCR(ot);\n"
"  }\n"
"  if (!tnf) free(lhl);\n"
"  PDEBUG((\"BPO: made %%d replacements\\n\", nrepl));\n"
"  return ot;\n"
"}\n"
	  );
}
/* emitrewrite: generates code to implement the output side of rule cr, with
   indentation ind */
void emitrewrite (rule_t *cr, char *ind) {
     var_t **pv, *v;
     pat_t **pp, *p;
     int pos;
     static int cnt = 0;

     printf("\n%s\t{\n", ind);
     printf("\t\t%sPDEBUG((\"applying rule %d\\n\"));\n", ind, cnt++);
     printf("\t\t%sassert(!GT(il, ih - %d));\n", ind, cr->nout-1);
     for (pp = cr->out, pos = cr->nout; pp && ((p = *pp)); pp++, pos--) {
	  printf("\t\t%sih[ - %d] = 0x%x ", ind, pos-1, 
		 (unsigned int)p->f_val);
	  for (pv = p->v; pv && ((v = *pv)); pv++)
	       printf("\n\t\t\t%s| ((%c<<%d)&0x%x)", 
		      ind, v->id+'a', v->sh, (unsigned int)v->msk);
	  printf(";\n");
     }
     printf("\t\t%sot -= %d;\n", ind, cr->nin-1);
     printf("\t\t%sih -= %d;\n", ind, cr->nout-1);
     printf("\t\t%sgoto nextpass;\n", ind);
     printf("%s\t}\n", ind);
}

/* emitchecks: generates code to individually test for matches of rules that
   cannot be distinguished using mrst method */
void emitchecks (rule_t **r, int mil, marktype status, char *ind) {
     var_t **pv, *v;
     pat_t **pp, *p;
     rule_t *cr;
     T *vmsk, *vval;
     int pos;
     int filtp = 0;		/* True if generating filter code */
     int asgn[26];		/* Records which variables are assigned */

     NEW0(vmsk, mil+1, ARENA0);
     NEW0(vval, mil+1, ARENA0);
				/* Emit bounds check */
     printf("\n%s\tif ((tnf || blengthmatch(lhl, lhp, %d))", ind, mil);
     if (!marked(status, mil, CHECKED))
	  printf("\n%s\t    && (ot-%d >= oh)) {", ind, mil-1);
     else
	  printf(") {");
     if (r[1] && r[2]) {	/* Emit filter code for common substrings */
	  for (pos = 1; pos <= mil; pos++) {
	       T val, msk = maskatpos(r, pos);
	       if (msk && valatpos(r, pos, msk, &val)) {
		    printf("\n%s\t%s((ot[ - %d]&0x%x) == 0x%x)", 
			   ind, filtp ? "    && " :  "if (", pos-1, 
			   (unsigned int)msk, (unsigned int)val);
		    vmsk[pos] = msk;
		    vval[pos] = val;
		    filtp = 1;
	       }
	  }
	  if (filtp) printf(") { /* begin filter */");
     }
     while ((cr = *r++)) {
	  int ifp = 0, i;
	  for (i = 0; i < 26; i++) asgn[i] = 0;
				/* Check constant part */
	  for (pp = cr->in, pos = cr->nin; pp && ((p = *pp)); pp++, pos--) {
				/* Skip checks that are already filtered */
	       if (p->f_msk != vmsk[pos] || p->f_val != vval[pos]) {
		    printf("\n%s\t%s((ot[ - %d]&0x%x) == 0x%x)",
			   ind, ifp ? "    && " :  "if (", pos-1,
			   (unsigned int)p->f_msk, (unsigned int)p->f_val);
		    ifp = 1;
	       }
	  }
				/* Check variables */
	  for (pp = cr->in, pos = cr->nin; pp && ((p = *pp)); pp++, pos--) {
	       for (pv = p->v; pv && ((v = *pv)); pv++) {
		    if (!asgn[v->id]) {
			 printf("\n%s\t%s(((%c = ((ot[ - %d]&0x%x)>>%d)))|1)", 
				ind, ifp ? "    && " :  "if (", v->id+'a',
				pos-1, (unsigned int)v->msk, v->sh);
			 asgn[v->id] = 1;
		    } else
			 printf("\n%s\t    && (%c == ((ot[ - %d]&0x%x)>>%d))",
				ind, v->id+'a', pos-1, (unsigned int)v->msk, 
				v->sh);
		    ifp = 1;
	       }
	  }
	  if (ifp) printf(")");
	  emitrewrite(cr, ind);
     }
     if (filtp) printf("\t%s} /* end filter */\n", ind);
     printf("\t%s}\n", ind);
     adealloc(ARENA0);
}

void mrstatpos (rule_t **r, int pos, int mil, marktype status, char *ind) {
     int lb, rb, n, i;
     int bcheckp = 0;		/* True if generating bounds checks */
     T msk = maskatpos(r, pos);
     T w = criticalwindow(r, pos, msk);
     lb = leftbit(w); rb = rightbit(w); n = lb-rb+1;
     if (lb == -1) {		/* No more window */
	  assert(rb == -1);
	  if (markedupto(status, mil, DONE)) {
	       rule_t **sr, **lr;
	       filterlen(r, mil, &sr, &lr);
				/* Try long rules before short ones */
	       if (lr) metamrst(lr, status, ind);
	       if (sr) emitchecks(sr, mil, status, ind);
	  } else
	       metamrst(r, status, ind);
	  return;
     }

     ind = indent(ind);
     if (!markedabove(status, pos, CHECKED)) {
	  bcheckp = 1;
	  printf("\n%sif (ot-%d >= oh) { /* begin bounds check */\n", 
		 ind, pos-1);
	  mark(status, pos, CHECKED); 
     }
     printf("%sswitch((ot[ - %d]&0x%x)>>%d) {\n", ind, pos-1, 
	    (unsigned int)w, rightbit(w));
     for (i = 0; i < (1<<n); i++) {
	  rule_t **r2;
	  printf("%scase %d:", ind, i);
	  r2 = filterwin(r, pos, w, i);
	  if (r2)
	       mrstatpos(r2, pos, mil, markcopy(status), ind);
	  printf("\n%sbreak;\n", ind);
     }
     printf("%s}\n", ind);
     if (bcheckp) printf("%s} /* end bounds check */\n", ind);
}

void metamrst (rule_t **r, marktype status, char *ind) {
     int mil = minlen(r);	/* Minimum length of input side of any rule */
     int pos = cwpos(r, mil, status);	/* Position of most critical window */
     assert(pos >= 1 && pos <= mil);
     mark(status, pos, DONE);
     mrstatpos(r, pos, mil, status, ind);
}

void treematch (List rl) {
     rule_t **rv;		/* Rule vector */
     l_ltov(rv, rule_t *, rl, ARENA1);
     emitprologue();
     metamrst(rv, markinit(maxlen(rv)), "");
     emitepilogue();
}

/* $Id: cwmisc.c,v 1.1.1.1 1997/12/05 01:25:42 maxp Exp $ */

#include "bog.h"

/* Contains code for computing and dealing with critical windows,
   filtering rule lists based on values and length,
   and other helper code as well */

int debugp, treep, nr;
Table stab;
int *done;

/*
 * Output-related
 */

/* indent: return a new indentation string, one '\t' deeper than ind */
char *indent (char *ind) {
     char *ns;
     NEW(ns, strlen(ind)+2, ARENA1);
     strcpy(ns, ind); strcat(ns, "\t");
     return ns;
}

/*
 * Bit manipulation
 */

/* leftbit: find the index of the leftmost (most-significant) bit in w */
int leftbit (T w) {
     int l = 8*sizeof(T)-1;
     T v = 1<<l;
     while (!(w&v) && v) { v >>= 1; l -= 1; }
     return l;
}

/* rightbit: find the index of the rightmost (least-significant) bit in w */
int rightbit (T w) {
     int r = 0;
     T v = 1;
     while (!(w&v) && v) { v <<= 1; r += 1; }
     if (!v) r = -1;
     return r;
}

/*
 * Rule set manipulation
 */

/* minlen: find minimum input-side length for the rule set rv */
int minlen (rule_t  **rv) {
     int i, min = rv[0]->nin;
     for (i = 1; rv[i]; i++)
	  if (min > rv[i]->nin) min = rv[i]->nin;
     return min;
}

/* maxlen: find maximum input-side length for the rule set rv */
int maxlen (rule_t **rv) {
     int i, max = rv[0]->nin;
     for (i = 1; rv[i]; i++)
	  if (max < rv[i]->nin) max = rv[i]->nin;
     return max;
}

/* maskatpos: find the intersection of the fixed masks at position pos from the
   end in the set of rules r */
T maskatpos (rule_t **r, int pos) {
     int i, elt;
     T msk = TMAX;
     for (i = 0; r[i]; i++) {
	  elt = r[i]->nin - pos; assert(elt >= 0);
	  msk &= r[i]->in[elt]->f_msk;
     }
     return msk;
}

/* valatpos: if rules r have a common fixed value at position pos and mask msk,
   returns 1 and sets *val to that value, else returns 0 */
int valatpos (rule_t **r, int pos, T msk, T *val) {
     T v;
     int i, elt = r[0]->nin - pos; assert(elt >= 0);

     for (v = (r[0]->in[elt]->f_val&msk), i = 1; r[i]; i++) {
	  elt = r[i]->nin - pos; assert(elt >= 0);
				/* msk must be a subset of any elt's f_msk */
	  assert((r[i]->in[elt]->f_msk&msk) == msk);
	  if ((r[i]->in[elt]->f_val&msk) != v)
	       return 0;
     }
     *val = v;
     return 1;
}

/*
 * Rule filtering
 */

/* filterlen: breaks rules in r into two sets; those with input sides of length
  == mil are placed in s, those with length > mil in l.  Requires that r
  contain no rules of length < mil */
void filterlen (rule_t **r, int mil, rule_t ***s, rule_t ***l) {
     rule_t *cr;
     List sl = 0, ll = 0;
     while ((cr = *r++)) {
	  assert(cr->nin >= mil);
	  if (cr->nin == mil) 
	       sl = l_append((void *)cr, sl, ARENA0);
	  else
	       ll = l_append((void *)cr, ll, ARENA0);
     }
				/* Build output vectors */
     l_ltov(*s, rule_t *, sl, ARENA1);
     l_ltov(*l, rule_t *, ll, ARENA1);
     adealloc(ARENA0);
}

/* filterwin: returns a vector of all the rules in r for which window w at
   position pos has value val.  Returns 0 if no such rules exist */
rule_t **filterwin (rule_t **r, int pos, T w, int val) {
     rule_t *cr;
     List fl = 0; rule_t **fv = 0;
     int sh = rightbit(w);
     while ((cr = *r++)) {
	  int elt = cr->nin - pos; assert(elt >= 0);
	  assert((cr->in[elt]->f_msk&w) == w);
	  if (((cr->in[elt]->f_val&w) >> sh) == val)
	       fl = l_append((void *)cr, fl, ARENA0);
     }
     if (fl) l_ltov(fv, rule_t *, fl, ARENA1);
     adealloc(ARENA0);
     return fv;
}

/*
 * Critical windows
 */

/* card: return the cardinality of (number of distinct values in) window w
   at position pos from the end of the input side of each rule in rule
   vector r */
int card (T w, rule_t **r, int pos) {
     Set s = set(ARENA1);
     int elt, i;
     for (i = 0; r[i]; i++) {
	  elt = r[i]->nin - pos; assert(elt >= 0);
	  set_add(s, (r[i]->in[elt]->f_val & r[i]->in[elt]->f_msk) & w);
     }
     return set_card(s);
}

/* iscritical: returns whether window w at position pos from the end of the
   input side of each rule in r is critical */
int iscritical (T w, rule_t **r, int pos) {
     int lb = leftbit(w), rb = rightbit(w);
     return card(w, r, pos) > (1<<(lb-rb));
}

/* criticalwindow: finds the critical window for rules r at position pos from
   the end of the input side of each rule, with mask msk */
T criticalwindow (rule_t **r, int pos, T msk) {
     int i, l = 8*sizeof(T)-1;
     T w = 0, wmax = 0;
     for (i = l; i >= 0; i--) {
	  w = w | (1<<i);	/* Extend w one bit to the right */
	  if (iscritical(w, r, pos) && (w&msk) == w)
	       wmax = w;
	  else {
	       int k = leftbit(w);
	       w &= (1<<k)-1;	/* Strip left bit of w */
	       if (card(w, r, pos) > card(wmax, r, pos) && (w&msk) == w)
		   wmax = w;
	  }
     }
     return wmax;
}

/* cwcard: returns the cardinality of the most critical window at position pos
   from the end of each rule in r */
int cwcard (rule_t **r, int pos) {
     T msk;
     if ((msk = maskatpos(r, pos))) {
	  T w = criticalwindow(r, pos, msk);
	  return card(w, r, pos);
     }
     return 0;
}

/* cwpos: finds the position (from the end) of the most critical unmarked
   window for the set of rules r with minimum input-side length mil */
int cwpos (rule_t **r, int mil, marktype status) {
     int card = 0, maxcard = 0, pos = -1, i;
     for (i = 1; i <= mil; i++) {
	  if (marked(status, i, DONE)) continue;
	  card = cwcard(r, i);
	  if (card >= maxcard) { /* >= to get a new card when '1' is marked */
	       maxcard = card;
	       pos = i;
	  }
     }
     return pos;
}

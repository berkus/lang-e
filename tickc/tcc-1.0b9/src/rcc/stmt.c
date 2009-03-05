#include "c.h"

#define SWSIZE 512

#define den(i,j) ((j-buckets[i]+1.0)/(v[j]-v[buckets[i]]+1))

int fordef;

struct swtch {
     Symbol sym;
     int lab;
     Symbol deflab;
     int ncases;
     int size;
     int *values;
     Symbol *labels;
};

struct code scodehead = { Start };
struct code dcodehead = { Start };
int codetype = NORMAL;
Code scodelist = &scodehead;
Code dcodelist = &dcodehead; 
Code codelist = &scodehead;

float density = 0.5;
Table stmtlabs;

static int foldcond ARGS((Tree e1, Tree e2));
static void kdefinelab ARGS((int));
static void kbranch ARGS((int));
static void branch ARGS((int));
static void caselabel ARGS((Swtch, int, int));
static void cmp ARGS((int, Symbol, int, int));
static Tree conditional ARGS((int));
static void dostmt ARGS((int, Swtch, int));
static int equal ARGS((Symbol, Symbol));
static void forstmt ARGS((int, Swtch, int));
static void ifstmt ARGS((int, int, Swtch, int));
static Symbol localaddr ARGS((Tree));
static void stmtlabel ARGS((void));
static void swcode ARGS((Swtch, int *, int, int));
static void swgen ARGS((Swtch));
static void swstmt ARGS((int, int, int));
static void whilestmt ARGS((int, Swtch, int));

#if defined(__GNUC__)
static void doscode ARGS((void));
static void dodcode ARGS((void));
static inline void doscode() {
     dcodelist = codelist;
     codelist = scodelist;
     codetype = NORMAL;
}
static inline void dodcode() {
     scodelist = codelist;
     codelist = dcodelist;
     codetype = TEMPLATE;
}
#else
#define doscode() {		\
     dcodelist = codelist;	\
     codelist = scodelist;	\
     codetype = NORMAL;		\
}
#define dodcode() {		\
     scodelist = codelist;	\
     codelist = dcodelist;	\
     codetype = TEMPLATE;	\
}
#endif

void sw2scode() {
     if (codetype == TEMPLATE) {
	  eval.ticklevel = 0;
	  doscode();
     }
}
void sw2dcode() {
     if (codetype == NORMAL) {
	  eval.ticklevel = level;
	  dodcode();
     }
}
Code code(kind) int kind; {
     Code cp;
     int dflag = 0;

     if (codetype == TEMPLATE 
	 && (eval.unqtlevel || eval.rtcp) && !eval.lcllevel) {
	  doscode();
	  dflag = 1;
     }

     if (kind > Start) {
	  for (cp = codelist; cp->kind < Label; )
	       cp = cp->prev;
	  if (cp->kind == Jump || cp->kind == Switch)
	       warning("unreachable code\n");
     }
     NEW(cp, FUNC);
     cp->kind = kind;
     cp->prev = codelist;
     cp->next = NULL;
     codelist->next = cp;
     codelist = cp;

     if (dflag)
	  dodcode();

     return cp;
}
void addlocal(p) Symbol p; {
     if (!p->defined) {
	  code(Local)->u.var = p;
	  p->defined = 1;
	  p->scope = level;
     }
}
void definept(p) Coordinate *p; {
     Code cp = code(Defpoint);

     cp->u.point.src = p ? *p : src;
     cp->u.point.point = npoints;
     if (ncalled > 0) {
	  int n = findcount(cp->u.point.src.file,
			    cp->u.point.src.x, cp->u.point.src.y);
	  if (n >= 0)
	       refinc = (float)n/ncalled;
     }
     if (glevel > 2)	locus(identifiers, &cp->u.point.src);
     if (events.points)
     {
	  Tree e = NULL;
	  apply(events.points, &cp->u.point.src, &e);
	  if (e)
	       listnodes(e, 0, 0);
     }
}
void statement(loop, swp, lev, evalp) int loop, lev; Swtch swp; int evalp; {
     float ref = refinc;

     if (Aflag >= 2 && lev == 15)
	  warning("more than 15 levels of nested statements\n");
     switch (t) {
     case IF:       
	  ifstmt(genlabel(2), loop, swp, lev + 1);
	  break;
     case WHILE:    
	  whilestmt(genlabel(3), swp, lev + 1); break;
     case DO:       
	  dostmt(genlabel(3), swp, lev + 1); 
	  if (! evalp)
	       expect(';');
	  break;
     case FOR:      
	  forstmt(genlabel(4), swp, lev + 1);
	  break;
     case BREAK:    
	  walk(NULL, 0, 0);
	  definept(NULL);
	  if (swp && swp->lab > loop)
	       branch(swp->lab + 1);
	  else if (loop)
	       branch(loop + 2);
	  else
	       error("illegal break statement\n");
	  t = gettok(); 
	  if (! evalp)
	       expect(';');
	  break;
     case CONTINUE: 
	  walk(NULL, 0, 0);
	  definept(NULL);
	  if (loop)
	       branch(loop + 1);
	  else
	       error("illegal continue statement\n");
	  t = gettok(); 
	  if (! evalp)
	       expect(';');
	  break;
     case SWITCH:   
	  swstmt(loop, genlabel(2), lev + 1);
	  break;
     case CASE:     {
	  int lab = genlabel(1);
	  if (swp == NULL)
	       error("illegal case label\n");
	  definelab(lab);
	  while (t == CASE) {
	       static char stop[] = { IF, ID, 0 };
	       Tree p;
	       t = gettok();
	       p = constexpr(0);
	       if (generic(p->op) == CNST && isint(p->type)) {
		    if (swp) {
			 needconst++;
			 p = cast(p, swp->sym->type);
			 needconst--;
			 caselabel(swp, p->u.v.i, lab);
		    }
	       } else
		    error("case label must be a constant integer "
			  "expression\n");
	       test(':', stop);
	  }
	  statement(loop, swp, lev, 0);
     }
     break;
     case DEFAULT:  
	  if (swp == NULL)
	       error("illegal default label\n");
	  else if (swp->deflab)
	       error("extra default label\n");
	  else {
	       swp->deflab = findlabel(swp->lab);
	       definelab(swp->deflab->u.l.label);
	  }
	  t = gettok();
	  expect(':');
	  statement(loop, swp, lev, 0); break;
     case RETURN:   {
	  Type rty = eval.ticklevel ? NULL : freturn(cfunc->type);
	  t = gettok();
	  definept(NULL);
	  if (t != ';')
	       if (!eval.ticklevel && rty == voidtype) {
		    error("extraneous return value\n");
		    expr(0);
		    retcode(NULL);
	       } else {
		    Tree t = expr(0);
		    retcode(t);
	       }
	  else {
	       if (!eval.ticklevel && rty != voidtype
		   && (rty != inttype || Aflag >= 1))
		    warning("missing return value\n");
	       retcode(NULL);
	  }
	  if (!eval.ticklevel)
	       branch(cfunc->u.f.label);
	  if (!evalp) {
	       expect(';');
	  }
     }
     break;
     
     case '{':      
	  compound(loop, swp, lev + 1); break;
     case ';':      
	  definept(NULL); t = gettok(); break;
     case GOTO:     
	  walk(NULL, 0, 0);
	  definept(NULL);
	  t = gettok();
	  if (t == ID) {
	       Symbol p = lookup(token, stmtlabs);
	       if (p == NULL) {
		    p = install(token, &stmtlabs, 0, FUNC);
		    p->scope = LABELS;
		    p->u.l.label = genlabel(1);
		    p->src = src;
	       }
	       if (eval.ticklevel)
		    if (lookup(token, eval.luse) == NULL)
			 installcopy(p, &eval.luse, 0, FUNC);
	       use(p, src);
	       branch(p->u.l.label);
	       t = gettok();
	  } else
	       error("missing label in goto\n"); 
	  if (! evalp)
	       expect(';');
	  break;

     case ID:       
	  if (getchr() == ':') {
	       stmtlabel();
	       statement(loop, swp, lev, 0);
	       break;
	  }
     default:      
	  definept(NULL);
	  if (kind[t] != ID) {
	       error("unrecognized statement\n");
	       t = gettok();
	  } else {
	       Tree e = expr0(0);
	       listnodes(e, 0, 0);
	       if (nodecount == 0 || nodecount > 200)
		    walk(NULL, 0, 0);
	       else if (glevel) walk(NULL, 0, 0);
	       if (! eval.tickp)
		    deallocate(STMT);
	  } 
	  if (! evalp)
	       expect(';');
	  break;

     }
     if (kind[t] != IF && kind[t] != ID
	 && t != '}' && t != EOI && !(evalp == 1 && t == ',')) {
	  static char stop[] = { IF, ID, '}', 0 };
	  error("illegal statement termination\n");
	  skipto(0, stop);
     }
     refinc = ref;
}

static void ifstmt(lab, loop, swp, lev) int lab, loop, lev; Swtch swp; {
     Tree cnd;
     int rtccnd;
     t = gettok();
     expect('(');
     definept(NULL);
     cnd = conditional(')');
     if ((rtccnd = isrtcop(cnd->op)))
	  walk(ktest(cnd), 0, lab);
     else
	  walk(cnd, 0, lab);

     refinc /= 2.0;
     if (eval.ticklevel)
	  tickrefcnt(DIV, 2);
     statement(loop, swp, lev, 0);
     if (rtccnd) {
	  if (t == ELSE) {
	       kbranch(lab+1);
	       t = gettok();
	       kdefinelab(lab);
	       statement(loop, swp, lev, 0);
	       kdefinelab(lab + 1);
	  } else
	       kdefinelab(lab);
     } else {
	  if (t == ELSE) {
	       branch(lab + 1);
	       t = gettok();
	       definelab(lab);
	       statement(loop, swp, lev, 0);
	       if (findlabel(lab + 1)->ref)
		    definelab(lab + 1);
	  } else
	       definelab(lab);
     }
     if (eval.ticklevel)
	  tickrefcnt(MUL, 2);
}
static Tree conditional(tok) int tok; {
     Tree p = expr(tok);

     if (Aflag > 1 && isfunc(p->type))
	  warning("%s used in a conditional expression\n",
		  funcname(p));
     return cond(p);
}
static void stmtlabel() {
     Symbol p = lookup(token, stmtlabs);

     if (p == NULL) {
	  p = install(token, &stmtlabs, 0, FUNC);
	  p->scope = LABELS;
	  p->u.l.label = genlabel(1);
	  p->src = src;
     }
     if (eval.ticklevel)
	  if (lookup(token, eval.ldef) == NULL)
	       installcopy(p, &eval.ldef, 0, FUNC);
     if (p->defined)
	  error("redefinition of label `%s' previously defined at %w\n", 
		p->name, &p->src);

     p->defined = 1;
     definelab(p->u.l.label);
     t = gettok();
     expect(':');
}
/* rtcrvalue: returns 1 if t is an expression that can legally initialize, 
   bound, or change the value of a derived rtc loop invariant, 0 otherwise. */
static int rtcrvalue (Tree t) {
     int retval = 1;
     int op = generic(t->op);

     if (op == ADDRL && !t->u.sym->rtcp || op == ADDRF
	 || op == ADDRG || op == ADRFL || op == ADRFF) 
	  return 0;
     if (op == RTC) 
	  return 1;
     if (t->kids[0]) retval &= rtcrvalue(t->kids[0]);
     if (t->kids[1]) retval &= rtcrvalue(t->kids[1]);
     return retval;
}
/* rtcasgn: returns 1 if t is an assignment that can define a "run-time
   constant" loop induction variable (i.e. lhs is a local variable and
   rhs is a constant or rtc.  Returns 0 otherwise.  t must be non-null. */
static int rtcasgn (Tree t, Symbol *drtciv) {
     if (eval.autounroll && eval.tickp && generic(t->op) == ASGN
	 && generic(t->kids[0]->op) == ADDRL && !t->kids[0]->u.sym->assigned
	 && rtcrvalue(t->kids[1])) {
	  *drtciv = t->kids[0]->u.sym;
	  return 1;
     }
     return 0;
}
/* rtcbound: marks the derived rtc loop iv (drtciv) as a derived
   run-time constant (sets rtcp and drtcp bits).
   Returns 1 if t is a tree that can bound the loop so as to make drtciv
   a derived-runtime constant, and 0 otherwise.  t must be non-null. */
static int rtcbound (Tree t, Symbol drtciv) {
     Tree k0 = t->kids[0], k1 = t->kids[1];
     int op = generic(t->op);

     drtciv->rtcp = drtciv->drtcp = 1;

     if (! (op == EQ || op == GE || op == GT || op == LE
	    || op == LT || op == NE))
	  return 0;

     assert(k0 && k1);

     if ((isrvalue(k0) && k0->kids[0]->u.sym
	  && k0->kids[0]->u.sym->rtcp && rtcrvalue(k1))
	 || (isrvalue(k1) && k1->kids[0]->u.sym
	     && k1->kids[0]->u.sym->rtcp && rtcrvalue(k0)))
	  return 1;
     return 0;
}
static void forstmt(lab, swp, lev) int lab, lev; Swtch swp; {
     int once = 0;
     int klab = 0;
     int rtce1 = 0;
     Symbol drtciv = NULL;	/* Possible derived rtc loop induction var */
     Tree e1 = NULL, e2 = NULL, e3 = NULL;
     Coordinate pt2, pt3;
	
     t = gettok();
     expect('(');
     definept(NULL);

     fordef = 1;
     if (kind[t] == ID)
	  e1 = texpr(expr0, ';', FUNC);
     else
	  expect(';');
     fordef = 0;

     if (rtcasgn(e1, &drtciv))
	  rtce1 = 1;
     else
	  walk(e1, 0, 0);
     pt2 = src;
     refinc *= 10.0;
     if (eval.ticklevel)
	  tickrefcnt(MUL, 10);
     if (kind[t] == ID)
	  e2 = texpr(conditional, ';', FUNC);
     else
	  expect(';');
     pt3 = src;
     if (kind[t] == ID)
	  e3 = texpr(expr0, ')', FUNC);
     else {
	  static char stop[] = { IF, ID, '}', 0 };
	  test(')', stop);
     }
     if (rtce1 && rtcbound(e2, drtciv) && rtcrvalue(e3)) {
				/* Mark loop induction variable as rtc */
	  walk(notetree(stringf(
	       "{\n%s;\nfor (%s; %s; %s) { /* begin loop */",
	       typestring(0, drtciv->type, drtciv->name),
	       unparsetree(e1), unparsetree(e2),
	       unparsetree(e3))), 0, 0);
	  statement(lab, swp, lev, 0);
	  definelab(lab+1);
	  if (findlabel(lab + 2)->ref)
	       definelab(lab+2);
	  walk(notetree(string("} /* end loop */ \n}")), 0, 0);
	  tickrefcnt(DIV, 10);
	  return;
     } else {
	  if (drtciv) 
	       drtciv->rtcp = drtciv->drtcp = 0;
	  if (e1)
	       walk(e1, 0, 0);
	  if (e2) {
	       if (isrtcop(e2->op)) {
		    klab = genlabel(1);
		    walk(ktest(e2), 0, klab);	/* no codegen */
	       } else {
		    once = foldcond(e1, e2);
		    if (!once)
			 branch(lab + 3);
	       }
	  }
     }
     definelab(lab);
     statement(lab, swp, lev, 0);
     definelab(lab + 1);
     definept(&pt3);
     if (e3)
	  walk(e3, 0, 0);
     if (e2) {
	  if (isrtcop(e2->op)) {
	       walk(notetree("printf(\"DCG Warning: run-time constant "
			     "may cause infinite loop in 'for' "
			     "statement\\n\");"), 0, 0);
	       branch(lab);		/* if rtc is true, infinite loop */
	       kdefinelab(klab);		/* else emit nothing */
	  } else {
	       if (!once)
		    definelab(lab + 3);
	       definept(&pt2);
	       walk(e2, lab, 0);
	  }
     } else {
	  definept(&pt2);
	  branch(lab);
     }
     if (findlabel(lab + 2)->ref)
	  definelab(lab + 2);
     if (eval.ticklevel)
	  tickrefcnt(DIV, 10);
}
static void swstmt(loop, lab, lev) int loop, lab, lev; {
     Tree e;
     struct swtch sw;
     Code head, tail;

     t = gettok();
     expect('(');
     definept(NULL);
     e = expr(')');
     if (!isint(e->type)) {
	  error("illegal type `%t' in switch expression\n",
		e->type);
	  e = retype(e, inttype);
     }
     e = cast(e, promote(e->type));
     if (generic(e->op) == INDIR && isaddrop(e->kids[0]->op)
	 && e->kids[0]->u.sym->type == e->type
	 && !isvolatile(e->kids[0]->u.sym->type)) {
	  sw.sym = e->kids[0]->u.sym;
	  walk(NULL, 0, 0);
     } else {
	  sw.sym = genident(REGISTER, e->type, level);
	  addlocal(sw.sym);
	  walk(asgn(sw.sym, e), 0, 0);
     }
     head = code(Switch);
     sw.lab = lab;
     sw.deflab = NULL;
     sw.ncases = 0;
     sw.size = SWSIZE;
     sw.values = newarray(SWSIZE, sizeof *sw.values, FUNC);
     sw.labels = newarray(SWSIZE, sizeof *sw.labels, FUNC);
     refinc /= 10.0;
     if (eval.ticklevel)
	  tickrefcnt(DIV, 10);
     statement(loop, &sw, lev, 0);
     if (sw.deflab == NULL) {
	  sw.deflab = findlabel(lab);
	  definelab(lab);
	  if (sw.ncases == 0)
	       warning("switch statement with no cases\n");
     }
     if (findlabel(lab + 1)->ref)
	  definelab(lab + 1);
     tail = codelist;
     codelist = head->prev;
     codelist->next = head->prev = NULL;
     if (sw.ncases > 0)
	  swgen(&sw);
     branch(lab);
     head->next->prev = codelist;
     codelist->next = head->next;
     codelist = tail;
     if (eval.ticklevel)
	  tickrefcnt(MUL, 10);
}
static void caselabel(swp, val, lab) Swtch swp; int val, lab; {
     int k;

     if (swp->ncases >= swp->size) {
	  int    *vals = swp->values;
	  Symbol *labs = swp->labels;
	  swp->size *= 2;
	  swp->values = newarray(swp->size, sizeof *swp->values, FUNC);
	  swp->labels = newarray(swp->size, sizeof *swp->labels, FUNC);
	  for (k = 0; k < swp->ncases; k++) {
	       swp->values[k] = vals[k];
	       swp->labels[k] = labs[k];
	  }
     }
     k = swp->ncases;
     for ( ; k > 0 && swp->values[k-1] >= val; k--) {
	  swp->values[k] = swp->values[k-1];
	  swp->labels[k] = swp->labels[k-1];
     }
     if (k < swp->ncases && swp->values[k] == val)
	  error("duplicate case label `%d'\n", val);
     swp->values[k] = val;
     swp->labels[k] = findlabel(lab);
     ++swp->ncases;
     if (Aflag >= 2 && swp->ncases == 258)
	  warning("more than 257 cases in a switch\n");
}
static void swgen(swp) Swtch swp; {
     int *buckets, k, n, *v = swp->values;

     buckets = newarray(swp->ncases + 1,
			sizeof *buckets, FUNC);
     for (n = k = 0; k < swp->ncases; k++, n++) {
	  buckets[n] = k;
	  while (n > 0 && den(n-1, k) >= density)
	       n--;
     }
     buckets[n] = swp->ncases;
     swcode(swp, buckets, 0, n - 1);
}
static void swcode(swp, b, lb, ub) Swtch swp; int b[]; int lb, ub; {
     int hilab, lolab, l, u, k = (lb + ub)/2;
     int *v = swp->values;

     if (k > lb && k < ub) {
	  lolab = genlabel(1);
	  hilab = genlabel(1);
     } else if (k > lb) {
	  lolab = genlabel(1);
	  hilab = swp->deflab->u.l.label;
     } else if (k < ub) {
	  lolab = swp->deflab->u.l.label;
	  hilab = genlabel(1);
     } else
	  lolab = hilab = swp->deflab->u.l.label;
     l = b[k];
     u = b[k+1] - 1;
     if (u - l + 1 <= 3)
     {
	  int i;
	  for (i = l; i <= u; i++)
	       cmp(EQ, swp->sym, v[i], swp->labels[i]->u.l.label);
	  if (k > lb && k < ub)
	       cmp(GT, swp->sym, v[u], hilab);
	  else if (k > lb)
	       cmp(GT, swp->sym, v[u], hilab);
	  else if (k < ub)
	       cmp(LT, swp->sym, v[l], lolab);
	  else
	       assert(lolab == hilab),
		    branch(lolab);
	  walk(NULL, 0, 0);
     }
     else {
	  Symbol table = genident(STATIC,
				  array(voidptype, u - l + 1, 0), LABELS);
	  (*IR->defsymbol)(table);
	  cmp(LT, swp->sym, v[l], lolab);
	  cmp(GT, swp->sym, v[u], hilab);
	  walk(tree
	       (JUMP, voidtype,
		rvalue((*optree['+'])
		       (ADD, pointer(idtree(table)),
			(*optree['-'])(SUB, cast(idtree(swp->sym), inttype),
				       consttree(v[l], inttype)))), NULL), 
	       0, 0);
	  code(Switch);
	  codelist->u.swtch.table = table;
	  codelist->u.swtch.sym = swp->sym;
	  codelist->u.swtch.deflab = swp->deflab;
	  codelist->u.swtch.size = u - l + 1;
	  codelist->u.swtch.values = &v[l];
	  codelist->u.swtch.labels = &swp->labels[l];
	  if (v[u] - v[l] + 1 >= 10000)
	       warning("switch generates a huge table\n");
     }
     if (k > lb) {
	  assert(lolab != swp->deflab->u.l.label);
	  definelab(lolab);
	  swcode(swp, b, lb, k - 1);
     }
     if (k < ub) {
	  assert(hilab != swp->deflab->u.l.label);
	  definelab(hilab);
	  swcode(swp, b, k + 1, ub);
     }
}
static void cmp(op, p, n, lab) int op, n, lab; Symbol p; {
     listnodes(eqtree(op,
		      cast(idtree(p), inttype),
		      consttree(n, inttype)),
	       lab, 0);
}
void retcode(p) Tree p; {
     Type ty;

     if (p == NULL) {
	  if (events.returns)
	       apply(events.returns, cfunc, NULL);
	  return;
     }
     p = pointer(p);
     if (eval.ticklevel) {
	  ty = p->type;
	  if (ty == voidtype)
	       error("attempt to return void type\n");
     } else
	  ty = assign(freturn(cfunc->type), p);
     if (ty == NULL) {
	  error("illegal return type; found `%t' expected `%t'\n",
		p->type, freturn(cfunc->type));
	  return;
     }
     p = cast(p, ty);
     if (retv) {
	  if (iscallb(p))
	       p = tree(RIGHT, p->type,
			tree(CALL+B, p->type,
			     p->kids[0]->kids[0], idtree(retv)),
			rvalue(idtree(retv)));
	  else
	       p = asgntree(ASGN, rvalue(idtree(retv)), p);
	  walk(p, 0, 0);
	  if (events.returns)
	       apply(events.returns, cfunc, rvalue(idtree(retv)));
	  return;
     }
     if (events.returns) {
	  Symbol t1 = genident(AUTO, p->type, level);
	  addlocal(t1);
	  walk(asgn(t1, p), 0, 0);
	  apply(events.returns, cfunc, idtree(t1));
	  p = idtree(t1);
     }
     p = cast(p, promote(p->type));
     if (isptr(p->type)) {
	  Symbol q = localaddr(p);
	  if (q && (q->computed || q->generated))
	       warning("pointer to a %s is an illegal return value\n",
		       q->scope == PARAM ? "parameter" : "local");
	  else if (q)
	       warning("pointer to %s `%s' is an illegal return value\n",
		       q->scope == PARAM ? "parameter" : "local", q->name);
	  p = cast(p, unsignedtype);
     }
     else if  (isspec(p->type)) {
	  assert(! eval.ticklevel);
	  p = retype(p, unsignedtype);
     }
     walk(tree(RET + widen(p->type), p->type, p, NULL), 0, 0);
}
void closureret(t) Tree t; {
     Type ty = t->type;
     walk(tree(CRET + (isunsigned(ty) ? I : ttob(ty)), ty, t, NULL), 0, 0);
}
void definelab(lab) int lab; {
     Code cp;
     Symbol p = findlabel(lab);

     assert(lab);
     walk(NULL, 0, 0);
     code(Label)->u.forest = newnode(LABELV, NULL, NULL, p);
     for (cp = codelist->prev; cp->kind <= Label; )
	  cp = cp->prev;
     while (   cp->kind == Jump
	       && cp->u.forest->kids[0]
	       && cp->u.forest->kids[0]->op == ADDRGP
	       && cp->u.forest->kids[0]->syms[0] == p) {
	  assert(cp->u.forest->kids[0]->syms[0]->u.l.label == lab);
	  p->ref--;
	  assert(cp->next);
	  assert(cp->prev);
	  cp->prev->next = cp->next;
	  cp->next->prev = cp->prev;
	  cp = cp->prev;
	  while (cp->kind <= Label)
	       cp = cp->prev;
     }
}
static void kdefinelab(lab) int lab; {
     assert(lab);
     walk(NULL, 0, 0);
     code(Label)->u.forest = newnode(KLABELV, NULL, NULL, kfindlabel(lab));
}
static void kbranch(lab) int lab; {
     assert(lab);
     walk(NULL, 0, 0);
     code(Label)->u.forest = kjump(lab);
}
Node jump(lab) int lab; {
     Symbol p = findlabel(lab);

     p->ref++;
     return newnode(JUMPV, newnode(ADDRGP, NULL, NULL, p),
		    NULL, NULL);
}
static void branch(lab) int lab; {
     Code cp;
     Symbol p = findlabel(lab);

     assert(lab);
     walk(NULL, 0, 0);
     code(Label)->u.forest = jump(lab);
     for (cp = codelist->prev; cp->kind < Label; )
	  cp = cp->prev;
     while (   cp->kind == Label
	       && cp->u.forest->op == LABELV
	       && !equal(cp->u.forest->syms[0], p)) {
	  equatelab(cp->u.forest->syms[0], p);
	  assert(cp->next);
	  assert(cp->prev);
	  cp->prev->next = cp->next;
	  cp->next->prev = cp->prev;
	  cp = cp->prev;
	  while (cp->kind < Label)
	       cp = cp->prev;
     }
     if (cp->kind == Jump || cp->kind == Switch) {
	  p->ref--;
	  codelist->prev->next = NULL;
	  codelist = codelist->prev;
     } else {
	  codelist->kind = Jump;
	  if (cp->kind == Label
	      &&  cp->u.forest->op == LABELV
	      &&  equal(cp->u.forest->syms[0], p))
	       warning("source code specifies an infinite loop");
     }
}
void equatelab(old, new) Symbol old, new; {
     assert(old->u.l.equatedto == NULL);
     old->u.l.equatedto = new;
     new->ref++;
}
static int equal(lprime, dst) Symbol lprime, dst; {
     assert(dst && lprime);
     for ( ; dst; dst = dst->u.l.equatedto)
	  if (lprime == dst)
	       return 1;
     return 0;
}
/* dostmt - do statement while ( expression ) */
static void dostmt(lab, swp, lev) int lab, lev; Swtch swp; {
     Tree cnd;
     int klab = genlabel(1);
     refinc *= 10.0;
     if (eval.ticklevel)
	  tickrefcnt(MUL, 10);
     t = gettok();
     definelab(lab);
     statement(lab, swp, lev, 0);
     definelab(lab + 1);
     expect(WHILE);
     expect('(');
     definept(NULL);
     cnd = conditional(')'); 
     if (isrtcop(cnd->op)) {
	  walk(ktest(cnd), 0, klab);	/* no codegen */
	  walk(notetree("printf(\"DCG Warning: run-time constant may "
			"cause infinite loop in 'do-while' "
			"statement\\n\");"), 0, 0);
	  branch(lab);		/* if rtc is true, infinite loop */
	  kdefinelab(klab);	/* else emit nothing */
     } else {
	  walk(cnd, lab, 0);
	  if (findlabel(lab + 2)->ref)
	       definelab(lab + 2);
     }
     if (eval.ticklevel)
	  tickrefcnt(DIV, 10);
}

/* foldcond - check if initial test in for(e1;e2;e3) S is necessary */
static int foldcond(e1, e2) Tree e1, e2; {
     int op = generic(e2->op);
     Symbol v;

     if (e1 == 0 || e2 == 0)
	  return 0;
     if (generic(e1->op) == ASGN && isaddrop(e1->kids[0]->op)
	 && generic(e1->kids[1]->op) == CNST) {
	  v = e1->kids[0]->u.sym;
	  e1 = e1->kids[1];
     } else
	  return 0;
     if ((op==LE || op==LT || op==EQ || op==NE || op==GT || op==GE)
	 && generic(e2->kids[0]->op) == INDIR
	 && e2->kids[0]->kids[0]->u.sym == v
	 && e2->kids[1]->op == e1->op) {
	  e1 = simplify(op, e2->type, e1, e2->kids[1]);
	  if (e1->op == CNST+I)
	       return e1->u.v.i;
     }
     return 0;
}

/* localaddr - returns q if p yields the address of local/parameter q; 
   otherwise returns 0 */
static Symbol localaddr(p) Tree p; {
     if (p == NULL)
	  return NULL;
     switch (generic(p->op)) {
     case INDIR: case CALL: case ARG:
	  return NULL;
     case ADDRL: case ADRFL: case ADDRF: case ADRFF:
	  return p->u.sym;
     case RIGHT: case ASGN:
	  if (p->kids[1])
	       return localaddr(p->kids[1]);
	  return localaddr(p->kids[0]);
     case COND: {
	  Symbol q;
	  assert(p->kids[1] && p->kids[1]->op == RIGHT);
	  if ((q = localaddr(p->kids[1]->kids[0])) != NULL)
	       return q;
	  return localaddr(p->kids[1]->kids[1]);
     }
     default: {
	  Symbol q;
	  if (p->kids[0] && (q = localaddr(p->kids[0])) != NULL)
	       return q;
	  return localaddr(p->kids[1]);
     }
     }
}

/* whilestmt - while ( expression ) statement */
static void whilestmt(lab, swp, lev) int lab, lev; Swtch swp; {
     Coordinate pt;
     Tree e;
     int klab = genlabel(1);

     refinc *= 10.0;
     if (eval.ticklevel)
	  tickrefcnt(MUL, 10);
     t = gettok();
     expect('(');
     walk(NULL, 0, 0);
     pt = src;
     e = texpr(conditional, ')', FUNC);
     if (isrtcop(e->op)) {
	  walk(ktest(e), 0, klab);	/* no code gen */
	  definelab(lab);
	  statement(lab, swp, lev, 0);
	  walk(notetree("printf(\"DCG Warning: run-time constant "
			"may cause infinite loop in 'do-while' "
			"statement\\n\");"), 0, 0);
	  branch(lab);
	  kdefinelab(klab);		/* emit nothing */
     } else {
	  branch(lab + 1);
	  definelab(lab);
	  statement(lab, swp, lev, 0);
	  definelab(lab + 1);
	  definept(&pt);
	  walk(e, lab, 0);
	  if (findlabel(lab + 2)->ref)
	       definelab(lab + 2);
     }
     if (eval.ticklevel)
	  tickrefcnt(DIV, 10);
}

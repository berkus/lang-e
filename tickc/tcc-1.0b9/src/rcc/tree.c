#include "c.h"

static int where = STMT;
static int nid = 1;		/* identifies trees & nodes in debugging output */
static struct nodeid {
     int printed;
     Tree node;
} ids[500];			/* if ids[i].node == p, then p's id is i */

static void printtree1 ARGS((Tree, int, int));
static Tree mktreecsym ARGS((Symbol, Type, int));
static inline Tree mktreespec ARGS((Tree, Type));
static inline Tree mktreefvar ARGS((Tree, Type));

/* 
   mktree: equivalent to lcc's tree(), this is the core of tcc's tree()
*/
#if defined(__GNUC__)
static inline Tree mktree ARGS((int, Type, Tree, Tree));
#else
				/* If not using gcc, use a macro */
#define mktree(kind, ty, left, right)	\
     NEW0(p, where);			\
     p->op = kind;			\
     p->type = ty;			\
     p->kids[0] = left;			\
     p->kids[1] = right;

#endif

#if defined(__GNUC__)
static inline Tree mktree(op, type, left, right)
int op; Type type; Tree left, right; {
     Tree p;

     NEW0(p, where);
     p->op = op;
     p->type = type;
     p->kids[0] = left;
     p->kids[1] = right;
     return p;
}
#endif

/* 
   mktreecsym: given a Symbol s of Type ty, returns a tree denoting
   the closure field containing s.  If cs = 1, the symbol denotes 
   a cspec: the generated tree will not be usable as an lvalue.
*/
static Tree mktreecsym(s, ty, cs) Symbol s; Type ty; int cs; {
     Tree p;
     assert(ty);
				/* Create a tree which refers to the
				   closure symbol */
     if (cs) {
	  p = mktree(ICS + (isunsigned(ty) ? I : ttob(ty)),
		     ty, NULL, NULL);
	  p->u.sym = s;
     } else {
	  if (isarray(ty) || isfunc(ty)) {
	       /* CHECK */
/*	       assert(0);*/
	       p = mktree(ADDRLP, ty, NULL, NULL);
	       p->u.sym = s;
	  } else {
	       p = mktree(ADDRLP, ptr(ty), NULL, NULL);
	       p->u.sym = s;
	       p = mktree(INDIR + (isunsigned(ty) ? I : ttob(ty)), 
			  ty, p, NULL);
	  }
     }
     if (eval.unqtlevel)
				/* Mark this tree as inside ` expression */
	  p->scope = eval.ticklevel + 1;
				/* Return the resulting tree */
     return p;
}
/* 
   mktreefvar: given a tree p denoting a free variable not of spec type,
   marks p as a free variable and returns a tree for a closure field
   containing the value of p.
*/
static inline Tree mktreefvar(p, type) Tree p; Type type; {
     Symbol s;
     assert(!isspec(type));
				/* Add a symbol for p to the closure */
     s = closureaddtree(p, CETFV);
     return mktreecsym(s, type, 0);
}
/* 
   mktreespec: given a tree p of spec type, marks p for evaluation at 
   specification time, and returns a tree for a closure field containing
   the result of evaluating p.  The type of the resulting tree is the
   evaluation type of the spec denoted by p.
*/
static inline Tree mktreespec(p, type) Tree p; Type type; {
     Symbol s;
     int cs;
     assert(isspec(type));
				/* Add a symbol for p to the closure */
     cs = iscspec(type);
     s = closureaddtree(p, cs ? CEFCS : CEFVS);
     return mktreecsym(s, type->type, cs);
}

Tree tree(op, type, left, right)
int op; Type type; Tree left, right; {
     Tree p;

     p = mktree(op, type, left, right);
     if (eval.ticklevel && !eval.rtcp) {
				/* If parsing a ` expression (but not $) */
	  if (eval.unqtlevel) {
				/* If unquoting... */
	       int scope = 0;
				/* Propagate scope information */
	       if (left)
		    scope = LEFTKID(p)->scope;
	       if (right && RIGHTKID(p)->scope > scope)
		    scope = RIGHTKID(p)->scope;
	       p->scope = scope;

	  } else {
				/* If not unquoting... */
	       if (isspec(type))
				/* This is a spec expression, not unquoted */
		    p = mktreespec(p, type);
	  }
     }
     return p;
}

Tree texpr(f, tok, a) Tree (*f) ARGS((int)); int tok, a; {
     int save = where;
     Tree p;

     where = a;
     p = (*f)(tok);
     where = save;
     return p;
}
/* right - return (RIGHT, root(p), q) or just p/q if q/p==0;
   if ty==NULL, use q->type, or q->type/p->type */
Tree right(p, q) Tree p, q; {
     assert(p || q);
     if (p && q)
	  return tree(RIGHT, q->type, root(p), q);
     else if (p)
	  return p;
     else
	  return q;
}

Tree root(p) Tree p; {
     if (p == NULL)
	  return p;
     switch (generic(p->op)) {
     case COND: {
	  Tree q = p->kids[1];
	  assert(q && q->op == RIGHT);
	  if (p->u.sym && q->kids[0] && generic(q->kids[0]->op) == ASGN)
	       q->kids[0] = root(q->kids[0]->kids[1]);
	  else
	       q->kids[0] = root(q->kids[0]);
	  if (p->u.sym && q->kids[1] && generic(q->kids[1]->op) == ASGN)
	       q->kids[1] = root(q->kids[1]->kids[1]);
	  else
	       q->kids[1] = root(q->kids[1]);
	  p->u.sym = 0;
	  if (q->kids[0] == 0 && q->kids[1] == 0)
	       p = root(p->kids[0]);
     }
     break;
     case AND: case OR:
	  if ((p->kids[1] = root(p->kids[1])) == 0)
	       p = root(p->kids[0]);
	  break;
     case NOT:
	  return root(p->kids[0]);
     case RIGHT:
	  if (p->kids[1] == 0)
	       return root(p->kids[0]);
	  if (p->kids[0] && p->kids[0]->op == CALL+B
	      &&  p->kids[1] && p->kids[1]->op == INDIR+B)
	       /* avoid premature release of the CALL+B temporary */
	       return p->kids[0];
	  if (p->kids[0] && p->kids[0]->op == RIGHT
	      &&  p->kids[1] == p->kids[0]->kids[0])
	       /* de-construct e++ construction */
	       return p->kids[0]->kids[1];
	  /* fall thru */
     case EQ:  case NE:  case GT:   case GE:  case LE:  case LT: 
     case ADD: case SUB: case MUL:  case DIV: case MOD:
     case LSH: case RSH: case BAND: case BOR: case BXOR:
	  p = tree(RIGHT, p->type, root(p->kids[0]), root(p->kids[1]));
	  return p->kids[0] || p->kids[1] ? p : 0;
     case INDIR:
	  if (p->type->size == 0 && unqual(p->type) != voidtype)
	       warning("reference to `%t' elided\n", p->type);
	  if (isptr(p->kids[0]->type) && isvolatile(p->kids[0]->type->type))
	       warning("reference to `volatile %t' elided\n", p->type);
	  /* fall thru */
     case CVI: case CVF:  case CVD:   case CVU: case CVC: case CVS: case CVP:
     case NEG: case BCOM: case FIELD:
	  return root(p->kids[0]);
     case ADDRL: case ADRFL: case ADDRG:
     case ADDRF: case ADRFF: case CNST:
	  if (needconst)
	       return p;
	  return NULL;
     case ARG: case ASGN: case CALL: case JUMP: case LABEL:
     case DJUMP: case KLABEL: case KTEST: case KCOND:
     case ICS:
     case VAARG: case VASTART: case VAEND:
	  break;
     case TEXPR: case RTC:
	  return NULL;
     default: assert(0);
     }
     return p;
}

/* 
   tickfixtree: given an operator and 2 operands, performs whatever convertions
   to/from c/vspec type are necessary for unquoting.
   Requires: ticklevel && unqtlevel && !eval.rtcp 
*/
void tickfixtree(op, l, r) int op; Tree *l, *r; {
     Tree left = *l, right = *r;

				/* Must be in unquoted part of ` expression */
     assert (eval.ticklevel && eval.unqtlevel && !eval.rtcp);

     if (left && isspec(left->type) && generic(op) != ARG)
				/* If left exists and is a spec, it is being
				   used as an operand in some operation: we
				   must transform it into a closure reference*/
	  *l = mktreespec(left, left->type);

     if (right && isspec(right->type) && generic(op) != ARG)
				/* If right exists and is a spec, it is being
				   used as an operand in some operation: we
				   must transform it into a closure reference*/
	  *r = mktreespec(right, right->type);

     if (((left = *l)) && ((right = *r))
	 && generic(left->op) != CNST && generic(right->op) != CNST) {
				/* If one of left or right is in ` expression
				   and the other is not, must create closure
				   entry for the one that is not and
				   replace it with reference to closure entry*/
	  int ls = left->scope, rs = right->scope;
	  
	  if (rs <= eval.ticklevel && ls > eval.ticklevel)
				/* Add closure entry for right */
	       *r = mktreefvar(right, right->type);
	  else if (rs > eval.ticklevel && ls <= eval.ticklevel)
				/* Add closure entry for left */
	       *l = mktreefvar(left, left->type);
     }
}

char *opname(op) int op; {
     static char *opnames[] = {
	  "",
	  "CNST",
	  "ARG",
	  "ASGN",
	  "INDIR",
	  "CVC",
	  "CVD",
	  "CVF",
	  "CVI",
	  "CVP",
	  "CVS",
	  "CVU",
	  "NEG",
	  "CALL",
	  "LOAD",
	  "RET",
	  "ADDRG",
	  "ADDRF",
	  "ADDRL",
	  "ADD",
	  "SUB",
	  "LSH",
	  "MOD",
	  "RSH",
	  "BAND",
	  "BCOM",
	  "BOR",
	  "BXOR",
	  "DIV",
	  "MUL",
	  "EQ",
	  "GE",
	  "GT",
	  "LE",
	  "LT",
	  "NE",
	  "JUMP",
	  "LABEL",
	  "AND",
	  "NOT",
	  "OR",
	  "COND",
	  "RIGHT",
	  "FIELD",
	  "TEXPR",
	  "ICS",	/*45*/
	  "RTC",
	  "CRET",
	  "ADRFF",
	  "ADRFL",
	  "GETREG",	/*50*/
	  "PUTREG",
	  "NOTE",
	  "KCOND",
	  "KLABEL",
	  "KTEST",	/*55*/
	  "KJUMP",
	  "VAARG",
	  "VASTART",
	  "VAEND",
	  "VREG",	/*60*/
	  "FASGN",
	  "ADRVS",
	  "DJUMP",
	  "SELF"
     }, typenames[] = " FDCSIUPVBcv";
     char *name;

     if (opindex(op) > 0 && opindex(op) < NELEMS(opnames))
	  name = opnames[opindex(op)];
     else
	  name = stringd(opindex(op));
     if (op >= AND && op <= FIELD)
	  return name;
     else if (optype(op) > 0 && optype(op) < sizeof(typenames) - 1)
	  return stringf("%s%c", name, typenames[optype(op)]);
     else
	  return stringf("%s+%d", name, optype(op));
}

int nodeid(p) Tree p; {
     int i = 1;

     ids[nid].node = p;
     while (ids[i].node != p)
	  i++;
     if (i == nid)
	  ids[nid++].printed = 0;
     return i;
}

/* printed - return pointer to ids[id].printed */
int *printed(id) int id; {
     if (id)
	  return &ids[id].printed;
     nid = 1;
     return 0;
}

/* printtree - print tree p on fd */
void printtree(p, fd) Tree p; int fd; {
     (void)printed(0);
     printtree1(p, fd, 1);
}

/* printtree1 - recursively print tree p */
static void printtree1(p, fd, lev) Tree p; int fd, lev; {
     int i;
     static char blanks[] = "                                         ";

     if (p == 0 || *printed(i = nodeid(p)))
	  return;
     fprint(fd, "#%d%s%s", i, &"   "[i < 10 ? 0 : i < 100 ? 1 : 2],
	    &blanks[sizeof blanks - lev]);
     fprint(fd, "%s %t", opname(p->op), p->type);
     *printed(i) = 1;
     for (i = 0; i < NELEMS(p->kids); i++)
	  if (p->kids[i])
	       fprint(fd, " #%d", nodeid(p->kids[i]));
     if (p->op == FIELD && p->u.field)
	  fprint(fd, " %s %d..%d", p->u.field->name,
		 fieldsize(p->u.field) + fieldright(p->u.field), 
		 fieldright(p->u.field));
     else if (generic(p->op) == CNST)
	  fprint(fd, " %s", vtoa(p->type, p->u.v));
     else if (p->u.sym)
	  fprint(fd, " %s", p->u.sym->name);
     if (p->node)
	  fprint(fd, " node=0x%x", p->node);
     fprint(fd, "\n");
     for (i = 0; i < NELEMS(p->kids); i++)
	  printtree1(p->kids[i], fd, lev + 1);
}

char *unparsetree(Tree t) {
     static int calldepth = 0;
     Tree k;
     Type ty;

     assert(calldepth >= 0);
     if (t == NULL)
	  return NULL;

     switch (generic(t->op)) {
     case CNST:
	  switch (optype(t->op)) {
	  case C: return stringf("%d", t->u.v.uc);
	  case D: return stringf("%e", t->u.v.d);
	  case F: return stringf("%f", t->u.v.f);
	  case I: return stringf("%d", t->u.v.i);
	  case P: return stringf("%x", t->u.v.p);
	  case S: return stringf("%d", t->u.v.us);
	  case U: return stringf("%d", t->u.v.u);
	  default: assert(0);
	  }
     case ARG:
	  if (t->kids[1])
	       return stringf("%s, %s", unparsetree(t->kids[1]),
			      unparsetree(t->kids[0]));
	  else
	       return unparsetree(t->kids[0]);
     case ASGN:
	  assert(t->kids[0] && t->kids[1]);
	  if (generic((k = t->kids[0])->op) == ADDRL
	      || generic(k->op) == ADDRF || generic(k->op) == ADDRG) {
	       assert(k->u.sym);
	       return stringf("(%s = %s)", k->u.sym->name,
			      unparsetree(t->kids[1]));
	  } else if (isptr(k->type))
	       return stringf("(*((%s)%s) = (%s)%s)", 
			      typestring(0, k->type, ""), unparsetree(k), 
			      typestring(0, deref(k->type), ""), 
			      unparsetree(t->kids[1]));
	  else
	       return stringf("(%s = %s)",
			      unparsetree(k), unparsetree(t->kids[1]));
     case INDIR: {
	  char *pre, *post;
	  if (t->op == INDIRP && !(isptr(t->type) && isfunc(t->type->type))) {
	       pre = "((char *)"; post = ")";
	  } else
	       pre = post = " ";
	  if (generic(((k = t->kids[0]))->op) == ADDRL
	      || generic(k->op) == ADDRF || generic(k->op) == ADDRG) {
	       assert(k->u.sym);
	       if (isarray(k->u.sym->type))
		    return stringf("%s(*(%s)%s)%s", pre,
				   typestring(0, ptr(t->type), ""),
				   k->u.sym->name, post);
	       else if (isstruct(k->u.sym->type))
		    return stringf("%s(*(%s)&%s)%s", pre,
				   typestring(0, ptr(t->type), ""),
				   k->u.sym->name, post);
	       else if (isptr(t->type) && !isfunc(t->type->type))
		    return stringf("%s((unsigned)%s)%s", pre, k->u.sym->name,
				   post);
	       else
		    return stringf("%s%s%s", pre, k->u.sym->name, post);
	  } else
	       return stringf("%s(*(%s)%s)%s", pre, 
			      typestring(0, ptr(t->type), ""),
			      unparsetree(k), post);
     }
     case CVC: case CVD: case CVF: case CVI: case CVP: case CVS: case CVU:
	  switch (optype(t->op)) {
	  case C: return stringf("((char)%s)", unparsetree(t->kids[0]));
	  case S: return stringf("((short)%s)", unparsetree(t->kids[0]));
	  case I: return stringf("((int)%s)", unparsetree(t->kids[0]));
	  case U: return stringf("((unsigned)%s)", unparsetree(t->kids[0]));
	  case F: return stringf("((float)%s)", unparsetree(t->kids[0]));
	  case D: return stringf("((double)%s)", unparsetree(t->kids[0]));
	  case P: return stringf("((void *)%s)", unparsetree(t->kids[0]));
	  default: assert(0);
	  }
     case NEG:
	  return stringf("(-%s)", unparsetree(t->kids[0]));
     case CALL:
	  k = t->kids[0];	  assert(k);
	  if (generic(k->op) == RIGHT) {
	       char * str;
	       assert(k->kids[0] && k->kids[1]);
	       ++ calldepth;
	       str = unparsetree(k->kids[0]);
	       if (generic(k->kids[1]->op) == ADDRL
		   || generic(k->kids[1]->op) == ADDRF
		   || generic(k->kids[1]->op) == ADDRG) {
		    assert(k->kids[1]->u.sym);
		    str = stringf("%s(%s)", k->kids[1]->u.sym->name, str);
	       } else {
		    str = stringf("%s(%s)", unparsetree(k->kids[1]), str);
	       }
	       -- calldepth;
	       return str;
	  } else
	       return stringf("%s()", unparsetree(k));
     case ADDRG: case ADDRF: case ADDRL:
	  assert(t->u.sym);
	  ty = t->u.sym->type;
	  if (isarray(ty))
	       return stringf("((unsigned) %s)", t->u.sym->name);
	  else if (isfunc(ty))
	       return string(t->u.sym->name);
	  else
	       return stringf("((unsigned) &%s)", t->u.sym->name);
     case ADD:
	  assert(t->kids[0] && t->kids[1]);
	  return stringf("(%s + %s)", unparsetree(t->kids[0]),
			 unparsetree(t->kids[1]));
     case SUB:
	  assert(t->kids[0] && t->kids[1]);
	  return stringf("(%s - %s)", unparsetree(t->kids[0]),
			 unparsetree(t->kids[1]));
     case LSH:
	  assert(t->kids[0] && t->kids[1]);
	  return stringf("(%s << %s)", unparsetree(t->kids[0]),
			 unparsetree(t->kids[1]));
     case MOD:
	  assert(t->kids[0] && t->kids[1]);
	  return stringf("(%s % %s)", unparsetree(t->kids[0]),
			 unparsetree(t->kids[1]));
     case RSH:
	  assert(t->kids[0] && t->kids[1]);
	  return stringf("(%s >> %s)", unparsetree(t->kids[0]),
			 unparsetree(t->kids[1]));
     case BAND:
	  assert(t->kids[0] && t->kids[1]);
	  return stringf("(%s & %s)", unparsetree(t->kids[0]),
			 unparsetree(t->kids[1]));
     case BCOM:
	  assert(t->kids[0]);
	  return stringf("(~%s)", unparsetree(t->kids[0]));
     case BOR:
	  assert(t->kids[0] && t->kids[1]);
	  return stringf("(%s | %s)", unparsetree(t->kids[0]),
			 unparsetree(t->kids[1]));
     case BXOR:
	  assert(t->kids[0] && t->kids[1]);
	  return stringf("(%s ^ %s)", unparsetree(t->kids[0]),
			 unparsetree(t->kids[1]));
     case DIV:
	  assert(t->kids[0] && t->kids[1]);
	  return stringf("(%s / %s)", unparsetree(t->kids[0]),
			 unparsetree(t->kids[1]));
     case MUL:
	  assert(t->kids[0] && t->kids[1]);
	  return stringf("(%s * %s)", unparsetree(t->kids[0]),
			 unparsetree(t->kids[1]));
     case EQ:
	  assert(t->kids[0] && t->kids[1]);
	  return stringf("(%s == %s)", unparsetree(t->kids[0]),
			 unparsetree(t->kids[1]));
     case GE:
	  assert(t->kids[0] && t->kids[1]);
	  return stringf("(%s >= %s)", unparsetree(t->kids[0]),
			 unparsetree(t->kids[1]));
     case GT:
	  assert(t->kids[0] && t->kids[1]);
	  return stringf("(%s > %s)", unparsetree(t->kids[0]),
			 unparsetree(t->kids[1]));
     case LE:
	  assert(t->kids[0] && t->kids[1]);
	  return stringf("(%s <= %s)", unparsetree(t->kids[0]),
			 unparsetree(t->kids[1]));
     case LT:
	  assert(t->kids[0] && t->kids[1]);
	  return stringf("(%s < %s)", unparsetree(t->kids[0]),
			 unparsetree(t->kids[1]));
     case NE:
	  assert(t->kids[0] && t->kids[1]);
	  return stringf("(%s != %s)", unparsetree(t->kids[0]),
			 unparsetree(t->kids[1]));
     case AND:
	  assert(t->kids[0] && t->kids[1]);
	  return stringf("(%s && %s)", unparsetree(t->kids[0]),
			 unparsetree(t->kids[1]));
     case NOT:
	  assert(t->kids[0]);
	  return stringf("(! %s)", unparsetree(t->kids[0]));
     case OR:
	  assert(t->kids[0] && t->kids[1]);
	  return stringf("(%s || %s)", unparsetree(t->kids[0]),
			 unparsetree(t->kids[1]));
     case COND: {
	  Tree kk0, kk1;
	  k = t->kids[1];
	  assert(t->kids[0] && k && k->op==RIGHT && k->kids[0] && k->kids[1]);
	  if (t->u.sym) {	/* Tree contains assignments to an (unnamed)
				   temporary.  Must strip out the assignments
				   to avoid having this temporary appear in
				   the C code.  See condtree().*/
	       assert(generic(k->kids[0]->op) == ASGN && k->kids[0]->kids[1] &&
		      generic(k->kids[1]->op) == ASGN && k->kids[1]->kids[1]);
	       kk0 = k->kids[0]->kids[1];
	       kk1 = k->kids[1]->kids[1];
	  } else {
	       kk0 = k->kids[0];
	       kk1 = k->kids[1];
	  }
				/* Emit the trees */
	  return stringf("(%s ? %s : %s)", unparsetree(t->kids[0]), 
			 unparsetree(kk0), unparsetree(kk1));
     }
     break;
     case RIGHT:
	  if (t->kids[0] && t->kids[1]) {
	       Tree k1 = t->kids[1], ka, kb;
	       k = t->kids[0];
	       if (calldepth && generic(k1->op) == ARG) {
		    /* RIGHT unnests a nested call */
		    return unparsetree(k1);
	       } else if (k->op == RIGHT && generic(k1->op) == INDIR
			  && ((ka = k->kids[0])) && ((kb = k->kids[1])) 
			  && generic(kb->op) == ASGN 
			  && generic(ka->op) == INDIR
			  && ka->kids[0] == kb->kids[0]) {
		    /* RIGHT denotes postincrement/decrement */
		    assert(kb->kids[1]);
		    if (generic(kb->kids[1]->op) == ADD)
			 return stringf("(%s++)", unparsetree(k1));
		    else if (generic(kb->kids[1]->op) == SUB)
			 return stringf("(%s--)", unparsetree(k1));
		    assert(0);
	       } else if (k->op == RIGHT && k1->op == FIELD
			  && ((ka = k->kids[0])) && ((kb = k->kids[1])) 
			  && generic(kb->op) == ASGN 
			  && ka->op == FIELD && ka == kb->kids[0]) {
		    /* RIGHT denotes postincrement/decrement of FIELD */
		    Tree kbk0;
		    assert(kb->kids[1] && kb->kids[1]->kids[0]);
		    kbk0 = kb->kids[1]->kids[0];
		    if (generic(kbk0->op) == ADD)
			 return stringf("(%s++)", unparsetree(k1));
		    else if (generic(kbk0->op) == SUB)
			 return stringf("(%s--)", unparsetree(k1));
		    assert(0);
	       } else
		    return stringf("(%s, %s)", unparsetree(k), 
				   unparsetree(k1));
	  } else if (t->kids[0])
	       return unparsetree(t->kids[0]);
	  else			/* encountered error in source code */
	       return "ERROR";
     case FIELD:
	  return unparsetree(t->kids[0]);
     case VAARG:
	  assert(cbe.have);
	  assert(t->kids[0]);
	  return stringf("va_arg(%s, %s)", unparsetree(t->kids[0]),
			 typestring(0, t->type, ""));
	  break;
     case VASTART:
	  assert(cbe.have);
	  assert(t->kids[0] && t->kids[1]);
	  return stringf("va_start(%s, %s)", 
			 unparsetree(t->kids[0]), unparsetree(t->kids[1]));
	  break;
     case VAEND:
	  assert(cbe.have);
	  assert(t->kids[0]);
	  return stringf("va_end(%s)", unparsetree(t->kids[0]));
	  break;
     case RTC:
	  return stringf("((unsigned)%s)", t->u.sym->name);
     case TEXPR:		/* `-expression; need to unparse for push() */
	  return string(t->u.sym->name);
     case ADRFF: case ADRFL:
	  return mangle(t->u.sym->name);
				/* other dcg ops; need never be unparsed */
     case ICS: case CRET: 
				/* random others; never unparse */
     case JUMP: case LABEL: case LOAD: case RET:
     default:
	  assert(0);
     }
     return NULL;
}

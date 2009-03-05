#include "c.h"

#ifndef __TCC__	/* To avoid duplicate enum error when tcc compiles itself */
enum { TC_REGISTER=1<<5,	/* Must be same as vcode/icode values */
       TC_MEMORY=1<<7
};
#endif

extern int fordef;

static char prec[] = {
#define xx(a,b,c,d,e,f,g) c,
#define yy(a,b,c,d,e,f,g) c,
#include "token.h"
};
static int oper[] = {
#define xx(a,b,c,d,e,f,g) d,
#define yy(a,b,c,d,e,f,g) d,
#include "token.h"
};

float refinc = 1.0;
static Code tickexpr1 ARGS((void));
static Tree expr2 ARGS((void));
static Tree expr3 ARGS((int));
static Tree nullcheck ARGS((Tree));
static Tree postfix ARGS((Tree));
static Tree primary ARGS((void));
static Type super ARGS((Type ty));

Tree expr(tok) int tok; {
     static char stop[] = { IF, ID, '}', 0 };
     Tree p = expr1(0);

     while (t == ',') {
	  Tree q;
	  t = gettok();
	  q = pointer(expr1(0));
	  p = tree(RIGHT, q->type, root(value(p)), q);
     }
     if (tok)	
	  test(tok, stop);
     return p;
}
Tree expr0 (int tok) {
     int ot = t;
     Tree tp = expr(tok);
     tp = root(tp);
     if (tp && generic(tp->op) == ICS && ot != TAT)
	  error("spec used as expression-statement must be preceded by '@'\n");
     return tp;
}
Tree expr1 (int tok) {
     static char stop[] = { IF, ID, 0 };
     Tree p = expr2();

     if (t == '='
	 || (prec[t] >=  6 && prec[t] <=  8)
	 || (prec[t] >= 11 && prec[t] <= 13)) {
	  int op = t;

	  t = gettok();
	  if (oper[op] == ASGN)
	       p = asgntree(ASGN, p, value(expr1(0)));
	  else {
	       expect('=');
	       p = incr(op, p, expr1(0));
	  }
     }
     if (tok)	
	  test(tok, stop);
     return p;
}
Tree incr(op, v, e) int op; Tree v, e; {
     return asgntree(ASGN, v, (*optree[op])(oper[op], v, e));
}
static Tree expr2() {
     Tree p = expr3(4);

     if (t == '?') {
	  Tree l, r;
	  Coordinate pts[2];
	  if (Aflag > 1 && isfunc(p->type))
	       warning("%s used in a conditional expression\n",
		       funcname(p));
	  if (! isrtcop(p->op))
	       p = pointer(p);
	  t = gettok();
	  pts[0] = src;
	  l = pointer(expr(':'));
	  pts[1] = src;
	  r = pointer(expr2());
	  p = condtree(p, l, r);
	  if (events.points)
	       if (p->op == COND) {
		    Symbol t1 = p->u.sym;
		    assert(p->kids[1]);
		    assert(p->kids[1]->op == RIGHT);
		    apply(events.points, &pts[0], &p->kids[1]->kids[0]);
		    apply(events.points, &pts[1], &p->kids[1]->kids[1]);
		    p = tree(COND, p->type, p->kids[0],
			     tree(RIGHT, p->type,
				  p->kids[1]->kids[0],
				  p->kids[1]->kids[1]));
		    p->u.sym = t1;
	       }
     }
     return p;
}
Tree value(p) Tree p; {
     int op = generic(rightkid(p)->op);

     if (op==AND || op==OR || op==NOT || op==EQ || op==NE
	 ||  op== LE || op==LT || op== GE || op==GT)
	  p = condtree(p, consttree(1, inttype),
		       consttree(0, inttype));
     return p;
}
static Tree expr3(k) int k; {
     int k1;

     Tree p = unary();

     for (k1 = prec[t]; k1 >= k; k1--)
	  while (prec[t] == k1 && *cp != '=') {
	       Tree r;		/*, l; */
	       Coordinate pt;
	       int op = t;
	       t = gettok();
	       pt = src;
	       p = pointer(p);
	       if (op == ANDAND || op == OROR) {
		    r = pointer(expr3(k1));
		    if (events.points)
			 apply(events.points, &pt, &r);
	       } else
		    r = pointer(expr3(k1 + 1));
	       p = (*optree[op])(oper[op], p, r); 
	  }
     return p;
}
Tree unary() {
     Tree p;
     Type ty;

     switch (t) {
     case '*':    
	  t = gettok(); p = unary(); 
	  p = pointer(p);
	  if (isptr(p->type)
	      && (isfunc(p->type->type) || isarray(p->type->type)))
	       p = retype(p, p->type->type);
	  else {
	       if (YYnull)
		    p = nullcheck(p);
	       p = rvalue(p);
	  } 
	  break;
     case '&':   
	  t = gettok(); p = unary(); 
	  if (isarray(p->type) || isfunc(p->type))
	       p = retype(p, ptr(p->type));
	  else
	       p = lvalue(p);
	  if (isaddrop(p->op) && p->u.sym->sclass == REGISTER)
	       error("invalid operand of unary &; `%s' is declared register\n",
		     p->u.sym->name);
	  else if (isrtcop(p->op))
	       error("invalid operand of unary &: runtime constant\n");
	  else if (isaddrop(p->op))
	       p->u.sym->addressed = 1;
	  break;
     case TTICK:
	  if (eval.ticklevel)
	       error("invalid nested ` expression\n");
	  if (cbe.have)
	       cbacksuspend();
	  t = gettok();
	  p = tickexpr();
	  if (cbe.have) {
	       cbackrestart();
	       if (cbe.flush)	/* False in the case of compile(`{...}, ...) */
		    cbufputc(t);
	  }
	  break;
     case TDLR:
	  if (!eval.ticklevel)
	       error("`$' applied outside ` expression\n");
	  if (eval.unqtlevel)
	       error("'$' applied within unquoted region\n");
	  if (eval.rtcp)
	       error("invalid nested `$' expression\n");
	  eval.rtcp = 1;
	  t = gettok();
	  p = unary();
	  eval.rtcp = 0;
	  rtctree(&p);
	  break;
     case TAT:
	  if (!eval.ticklevel)
	       error("`@' applied outside ` expression\n");
	  if (eval.rtcp)
	       error("`@' applied within runtime constant expression\n");
	  t = gettok();
	  p = unary();
				/* Check that operand has spec type; cannot use
				   isspec() because conversion has occurred */
	  if (! (generic(p->op) == ICS
		 || (p->kids[0] && p->kids[0]->u.sym
		     && p->kids[0]->u.sym->specp)))
	       error("operand of '@' has illegal type '%t'\n",p->type);
	  break;
     case '+':    
	  t = gettok(); p = unary(); 
	  p = pointer(p);
	  if (isarith(p->type))
	       p = cast(p, promote(p->type));
	  else
	       typeerror(ADD, p, NULL);  
	  break;
     case '-':    
	  t = gettok(); p = unary(); 
	  p = pointer(p);
	  if (isarith(p->type)) {
	       p = cast(p, promote(p->type));
	       if (isunsigned(p->type)) {
		    warning("unsigned operand of unary -\n");
		    p = simplify(NEG, inttype, cast(p, inttype), NULL);
		    p = cast(p, unsignedtype);
	       } else
		    p = simplify(NEG, p->type, p, NULL);
	  } else
	       typeerror(SUB, p, NULL); 
	  break;
     case '~':    
	  t = gettok(); p = unary(); 
	  p = pointer(p);
	  if (isint(p->type)) {
	       Type ty = promote(p->type);
	       p = simplify(BCOM, ty, cast(p, ty), NULL);
	  } else
	       typeerror(BCOM, p, NULL);  
	  break;
     case '!':    t = gettok(); 
	  p = unary();
	  p = pointer(p);
	  if (isscalar(p->type))
	       p = simplify(NOT, inttype, cond(p), NULL);
	  else
	       typeerror(NOT, p, NULL); 
	  break;
     case INCR:   
	  t = gettok(); 
	  p = unary();
	  p = incr(INCR, pointer(p), consttree(1, inttype)); 
	  break;
     case DECR:   
	  t = gettok(); p = unary();
	  p = incr(DECR, pointer(p), consttree(1, inttype)); 
	  break;
     case TLOCAL: {
	  int spec = 0;
	  if (cbe.have) {
	       assert(cbe.flush);
	       cbufclear(); cbufflushoff();
	  }

	  t = gettok(); expect('(');
	  
	  if (t == AUTO || t == REGISTER) {
	       if (t == REGISTER)
		    spec = TC_REGISTER;
	       t = gettok();
	  }
	  if (istypename(t, tsym)) {
	       ty = typename();
	  } else {
	       error("argument to `local' must be a type name\n");
	       ty = inttype;
	  }
	  expect(')');
	  
	  if (isvolatile(ty)) {
	       if (spec == TC_REGISTER)
		    error("register declaration ignored for vspec of %s\n",
			  typestring(0, ty, ""));
	       spec = TC_MEMORY;
	  }
	  p = calllocaltree(ty, spec);

	  if (cbe.have) {
	       cbufflushon();
	       cbufflush(unparsetree(p));
	       cbufputc(t);
	  }
     }
     break;
     case TPARAM:     {
	  Tree f, args;

	  if (cbe.have) {
	       assert(cbe.flush);
	       cbufclear(); cbufflushoff();
	  }

	  t = gettok(); expect('(');

	  if (istypename(t, tsym)) {
	       ty = typename();
	  } else {
	       error("argument 1 to `param' must be a type name\n");
	       ty = inttype;
	  }
	  expect(',');
	  p = unary(); p = pointer(p);
	  if (! isint(p->type)) {
	       error("argument 2 to `param' must be an integer expression\n");
	       p = consttree(0, unsignedtype);
	  }
	  expect(')');

	  f = idtree(findtcsym(eval.fc ? tcname.Paramf : tcname.Param, 
			       globals));
	  args = tree(ARGI, p->type, p,
		      tree(ARGI, unsignedtype,
			   consttree(ttov(ty), unsignedtype), NULL));
	  p = calltree(f, mkvspec(ty), args, NULL);

	  if (cbe.have) {
	       cbufflushon();
	       cbufflush(unparsetree(p));
	       cbufputc(t);
	  }
     }
	  break;
     case TSC:
     case TFC: {
	  int kind = t;
	  if (eval.ticklevel)
	       error("`%s' invoked inside ` expression\n",
		     kind == TSC ? "tc_slow_compile" : "tc_fast_compile");
	  if (cbe.have) {
	       assert(cbe.flush);
	       cbufclear(); cbufflushoff();
	  }
	  t = gettok();
	  if (t == '(') {	/* Parens are optional */
	       t = gettok(); expect(')');
	  }
	  p = NULL;
	  if (cbe.have) {
	       cbufflushon(); cbufflush(0);
	  }
	  if (kind == TFC) {
	       dIR = binding->Vir; eval.fc = 1;
	  } else {
	       dIR = binding->Iir; eval.fc = 0;
	  }
     }
     break;
     case TCOMPILE:      {
	  Tree f, arg2, args;

	  if (eval.ticklevel)
	       error("`compile' invoked inside ` expression\n");

	  if (cbe.have) {
	       assert(cbe.flush);
	       cbufclear(); 
	       cbufflush(eval.fc ? tcname.Compilef : tcname.Compile);
	  }

	  t = gettok(); expect('(');

	  p = unary();
	  if (! (iscspec(p->type) && p->type->type 
		 && p->type->type == voidtype)) {
	       p = consttree(0, unsignedtype);
	       error("argument 1 to `compile' must have type void cspec\n");
	  }
	  args = tree(ARGP, ptr(findtcsym(tcname.Cspec_t, identifiers)->type),
		      p, NULL);

	  expect(',');
	  if (cbe.have) {
	       cbufclear();
	       cbufflushoff();
	  }
	  if (istypename(t, tsym)) {
	       ty = typename();
	  } else {
	       error("argument 2 to `compile' must be a type name\n");
	       ty = inttype;
	  }
	  expect(')');

	  f = idtree(findtcsym(eval.fc ? tcname.Compilef : tcname.Compile, 
			       globals));
	  arg2 = consttree(ttov(ty), unsignedtype);
	  args = tree(ARGI, unsignedtype, arg2, args);
	  p = calltree(f, ptr(func(ty, NULL, NULL, 1)), args, NULL);

	  if (cbe.have) {
	       cbufflushon();
	       cbufflush(unparsetree(arg2)); cbufputc(')');
	       cbufputc(t);
	  }
     }
	  break;
     case TDECOMPILE:      {
	  Tree f, args;

	  if (eval.ticklevel)
	       error("`decompile' invoked inside ` expression\n");

	  if (cbe.have) {
	       assert(cbe.flush);
	       cbufclear(); 
	       cbufflush(tcname.Decompile);
	  }
	  t = gettok(); expect('(');

	  p = unary();
	  if (! (isptr(p->type) && p->type->type && isfunc(p->type->type))) {
	       p = consttree(0, unsignedtype);
	       error("argument to `decompile' must be a function pointer\n");
	  }
	  args = tree(ARGP, ptr(findtcsym(tcname.Code_t, identifiers)->type),
		      p, NULL);
	  expect(')');

	  f = idtree(findtcsym(tcname.Decompile, globals));
	  p = calltree(f, voidtype, args, NULL);

     }
	  break;
     case TPUSHI: {
	  Tree f;
	  if (eval.ticklevel)
	       error("`push_init' invoked inside ` expression\n");

	  if (cbe.have) {
	       assert(cbe.flush);
	       cbufclear(); cbufflushoff();
	  }

	  t = gettok();
	  if (t == '(') {	/* Parens are optional */
	       t = gettok(); expect(')');
	  }

	  f = idtree(findtcsym(tcname.Pushi, globals));
	  p = calltree(f, mkcspec(voidtype), NULL, NULL);

	  if (cbe.have) {
	       cbufflushon();
	       cbufflush(unparsetree(p));
	       cbufputc(t);
	  }
     }
	  break;
     case TPUSH: {
	  Tree f, args;
	  if (eval.ticklevel)
	       error("`push' invoked inside ` expression\n");

	  if (cbe.have) {
	       assert(cbe.flush);
	       cbufclear();
	       cbufflushoff();
	  }

	  t = gettok(); expect('(');
	  
	  p = unary();
	  if (! (iscspec(p->type) && p->type->type 
		 && p->type->type == voidtype)) {
	       p = consttree(0, unsignedtype);
	       error("argument 1 to `push' must have type void cspec\n");
	  }
	  args = tree(ARGP, 
		      ptr(findtcsym(tcname.Dcall_t, identifiers)->type),
		      p, NULL);
	  
	  expect(',');

	  p = unary();
	  if (! iscspec(p->type)) {
	       ty = inttype;
	       p = consttree(0, unsignedtype);
	       error("argument 2 to `push' must have type cspec\n");
	  } else
	       ty = p->type->type;
	  expect(')');
	  
	  args = tree(ARGP, 
		      ptr(findtcsym(tcname.Cspec_t, identifiers)->type), p,
		      tree(ARGI, 
			   unsignedtype, consttree(ttov(ty), unsignedtype),
			   args));
	  f = idtree(findtcsym(tcname.Push, globals));
	  p = calltree(f, voidtype, args, NULL);

	  if (cbe.have) {
	       cbufflushon();
	       cbufflush(";\n");
	       cbufflush(unparsetree(p));
	       cbufputc(t);
	  }
     }
	  break;
     case TARG: {
	  Tree f, args;
	  if (eval.ticklevel)
	       error("`arg' invoked inside ` expression\n");

	  if (cbe.have) {
	       assert(cbe.flush);
	       cbufclear();
	       cbufflushoff();
	  }

	  t = gettok(); expect('(');
	  
	  p = unary();		/* Argument 1 */
	  if (! isint(p->type)) {
	       p = consttree(0, unsignedtype);
	       error("argument 1 to `arg' must be an integer\n");
	  }
	  args = tree(ARGI, p->type, p, NULL);
	  expect(',');

	  p = unary();		/* Argument 2 */
	  if (! (iscspec(p->type) && p->type->type 
		 && p->type->type == voidtype)) {
	       p = consttree(0, unsignedtype);
	       error("argument 2 to `arg' must have type void cspec\n");
	  }
	  args = tree(ARGP, 
		      ptr(findtcsym(tcname.Dcall_t, identifiers)->type),
		      p, args);	  
	  expect(',');

	  p = unary();		/* Argument 3 */
	  if (! iscspec(p->type)) {
	       ty = inttype;
	       p = consttree(0, unsignedtype);
	       error("argument 3 to `arg' must have type cspec\n");
	  } else
	       ty = p->type->type;
	  expect(')');
	  
	  args = tree(ARGP, 
		      ptr(findtcsym(tcname.Cspec_t, identifiers)->type), p,
		      tree(ARGI, 
			   unsignedtype, consttree(ttov(ty), unsignedtype),
			   args));
	  f = idtree(findtcsym(tcname.Arg, globals));
	  p = calltree(f, voidtype, args, NULL);

	  if (cbe.have) {
	       cbufflushon();
	       cbufflush(";\n");
	       cbufflush(unparsetree(p));
	       cbufputc(t);
	  }
     }
	  break;
     case TJUMP: {
	  Symbol s;
	  if (cbe.have && !eval.ticklevel) {
	       cbacksuspend();
	  }
	  t = gettok();
	  p = unary();		/* parens are optional */
	  if (!(iscspec(p->type) && p->type->type == voidtype)
	      && !(eval.ticklevel && p->op == ICSV)) {
	       p = consttree(0, unsignedtype);
	       error("argument to `jump' must have type void cspec\n");
	       break;
	  }
	  if (eval.ticklevel) {
	       s = p->u.sym;
	       p = tree(DJUMP, voidtype, NULL, NULL);
	       p->u.sym = s;
	       installcopy(s, &eval.targ, 0, FUNC);
	  } else {
	       initevalinfo();
	       s = install(mkname(NAMESPEC), &eval.fcs, 0, FUNC);
	       s->u.x.t = p;
	       s->type = p->type;
	       s->specp = 1;
	       installcopy(s, &eval.targ, 0, FUNC);
	       s = defclosure
		    (declnamedclosure
		     (findtcsym(eval.fc ? tcname.Jumpf : tcname.Jump, 
				identifiers), NULL, s));
	       storeevalinfo(NULL, NULL, NULL);
	       p = ticktree(s, voidtype);
	       if (cbe.have) {
		    cbackrestart();
		    if (cbe.flush)
			 cbufputc(t);
	       }
	  }
     }
	  break;
     case TLABEL: {
	  Symbol s;
	  if (cbe.have && !eval.ticklevel) {
	       cbacksuspend();
	  }
	  t = gettok();
	  if (t == '(') {	/* Parens are optional */
	       t = gettok(); expect(')');
	  }
	  p = NULL;
	  if (eval.ticklevel) {
	       warning("`label' invoked inside `-expression: "
		       "probably not what you want\n");
	  } else {
	       initevalinfo();
	       s = defclosure
		    (declnamedclosure
		     (findtcsym(eval.fc ? tcname.Labelf : tcname.Label, 
				identifiers), NULL, NULL));
	       storeevalinfo(NULL, NULL, NULL);
	       p = ticktree(s, voidtype);
	       if (cbe.have) {
		    cbackrestart();
		    if (cbe.flush)
			 cbufputc(t);
	       }
	  }
     }

	  break;
     case TSELFD: ty = doubletype; goto doself;
     case TSELFF: ty = floattype; goto doself;
     case TSELFI: ty = inttype; goto doself;
     case TSELFV: ty = voidtype; goto doself;
     doself:
	  t = gettok();
	  if (! eval.ticklevel)
	       error("invalid use of `self` outside ` expression\n");
	  p = postfix(tree(SELF+P, func(ty, NULL, NULL, 1), 0, 0));
	  break;
     case SIZEOF: {
	  Type ty;
	  t = gettok(); 
	  p = NULL;
	  if (t == '(') {
	       t = gettok();
	       if (istypename(t, tsym)) {
		    ty = typename();
		    expect(')');
	       } else {
		    p = postfix(expr(')'));
		    ty = p->type;
	       }
	  } else {
	       p = unary();
	       ty = p->type;
	  }
	  assert(ty);
	  if (isfunc(ty) || ty->size == 0)
	       error("invalid type argument `%t' to `sizeof'\n", ty);
	  else if (p && rightkid(p)->op == FIELD)
	       error("`sizeof' applied to a bit field\n");
	  p = consttree(ty->size, unsignedtype); 
     } 
	  break;
     case '(':
	  t = gettok();
	  if (istypename(t, tsym)) {
	       Type ty, ty1 = typename(), pty;
	       expect(')');
	       ty = unqual(ty1);
	       if (isenum(ty)) {
		    Type ty2 = ty->type;
		    if (isconst(ty1))
			 ty2 = qual(CONST, ty2);
		    if (isvolatile(ty1))
			 ty2 = qual(VOLATILE, ty2);
		    ty1 = ty2;
		    ty = ty->type;
	       }
	       if (Xflag && isstruct(ty) && t == '{') {
		    Symbol t1 = temporary(AUTO, ty, level);
		    if (Aflag >= 2)
			 warning("non-ANSI constructor for `%t'\n", ty);
		    p = tree(RIGHT, ty1, structexp(ty, t1), idtree(t1));
		    break;
	       }
	       p = unary(); 
	       p = pointer(p);
	       pty = p->type;
	       if (isenum(pty))
		    pty = pty->type;
	       if (isarith(pty) && isarith(ty)
		   ||  isptr(pty)   && isptr(ty))
		    p = cast(p, ty);
	       else if (isptr(pty) && isint(ty)
			|| isint(pty) && isptr(ty)) {
		    if (Aflag >= 1 && ty->size < pty->size && !eval.tCompiling)
			 warning("conversion from `%t' to `%t' is "
				 "compiler dependent\n", p->type, ty);
		    p = cast(p, ty);
	       } else if (isvspec(pty) && isvspec(ty)) {
		    p = cast(p, ty);
	       } else if (eval.tCompiling) {
		    /* typedefs are stored in 'identifiers' table... */
		    Symbol vssym = lookup(tcname.Local_t, identifiers); 
		    Symbol cdsym = lookup(tcname.Closure_t, identifiers);
		    if (ty == vssym->type && isvspec(pty)
			|| ty == cdsym->type && iscspec(pty))
			 debug(warning("coercing spec to internal "
				       "representation\n"));
		    else if (ty != voidtype)
			 fatal("unary",
			       stringf("illegal cast from '%t' to '%t' "
				       "in templates\n", pty, ty),
			       0);
	       } else if (ty != voidtype) {
		    error("cast from `%t' to `%t' is illegal\n",
			  p->type, ty1);
		    ty1 = inttype;
	       }
	       p = retype(p, ty1);
	       if (generic(p->op) == INDIR)
		    p = tree(RIGHT, ty, NULL, p);
	  } else {
	       p = postfix(expr(')'));
	  }
	  break;
     case TVAARG: 
	  if (! cbe.have)
	       goto postfixprimary;
	  {
	       Type ty;
	       t = gettok(); expect('(');
	       p = unary();  expect(',');
	       if (p->type != valisttype)
		    error("type error: incompatible type passed to va_arg\n");
	       if (istypename(t, tsym)) {
		    ty = typename();
	       } else {
		    error("argument 2 to va_arg must be a type name\n");
		    ty = inttype;
	       }
	       expect(')');
	       p = tree(VAARG, ty, p, NULL);
	  }
	  break;
     case TVASTART: {
	  Tree q;
	  if (! cbe.have)
	       goto postfixprimary;
	  t = gettok(); expect('(');
	  p = unary();  expect(',');
	  if (p->type != valisttype)
	       error("type error: incompatible type passed to va_start\n");
	  q = unary();  expect(')'); /* We don't check that p here is the
					last arg of the current function. */
	  p = tree(VASTART, voidtype, p, q);
     }
     break;
     case TVAEND:
	  if (! cbe.have)
	       goto postfixprimary;
	  t = gettok(); 
	  expect('(');
	  p = postfix(expr(')'));
	  if (p->type != valisttype)
	       error("type error: incompatible type passed to va_end\n");
	  p = tree(VAEND, voidtype, p, NULL);
	  break;
     default:
     postfixprimary:
	  p = postfix(primary());
     }

     return p;
}
/* tickexpr: parse an expression enclosed in '`' */
Tree tickexpr() {
     Type ty = NULL;
     Tree e = NULL;
     Code cteb = NULL; 
     Symbol csym;
 
     initevalinfo();
     walk(NULL,0,0);
     sw2dcode();

     switch (t) {
     case IF:  case WHILE:  case DO:  case FOR: case BREAK:  case CONTINUE:
     case SWITCH: case CASE:  case DEFAULT:  case RETURN: 
     case '{':  case ';':  case GOTO:
	  cteb = tickexpr1();
	  ty = voidtype;
	  break;
     case ID:   
	  if (getchr() == ':') {
	       cteb = tickexpr1();
	       ty = voidtype;
	       break;
	  }
     default:
	  definept(NULL);
	  if (kind[t] != ID) {
	       error("unrecognized `C expression\n");
	       exit(1);
	  } else {
	       cteb = code(Closurebeg);
	       e = unary();
	       ty = e->type;
	       closureret(e);
	       code(Closureend)->u.begin = cteb;
	  }
     }

     sw2scode();
     csym = defclosure(declclosure(ty));
     assert(cteb);
     storeevalinfo(cteb,csym,ty);
     return ticktree(csym,ty);
}
static Code tickexpr1() {
				/* Begin emitting code for a `-expr */
     Code cteb = code(Closurebeg);
     statement(0, NULL, 0, 1);	/* Parse body of `-expr as a `-stmt */
     walk(NULL, 0, 0);		/* Append `-statement forest to code list */
     code(Closureend)->u.begin = cteb;
     return cteb;
}
static Tree postfix(p) Tree p; {
     for (;;)
	  switch (t) {
	  case INCR:  
	       p = tree(RIGHT, p->type,
			tree(RIGHT, p->type, p,
			     incr(t, p, consttree(1, inttype))), p);
	       t = gettok(); break;
	  case DECR:  
	       p = tree(RIGHT, p->type,
			tree(RIGHT, p->type, p,
			     incr(t, p, consttree(1, inttype))), p);
	       t = gettok(); break;
	  case '[':   {
	       Tree q;
	       t = gettok();
	       q = expr(']');
	       if (YYnull)
		    if (isptr(p->type))
			 p = nullcheck(p);
		    else if (isptr(q->type))
			 q = nullcheck(q);
	       p = (*optree['+'])(ADD, pointer(p), pointer(q));
	       if (isptr(p->type) && isarray(p->type->type))
		    p = retype(p, p->type->type);
	       else
		    p = rvalue(p);
	  } break;
	  case '(':   {
	       Type ty;
	       Coordinate pt;
	       p = pointer(p);
	       if (isptr(p->type) && isfunc(p->type->type))
		    ty = p->type->type;
	       else {
		    error("found `%t' expected a function\n", p->type);
		    ty = func(voidtype, NULL, NULL, 1);
	       }
	       pt = src;
	       t = gettok();
	       p = call(p, ty, pt);
	  } break;
	  case '.':   
	       t = gettok();
	       if (t == ID) {
		    if (isstruct(p->type)) {
			 Tree q = addrof(p);
			 p = field(q, token);
			 q = rightkid(q);
			 if (isaddrop(q->op) && q->u.sym->temporary) {
			      p = tree(RIGHT, p->type, p, NULL);
			      p->u.sym = q->u.sym;
			 }
		    } else
			 error("left operand of . has incompatible type "
			       "`%t'\n", p->type);
		    t = gettok();
	       } else
		    error("field name expected\n"); break;
	  case DEREF: 
	       t = gettok();
	       p = pointer(p);
	       if (t == ID) {
		    if (isptr(p->type) && isstruct(p->type->type)) {
			 if (YYnull)
			      p = nullcheck(p);
			 p = field(p, token);
		    } else
			 error("left operand of -> has incompatible type "
			       "`%t'\n", p->type);
		    t = gettok();
	       } else
		    error("field name expected\n"); break;
	  default:
	       return p;
	  }
}
static Tree primary() {
     Tree p;

     assert(t != '(');
     switch (t) {
     case ICON:
     case FCON: 
	  p = tree(CNST + ttob(tsym->type), tsym->type, NULL, NULL);
	  p->u.v = tsym->u.c.v;
	  break;
     case SCON: 
	  tsym->u.c.v.p = stringn(tsym->u.c.v.p, tsym->type->size);
	  tsym = constant(tsym->type, tsym->u.c.v); 
	  if (tsym->u.c.loc == NULL)
	       tsym->u.c.loc = genident(STATIC, tsym->type, GLOBAL);
	  p = idtree(tsym->u.c.loc); 
	  if (eval.ticklevel
	      && (lookup(tsym->name,eval.esymstr) == NULL))
	       (installcopy(tsym, &(eval.esymstr), 0, PERM))->generated = 0;
	  break;
     case ID:   
	  if (tsym == NULL) {
	       Symbol q = install(token, &identifiers, level, 
				  level < LOCAL ? PERM : FUNC);
	       q->src = src;
	       t = gettok();
	       if (t == '(') {
		    Symbol r;
		    q->sclass = EXTERN;
		    q->type = func(inttype, NULL, NULL, 1);
		    if (Aflag >= 1)
			 warning("missing prototype\n");
		    (*IR->defsymbol)(q);
		    if ((r = lookup(q->name, externals)) != NULL) {
			 q->defined = r->defined;
			 q->temporary = r->temporary;
			 q->generated = r->generated;
			 q->computed = r->computed;
			 q->addressed = r->addressed;
		    } else {
			 r = install(q->name, &externals, GLOBAL, PERM);
			 r->src = q->src;
			 r->type = q->type;
			 r->sclass = EXTERN;
		    }
		    assert(q->x.name);
		    if (eval.ticklevel) {
			 Symbol x;
			 if ((x = lookup(q->name, eval.esymaddr)) == NULL
			     || x->original != q)
				/* The second check deals with symbols used
				   in an outer scope after they have appeared 
				   in an inner scope: "{{ foo(); } foo();}" */
			      (installcopy
			       (q, &(eval.esymaddr), 0, PERM))->generated = 0;
			      
		    }
	       } else {
		    error("undeclared identifier `%s'\n", q->name);
		    q->sclass = AUTO;
		    q->type = inttype;
		    if (q->scope == GLOBAL)
			 (*IR->defsymbol)(q);
		    else
			 addlocal(q);
	       }
	       if (xref)
		    use(q, src);
	       if (eval.ticklevel && !eval.rtcp && !eval.unqtlevel)
		    closureaddsym(q, CETFV);
	       return idtree(q);
	  }
	  if (eval.ticklevel && !eval.rtcp && !eval.unqtlevel)
	       closureaddsym(tsym, CETFV);
	  
	  if (xref)
	       use(tsym, src);
	  if (tsym->sclass == ENUM)
	       p = consttree(tsym->u.value, inttype);
	  else {
	       if (tsym->sclass == TYPEDEF)
		    error("illegal use of type name `%s'\n", tsym->name);
	       p = idtree(tsym);
	  }
	  break;
     default:
	  error("illegal expression\n");
	  p = consttree(0, inttype);
     }
     t = gettok();
     return p;
}
Tree idtree(p) Symbol p; {
     int op;
     int dofreevar = 0;
     Tree e;
     Type ty = p->type;

     if (eval.ticklevel && !(eval.rtcp || eval.unqtlevel))
				/* If we're parsing a ` expression ... */
	  if (!isspec(ty) && p->copy) {
				/* ... and p is a free variable not of spec
				   type, mark it appropriately. */
	       p = p->copy;
	       dofreevar = 1;
				/* Specs are handled in tree() */
	  }

     ty = ty ? unqual(ty) : voidtype;

     p->ref += refinc;
     if (p->scope  == GLOBAL
	 ||  p->sclass == STATIC || p->sclass == EXTERN)
	  op = ADDRG+P;
     else if (p->scope == PARAM) {
	  op = dofreevar ? ADRFF+P : ADDRF+P;
	  if (isstruct(p->type) && !IR->wants_argb) {
	       e = tree(op, ptr(ptr(p->type)), NULL, NULL);
	       e->u.sym = p;
	       return rvalue(rvalue(e));
	  }
     } else
	  op = dofreevar ? ADRFL+P : ADDRL+P;
     if (isarray(ty) || isfunc(ty)) {
	  e = tree(op, p->type, NULL, NULL);
	  e->u.sym = p;
     } else {
	  e = tree(op, ptr(p->type), NULL, NULL);
	  if (eval.ticklevel && eval.unqtlevel)
	       /* Propagate scope information if necessary */
	       e->scope = p->scope;
	  e->u.sym = p;
	  e = rvalue(e);
     }
     if (eval.ticklevel /*&& eval.unqtlevel*/)
	  /* Removed eval.unqtlevel to correctly error-check things like 
	       void cspec *c;
	       `{ int i; *(c+i); };
           */
				/* This is a local which we may need to store
				   in closure because of unquoting */
	  e->scope = p->scope;
     return e;
}
Tree rvalue(p) Tree p; {
     Type ty = deref(p->type);

     ty = unqual(ty);
     if (YYnull && !isaddrop(p->op))	/* omit */
	  p = nullcheck(p);		/* omit */
     return tree(INDIR + (isunsigned(ty) ? I : ttob(ty)),
		 ty, p, NULL);
}
Tree lvalue(p) Tree p; {
     Tree t;
     if (generic(p->op) != INDIR) {
	  error("lvalue required\n");
	  return value(p);
     }
     t = p->kids[0];
     if (eval.tickp && generic(t->op) == ADDRL && !fordef) {
	  t->u.sym->assigned = 1;
	  if (t->u.sym->rtcp) {
	       assert(t->u.sym->drtcp);
	       error("'%s' is a derived run-time constant. "
		     "Cannot use it as an lvalue.\n", t->u.sym->name);
	       return p;
	  }
     } else if (unqual(p->type) == voidtype)
	  warning("`%t' used as an lvalue\n", p->type);
     return t;
}
Tree retype(p, ty) Tree p; Type ty; {
     Tree q;

     if (p->type == ty)
	  return p;
     q = tree(p->op, ty, p->kids[0], p->kids[1]);
     q->u = p->u;
     return q;
}
Tree rightkid(p) Tree p; {
     while (p && p->op == RIGHT)
	  if (p->kids[1])
	       p = p->kids[1];
	  else if (p->kids[0])
	       p = p->kids[0];
	  else
	       assert(0);
     assert(p);
     return p;
}
int hascall(p) Tree p; {
     if (p == 0)
	  return 0;
     if (generic(p->op) == CALL || 
	 (IR->mulops_calls &&
	  (p->op == DIV+I || p->op == MOD+I || p->op == MUL+I
	   || p->op == DIV+U || p->op == MOD+U || p->op == MUL+U)))
	  return 1;
     return hascall(p->kids[0]) || hascall(p->kids[1]);
}
Type binary(xty, yty) Type xty, yty; {
     if (isdouble(xty) || isdouble(yty))
	  return doubletype;
     if (xty == floattype || yty == floattype)
	  return floattype;
     if (isunsigned(xty) || isunsigned(yty))
	  return unsignedtype;
     return inttype;
}
Tree pointer(p) Tree p; {
     if (isarray(p->type))
	  /* assert(p->op != RIGHT || p->u.sym == NULL), */
	  p = retype(p, atop(p->type));
     else if (isfunc(p->type))
	  p = retype(p, ptr(p->type));
     return p;
}
Tree ktest(p) Tree p; {
     Tree kt = tree(KTEST, p->type, NULL, NULL);
     assert(p->u.sym && p->u.sym->rtcp);
     kt->u.sym = p->u.sym; kt->u.sym->x.name = p->u.sym->x.name;
     return kt;
}
Tree cond(p) Tree p; {
     int op = generic(rightkid(p)->op);

     if (op == AND || op == OR || op == NOT
	 ||  op == EQ  || op == NE
	 ||  op == LE  || op == LT || op == GE || op == GT)
	  return p;
     p = pointer(p);
     p = cast(p, promote(p->type));
     return (*optree[NEQ])(NE, p, consttree(0, inttype));
}
Tree cast(p, type) Tree p; Type type; {
     Type pty, ty = unqual(type);

     p = value(p);
     if (p->type == type)
	  return p;
     pty = unqual(p->type);
     if (pty == ty)
	  return retype(p, type);
     if (Xflag && isstruct(pty) && isstruct(ty) && extends(pty, ty)) {
	  Field q = extends(pty, ty);
	  return rvalue(simplify(ADD+P, ptr(ty), addrof(p),
				 consttree(q->offset, inttype)));
     }
     if (type == valisttype) {
	  assert(cbe.have);
	  return retype(p, type);
     }
     switch (pty->op) {
     case CHAR:    p = simplify(CVC, super(pty), p, NULL); break;
     case SHORT:   p = simplify(CVS, super(pty), p, NULL); break;
     case FLOAT:   p = simplify(CVF, doubletype, p, NULL); break;
     case INT:     p = retype(p, inttype);                 break;
     case DOUBLE:  p = retype(p, doubletype);              break;
     case ENUM:    p = retype(p, inttype);                 break;
     case UNSIGNED:p = retype(p, unsignedtype);            break;
     case POINTER:
	  if (isptr(ty)) {
	       Field q;
	       if (Xflag && isstruct(pty->type) && isstruct(ty->type)
		   && (q = extends(pty->type, ty->type)) != NULL)
		    return simplify(ADD+P, ty, p, 
				    consttree(q->offset, inttype));
	       if ((isfunc(pty->type) && !isfunc(ty->type)
		    || !isfunc(pty->type) &&  isfunc(ty->type))
		   && !eval.tCompiling)
		    warning("conversion from `%t' to `%t' is compiler "
			    "dependent\n", p->type, ty);

	       return retype(p, type);
	  } else
	       p = simplify(CVP, unsignedtype, p, NULL);
	  break;
     case VSPEC:
	  if (! isvspec(type))
	       error("attempt to cast vspec to non-vspec type");
	  else
	       return retype(p, type); /* MAXP: HACK ALERT */
				/* may need to create a cast tree */
     case CSPEC:
	  if (! iscspec(type))
	       error("attempt to cast cspec to non-cspec type");
	  else
	       return retype(p, type);
	  break;
     default: assert(0);
     }
     {
	  Type sty = super(ty);
	  pty = p->type;
	  if (pty != sty)
	       if (pty == inttype)
		    p = simplify(CVI, sty, p, NULL);
	       else if (pty == doubletype)
		    if (sty == unsignedtype) {
			 Tree c = tree(CNST+D, doubletype, NULL, NULL);
			 c->u.v.d = (double)INT_MAX + 1;
			 p = condtree(
			      simplify(GE, doubletype, p, c),
			      (*optree['+'])
			      (ADD,
			       cast(cast(simplify
					 (SUB, doubletype, p, c), inttype),
				    unsignedtype),
			       consttree((unsigned)INT_MAX + 1, unsignedtype)),
			      simplify(CVD, inttype, p, NULL));
		    } else
			 p = simplify(CVD, sty, p, NULL);
	       else if (pty == unsignedtype)
		    if (sty == doubletype) {
			 Tree two = tree(CNST+D, doubletype, NULL, NULL);
			 two->u.v.d = 2.;
			 p = (*optree['+'])
			      (ADD, (*optree['*'])
			       (MUL, two, simplify
				(CVU, inttype, simplify
				 (RSH, unsignedtype, p, 
				  consttree(1, inttype)), NULL)),
			       simplify(CVU, inttype,
					simplify(BAND, unsignedtype, p, 
						 consttree(1, unsignedtype)), 
					NULL));
		    } else
			 p = simplify(CVU, sty, p, NULL);
	       else assert(0);
     }
     if (ty == signedchar || ty == chartype || ty == shorttype)
	  p = simplify(CVI, type, p, NULL);
     else if (isptr(ty)
	      || ty == unsignedchar || ty == unsignedshort)
	  p = simplify(CVU, type, p, NULL);
     else if (ty == floattype)
	  p = simplify(CVD, type, p, NULL);
     else
	  p = retype(p, type);
     return p;
}
static Type super(ty) Type ty; {
     if (ty == signedchar || ty == chartype || isenum(ty)
	 ||  ty == shorttype  || ty == inttype  || ty == longtype)
	  return inttype;
     if (isptr(ty)
	 || ty == unsignedtype  || ty == unsignedchar
	 || ty == unsignedshort || ty == unsignedlong)
	  return unsignedtype;
     if (ty == floattype || ty == doubletype || ty == longdouble)
	  return doubletype;
     assert(0);
     return NULL;
}
Tree field(p, name) Tree p; char *name; {
     Field q;
     Type ty1, ty = p->type;

     if (isptr(ty))
	  ty = deref(ty);
     ty1 = ty;
     ty = unqual(ty);
     if ((q = fieldref(name, ty)) != NULL) {
	  if (isarray(q->type)) {
	       ty = q->type->type;
	       if (isconst(ty1) && !isconst(ty))
		    ty = qual(CONST, ty);
	       if (isvolatile(ty1) && !isvolatile(ty))
		    ty = qual(VOLATILE, ty);
	       ty = array(ty, q->type->size/ty->size, q->type->align);
	  } else {
	       ty = q->type;
	       if (isconst(ty1) && !isconst(ty))
		    ty = qual(CONST, ty);
	       if (isvolatile(ty1) && !isvolatile(ty))
		    ty = qual(VOLATILE, ty);
	       ty = ptr(ty);
	  }

	  if (YYcheck && !isaddrop(p->op) && q->offset > 0) /* omit */
	       p = nullcall(ty, YYcheck, p, 
			    consttree(q->offset, inttype)); /* omit */
	  else						    /* omit */
	       p = simplify(ADD+P, ty, p, consttree(q->offset, inttype));

	  if (q->lsb) {
	       p = tree(FIELD, ty->type, rvalue(p), NULL);
	       p->u.field = q;
	  } else if (!isarray(q->type))
	       p = rvalue(p);

     } else {
	  error("unknown field `%s' of `%t'\n", name, ty);
	  p = rvalue(retype(p, ptr(inttype)));
     }
     return p;
}
/* funcname - return name of function f or a function' */
char *funcname(f) Tree f; {
     if (isaddrop(f->op))
	  return stringf("`%s'", f->u.sym->name);
     return "a function";
}
static Tree nullcheck(p) Tree p; {
     if (!needconst && YYnull) {
	  p = value(p);
	  if (strcmp(YYnull->name, "_YYnull") == 0) {
	       Symbol t1 = temporary(REGISTER, voidptype, level);
	       p = tree(RIGHT, p->type,
			tree(OR, voidtype,
			     cond(asgn(t1, cast(p, voidptype))),
			     calltree(pointer(idtree(YYnull)), voidtype,
				      tree(ARG+I, inttype,
					   consttree(lineno, inttype), NULL), 
				      NULL)),
			idtree(t1));
	  }
	  else
	       p = nullcall(p->type, YYnull, p, consttree(0, inttype));

     }
     return p;
}
Tree nullcall(pty, f, p, e) Type pty; Symbol f; Tree p, e; {
     Tree fp, r;
     Type ty;

     if (isarray(pty))
	  return retype(nullcall(atop(pty), f, p, e), pty);
     ty = unqual(unqual(p->type)->type);
     if (file && *file)
	  fp = idtree(mkstr(file)->u.c.loc);
     else
	  fp = cast(consttree(0, inttype), voidptype);
     r = calltree(pointer(idtree(f)), pty,
		  tree(ARG+I, inttype, consttree(lineno, inttype), tree(
		       ARG+P, ptr(chartype), fp, tree(
			    ARG+I, unsignedtype, 
			    consttree(ty->align, unsignedtype), 
			    tree(ARG+I, unsignedtype, 
				 consttree(ty->size, unsignedtype), 
				 tree(ARG+I, inttype, e, 
				      tree(ARG+P, p->type, p, NULL)))))),
		  NULL);
     if (hascall(e))
	  r = tree(RIGHT, r->type, e, r);
     if (hascall(p))
	  r = tree(RIGHT, r->type, p, r);
     return r;
}

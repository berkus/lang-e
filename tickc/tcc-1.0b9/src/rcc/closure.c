#include "c.h"

/* This file contains code to create and manage closures for cspec objects. */

static void cleanfvs ARGS((Tree));
static void closurefielddo ARGS((Symbol));
static Type declclosureinternal ARGS((char *));
static void declclosurecp ARGS((char *, Type, Type));
static void declclosureentry ARGS((Symbol, void *));
static void allocclosure ARGS((Symbol));
static void defclosurecp ARGS((Symbol));
static void defclosureentry ARGS((Symbol, void*));
static void defclosuretarget ARGS((Symbol, void *));

static void checklocalgotos ARGS((void));
static void checklabeldef ARGS((Symbol, void *));

struct evalstate eval;

static int closureID;		/* current closure unique ID */
static int rtcID;		/* current runtime constant ID (per closure) */
static int specID;		/* current spec ID (per closure) */
static int tfvID;		/* current internal f.v. ID (per closure) */
static int lclID;		/* current auto-generated local ID */

char* tfvname = "_tfv_";
char* rtcname = "_rtc_";
char* lclname = "_lcl_";
char* specname = "_spec_";

/* Structure for storing closure fields */
typedef struct _CF {
     Tree t;			/* Closure field tree */
     struct _CF *next;		/* Link to rest of structure */
} CF;

/* Anchor of linked list for closure fields */
static CF cfa;
/* Current head of linked list for closure fields */
static CF * cfh = &cfa;

/* hasspec: returns true iff type is a spec type or a ptr to a type that is a
   spec type or a function return a spec type */
static inline int hasspec (Type type) {
     assert(type);
     return (isspec(type) 
				/* To iterate is human, */
	     || isfunc(type) && hasspec(freturn(type))
				/* To recurse divine.  -L. Peter Deutsch */
	     || isptr(type) && hasspec(type->type));
}

static inline int hasrtc (Tree t) {
     assert(t);
     return (generic(t->op) == RTC
	     || t->kids[0] && hasrtc(t->kids[0])
	     || t->kids[1] && hasrtc(t->kids[1]));
}

/* 
   closureaddsym: 
   adds a copy of the Symbol sym to the current closure in a bin which is a
   function of class.
*/

Symbol closureaddsym (Symbol sym, ceclass class) {
     Symbol s = NULL;

     switch(class) {
     case CELCL:
	  assert(0);
     case CEFCS: case CEFVS:
	  assert(0);
     case CETFV: {
	  Symbol q = lookup(sym->name, identifiers);
	  if (!q || q->scope > eval.ticklevel)
	       break;		/* If q is not a free variable, exit */
	  assert(q->scope >= GLOBAL);
	  if (hasspec(q->type)) /* Specs are handled by CEF[CV]S */
	       break;
	  if (isenum(q->type))	/* Do not need to be stored as FVs */
	      break;
	  if (q->scope > GLOBAL && lookup(q->name, eval.tfv) == NULL) {
				/* q is a free var; store it in Table tfv */
	       s = installcopy(q, &eval.tfv, 0, FUNC);
	       s->tfvp = 1;
	  } else if (q->scope == GLOBAL
		   && lookup(q->name, eval.esymval) == NULL
		   && lookup(q->name, eval.esymaddr) == NULL) {
				/* q is an external sym; store it for use in
				   making CGF, but not in closure assignment */
	       s = installcopy(q, &eval.esymval, 0, PERM);
	       s->generated = 0;
	       s->tfvp = 1; 
	  }
	  return s;
     }
     case CERTC:
     default:
	  assert(0);
     }
     return s;
}

/* 
   closureaddtree: 
   1. installs a Symbol s (corresponding to t) in the current closure, in a
      bin which is a function of class;
   2. stores t for evaluation at specification time
   3. returns s
*/
Symbol closureaddtree (Tree t, ceclass class) {
     Symbol s = NULL;
     char *name;

     if (hasrtc(t)) {
	  error("binding time mismatch: attempt to unquote '$'\n");
	  exit(1);
     }
     switch(class) {
     case CELCL: {
				/* CELCL is the class for a tree computed in a
				   ` expr, whose result vspec must be visible
				   outside the ` expr because of unquoting.
				   For example: int cspec f(int vspec);
				   		`{ int a; f(a); }
				   'a' would be a tree with class CELCL */
				/* Create a symbol */
	  name = mkname(NAMELCL);
/* CHECK THIS: */
				/* Install symbol in closure; inside ` expr
				   it appears as a local (scope>ticklevel) */
	  s = install(name, &eval.lcl, eval.ticklevel+1, FUNC);
	  assert(! isspec(t->type));
	  s->lclp = 1;
	  s->type = t->type;
				/* Set this so that any code is added to
				   dynamic code list even though unqtlevel>0 */
	  eval.lcllevel = 1;
				/* Generate code for assignment to symbol 
				   inside ` expression */
	  walk(asgn(s, t), 0, 0);
	  eval.lcllevel = 0;
				/* Return symbol */
	  return s;
     }
     break;
     case CEFCS: case CEFVS: {
				/* Each occurrence of a cspec or vspec 
				   generates a new closure entry */
	  if (t->op == INDIR+VS) {
				/* If this is a simple vspec, mangle its name
				   consistently to have unique closure entry */
	       assert(t->kids[0] && t->kids[0]->u.sym->type->op == VSPEC);
	       name = mangle(t->kids[0]->u.sym->name);
	  } else		/* Otherwise, generate a unique name; to 
				   minimize closure entries would have to
				   compare tree structure of other specs */
	       name = mkname(NAMESPEC);
				/* Install s in closure */
	  if (class == CEFCS) 
	       s = install(name, &eval.fcs, 0, FUNC);
	  else {		/* If a vspec, it may already exist */
	       if ((s = lookup(name, eval.fvs)) == NULL)
		    s = install(name, &eval.fvs, 0, FUNC);
	       else
		    return s;
	  }
				/* Store t for evaluation at spec time */
	  cleanfvs(t);
	  s->u.x.t = t;
	  s->type = t->type;
	  s->specp = 1;
	  return s;
     }
     case CETFV: {
				/* Each occurrence of an internal f.v. tree
				   generates a new closure entry */
	  name = mkname(NAMEFV);
				/* Install s in closure */
	  s = install(name, &eval.tfv, 0, FUNC);
				/* Store t for evaluation at spec time */
	  s->u.x.t = t;
	  s->type = t->type;
	  s->tfvp = 1;
	  return s;
     }
     break;
     case CERTC:
     default:
	  assert(0);
     }
     return s;
}

/* 
   closurefield: returns a tree which, when the current closure
   assignment is generated, will produce a reference to the field in the
   closure corresponding to sym.
*/
Tree closurefield (Symbol sym) {
     CF *cf;
     Tree t;
     Type ty;

     ty = sym->type;
     NEW0(cf, FUNC);
     assert(! isspec(ty));
     t = idtree(sym); t->op = generic(t->op)+VS;
     cf->t = retype(t, mkvspec(ty));
     cf->next = NULL; cfh->next = cf; cfh = cf;
     return cf->t;
}

/* 
   closurefielddo: replace temporary closure field references with real
   references into the closure structure.  s should be a Symbol denoting the
   current closure object having complete type (containing field offsets).
*/
static void closurefielddo (Symbol s) {
     Tree t;
     char *fn;			/* Current field name */
     CF *cf = cfa.next;		/* Pointer to incomplete closure field list */

     while (cf) {
	  assert(cf->t && cf->t->kids[0]);
	  t = cf->t->kids[0];
	  assert(t->u.sym);
				/* Extract name of incomplete closure field  */
	  fn = t->u.sym->name;
				/* Create and insert complete closure field */
	  *cf->t = *field(idtree(s), fn);
				/* Move on to next incomplete closure field */
	  cf = cf->next;
     }

     /* Reset the linked list */
     cfh = &cfa;
     cfa.next = NULL;
}

/*
   cleanfvs: remove free var references from a tree which we have discovered
   needs to be evaluated at specification time.  This replaces all ADRFLs
   with ADDRLs and deallocates free var symbols.
*/
static void cleanfvs (Tree t) {
     if (generic(t->op) == ADRFL || generic(t->op) == ADRFF) {
				/* Replace f.v. references with real ones */
	  Symbol s;

				/* Change the opcode */
	  t->op = (generic(t->op) == ADRFL ? ADDRL : ADDRF) + optype(t->op);
	  s = t->u.sym;
	  assert(s && s->original && s->tfvp);

	  s->nfvp = 1;		/* Mark this as *not* a f.v. */
	  s = t->u.sym = s->original;
	  s->copy = NULL;
				/* This is a leaf of a tree */
	  assert(t->kids[0] == NULL && t->kids[1] == NULL);
     } else {
				/* Recurse on subtrees, if any */
	  if (t->kids[0])
	       cleanfvs(t->kids[0]);
	  if (t->kids[1])
	       cleanfvs(t->kids[1]);
     }
}

/* 
   declnamedclosure: records a new closure type at the global level in the type
   symbol table; this closure has cgf cgf, label lbl, and one fvs fvs.
   
   This is a hack to "elegantly" enable dynamic jump/label.  Should eventually
   have this use stdarg/varargs to create whatever closure specified in the
   arguments.
*/
Type declnamedclosure(Symbol cgf, Symbol lbl, Symbol fvs) {
     Type structtype;

     structtype = declclosureinternal(stringf("%s%d", tcname.Cgfroot, 
					      closureID++));

     declclosurecp(cgf->name, structtype, voidtype);

     if (lbl) declclosureentry(lbl, (void *)structtype);
     if (fvs) declclosureentry(fvs, (void *)structtype);

     computefieldoffsets(structtype);

     structtype->fullp = 1;	/* `C-C: this type should be fully unparsed */
  
     return structtype;
}

/* 
   declclosure: records a new type at the global level in the type
   symbol table, to represent the type of the current tickexpr closure. 
*/
Type declclosure (Type exprtype) {
     Type structtype;

     checklocalgotos();

     structtype = declclosureinternal(stringf("%s%d", tcname.Cgfroot, closureID));

     declclosurecp(NULL, structtype, exprtype);

     forall(eval.lcl, declclosureentry, (void *)structtype);
     foreach(eval.fvs, 0, declclosureentry, (void *)structtype);
     foreach(eval.fcs, 0, declclosureentry, (void *)structtype);
     foreach(eval.tfv, 0, declclosureentry, (void *)structtype);
     foreach(eval.rtc, 0, declclosureentry, (void *)structtype);

     computefieldoffsets(structtype);

     structtype->fullp = 1;	/* `C-C: this type should be fully unparsed */
  
     return structtype;
}

/*
   declclosureinternal:
   does the dirty work for declaring a closure:
   . checks for name conflicts at the global type level
   . appropriately adds the closure type to the global
     symbol table, letting it be visible through the all
     link from other symbol tables
   . returns the type of the closure  
*/
static Type declclosureinternal (char *name) {
     Symbol p;
     Table tp, tpnext=types;

     assert(tpnext);
     while (tpnext->previous && tpnext->previous->level > GLOBAL)
	  tpnext = tpnext->previous;
     tp = tpnext->previous ? tpnext->previous : tpnext;

				/* Check for name conflicts */
     if ((p = lookup(name, tp)) != NULL)
	  fatal("declclosureinternal", 
		"internal closure type name conflict\n", 0);

				/* Install the type at the global level */
     p = install(name, &tp, GLOBAL, PERM);
     p->type = type(B, NULL, 0, 0, p);

     return p->type;
}

/* 
   declclosurecp: adds a closure function pointer to the closure structure
   type structtype.  The return type of the function pointer
   is v_reg or void, depending on whether the exprtype of
   the corresponding tickexpr is non-void or void, respectively. 
*/
static void declclosurecp (char *name, Type structtype, Type exprtype) {
     char *codename, *structname;
     Type ftype, rtype, *proto;

     proto = newarray(2, sizeof(Type *), PERM);
     proto[0] = voidptype; proto[1] = NULL;
     if (exprtype == voidtype)
	  rtype = voidtype;
     else
	  rtype = (findtcsym(tcname.Local_t, identifiers))->type;
     ftype = func(rtype, proto, NULL, 0);

     structname = structtype->u.sym->name;
     codename = name ? name : stringf("%s%s", structname, tcname.Cgfsuffix);

     if (cbe.have)
	  outsfdx(stringf("%s (void *__c_arg);\n", 
			  typestring(0, rtype, codename)),
		  hhhfdIdx);

     if (cbe.emit) {
	  cbufflush(0); cbufflushoff();
     }
     eval.cgfsym = dclglobal(0, codename, ftype, &src, NULL);
     if (cbe.emit)
	  cbufflushon();
				/* Add cgf field */
     newfield(tcname.Cgffield, structtype, ptr(ftype));
				/* Add label field */
     newfield(tcname.Labfield, structtype, 
	      findtcsym(tcname.Label_t, identifiers)->type);
}

/* 
   declclosureentry:
   function passed to foreach to be applied to each element of freevars;
   adds the given element to the closure type in v 
*/
static void declclosureentry(Symbol s, void *v) {
     Type vty, sty;

     assert(s);
     sty = s->type;

     vty = (Type)v;
     assert(vty && sty);

     if (s->tfvp && s->nfvp	/* Do not emit removed free vars or rtcs */
	 || s->rtcp && (s->nrtcp || s->drtcp))
	  return;
     
     if (s->lclp)
	  sty = mkvspec(sty);
     else if (isarray(sty))
	  sty = atop(sty);
     else if (!s->rtcp && !s->specp)
	  sty = ptr(sty);

     newfield(s->name, vty, sty);
}

/* 
   defclosure: instantiates a closure object of type closurereftype, 
   initializing its fields.  Returns the symbol corresponding
   to the object.
*/
Symbol defclosure (Type closurereftype) {
     Symbol theclosure;

     assert(!cbe.emit || !cbe.flush);
     assert(closurereftype->fullp);
     if (cbe.have)
	  outsfdx(stringf("%s;\n", fulltypestring(closurereftype, "")), 
		  hhhfdIdx);

     NEW0(theclosure, FUNC);	/* Create new closure symbol */
     theclosure->name = tcname.Gencspec;
     theclosure->scope = LOCAL;
     theclosure->sclass = REGISTER;
     theclosure->type = ptr(closurereftype);
     addlocal(theclosure);

     if (cbe.have)		/* `C-C */
	  outsfdx(stringf("("), cccfdIdx);

     closurefielddo(theclosure);/* Complete incomplete closure field refs */
     allocclosure(theclosure);	/* Gen code to allocate closure */
     defclosurecp(theclosure);	/* Gen code to define pointer to CGF */

				/* Assign values to free variables, run-time 
				   constants, and lcls */
     forall(eval.lcl, defclosureentry, (void *)theclosure);
     foreach(eval.fvs, 0, defclosureentry, (void *)theclosure);
     foreach(eval.fcs, 0, defclosureentry, (void *)theclosure);
     foreach(eval.tfv, 0, defclosureentry, (void *)theclosure);
     foreach(eval.rtc, 0, defclosureentry, (void *)theclosure);
     foreach(eval.targ, 0, defclosuretarget, (void *)theclosure);

     if (cbe.have)		/* `C-C */
	  outsfdx(stringf(",\n\t\t%s)", tcname.Gencspec), cccfdIdx);

     return theclosure;
}

/* 
   allocclosure: returns a tree to allocate space to theclosure 
*/
static void allocclosure (Symbol theclosure) {
     Symbol msym;
     Tree t, f, args;
     Type ctype = theclosure->type;

     if (eval.fc && !eval.leafp) {
	  Symbol lsym = findtcsym(tcname.Leafp, globals);
	  walk(asgn(lsym, consttree(0, inttype)), 0, 0);
     }

     msym = findtcsym(tcname.Malloc, globals);
     msym->type = func(ptr(chartype), NULL, NULL, 1);

     f = idtree(msym);
     args = tree(ARGI, unsignedtype,
		 consttree(ctype->type->size, unsignedtype), NULL);
     t = asgn(theclosure, calltree(f, ctype, args, NULL));

     if (cbe.have)
	  outsfdx(unparsetree(t), cccfdIdx);
     else
	  walk(t, 0, 0);
}

/* 
   defclosurecp: define the closure's function pointer 
*/
static void defclosurecp (Symbol theclosure) {
     Tree lhs, rhs, t;
				/* Code */
     lhs = field(idtree(theclosure), tcname.Cgffield);
     rhs = idtree(eval.cgfsym);
     t = asgntree(ASGN, lhs, rhs);
     if (cbe.have)
	  outsfdx(stringf(",\n\t\t%s", unparsetree(t)), cccfdIdx);
     else
	  walk(t, 0, 0);
				/* Label */
     lhs = field(idtree(theclosure), tcname.Labfield);
     rhs = consttree(0, unsignedtype);
     t = asgntree(ASGN, lhs, rhs);
     if (cbe.have)
	  outsfdx(stringf(",\n\t\t%s", unparsetree(t)), cccfdIdx);
     else
	  walk(t, 0, 0);
}

/* 
   defclosureentry: create code for a closure assignment 
*/
static void defclosureentry (Symbol e, void *v) {
     Symbol theclosure = (Symbol)v;
     Symbol entry;
     Tree lhs, rhs, t;

     if (e->tfvp && e->nfvp	/* Do not emit removed free vars or rtcs */
	 || e->rtcp && (e->nrtcp || e->drtcp))
	  return;

     entry = (e->specp || e->rtcp || e->lclp) ? e : e->original;

     lhs = field(idtree(theclosure), entry->name);

     if (entry->lclp) {
	  rhs = calllocaltree(entry->type, 0);
     } else if (entry->rtcp || entry->specp) {
	  rhs = entry->u.x.t;
     } else {
	  rhs = idtree(entry);
	  if (!isfunc(entry->type) && !isarray(entry->type)) {
	       entry->addressed = 1;  /* can no longer live in a register */
	       rhs = lvalue(rhs);
	  }
     }

     t = asgntree(ASGN, lhs, rhs);
     if (cbe.have)
	  outsfdx(stringf(",\n\t\t%s", unparsetree(t)), cccfdIdx);
     else
	  walk(t, 0, 0);
}

/* 
   defclosuretarget: prepare for linking a cspec that is the target of 
   a jump().
*/
static void defclosuretarget (Symbol e, void *v) {
     Symbol theclosure = (Symbol)v;
     Tree arg, f, t;

     assert(e->specp);

     arg = tree(ARGP, theclosure->type, 
		field(idtree(theclosure), e->name), NULL);
     f = idtree(findtcsym(tcname.Mktarget, globals));
     t = calltree(f, voidtype, arg, NULL);

     if (cbe.have)
	  outsfdx(stringf(",\n\t\t%s", unparsetree(t)), cccfdIdx);
     else
	  walk(t, 0, 0);
}

/* 
   initevalinfo: initializes bookkeeping information necessary to parse
   a tickexpr
*/
void initevalinfo () {
     eval.tickp = 1;
     eval.leafp = 1;

     eval.lcl = table(NULL, 0);
     eval.fvs = table(NULL, 0);
     eval.fcs = table(NULL, 0);
     eval.tfv = table(NULL, 0);
     eval.rtc = table(NULL, 0);
     eval.targ = table(NULL, 0);

     eval.luse = table(NULL, 0);
     eval.ldef = table(NULL, 0);
	  
     eval.statics = table(NULL,0);
     eval.labels = NULL;

     if (!eval.esymaddr) {
	  assert(!(eval.esymval || eval.esymcnst || eval.esymstr));
	  eval.esymcnst = permtable(NULL,0);
	  eval.esymstr = permtable(NULL,0);
	  eval.esymaddr = permtable(NULL,0);
	  eval.esymval = permtable(NULL,0);
     }
}

/*
   storeevalinfo: define the u.closure field of Code code according
   to the current eval info, sym, and ty.  Clear the eval structure.
*/
void storeevalinfo (Code code, Symbol sym, Type ty) {
     if (code) {
	  code->u.closure.sym = sym;
	  code->u.closure.type = ty;
	  code->u.closure.cgf = eval.cgfsym;
	  code->u.closure.uid = closureID++;

	  code->u.closure.lcl = eval.lcl;
	  code->u.closure.fvs = eval.fvs;
	  code->u.closure.fcs = eval.fcs;
	  code->u.closure.tfv = eval.tfv;
	  code->u.closure.rtc = eval.rtc;

	  code->u.closure.labels = eval.labels;
	  code->u.closure.statics = eval.statics;
     }
     eval.tickp = 0;
     rtcID = specID = tfvID = lclID = 0;
     eval.lcl = eval.fvs = eval.fcs = eval.tfv = eval.rtc = eval.targ = NULL;
     eval.luse = eval.ldef = NULL;
     eval.labels = NULL;
     eval.statics = NULL;
     eval.tExists = eval.tExistsHere = 1;
}


/* checklocalgotos: ensure that gotos within the current cspec reference no
   labels defined outside the cspec */
static void checklocalgotos (void) {
     foreach(eval.luse, 0, checklabeldef, 0);
}

/* checklabeldef: make sure that label e has been defined in current cspec */
static void checklabeldef (Symbol e, void *v) {
     if (lookup(e->name, eval.ldef) == NULL)
	  error("reference to label `%s' undefined in cspec\n", e->name);
}


/* tickrefcnt: if emitting icode, generate code that computes reference
   weights given the current opcode op and value val.
   op = ASGN: save old refwt; op = INDIR: restore old refwt;
   op = MUL, DIV: multiply or divide refwt by val. */
void tickrefcnt (int op, int val) {
     if (!eval.fc) {
	  switch (op) {
	  case MUL:
	       walk(notetree(stringf("i_refmul(%d);", val)), 0, 0);
	       break;
	  case DIV:
	       walk(notetree(stringf("i_refdiv(%d);", val)), 0, 0);
	       break;
	  default: assert(0);
	  }
     }
}


/* mkname: a name generating function */
char *mkname (nameclass class) {
     switch (class) {
     case NAMERTC:
	  return stringf("%s_%d_%d", rtcname, closureID, rtcID++);
     case NAMESPEC:
	  return stringf("%s_%d_%d", specname, closureID, specID++);
     case NAMEFV:
	  return stringf("%s_%d_%d", tfvname, closureID, tfvID++);
     case NAMELCL:
	  return stringf("%s_%d_%d", lclname, closureID, lclID++);
     default:
	  assert(0);
     }
     return 0;
}

/* mangle: mangle names for use inside CGFs */
char *mangle (char * name) {
     return stringf("_%s",name);
}

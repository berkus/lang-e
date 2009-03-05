#include "c.h"

#define isunnamed(ty) (*(ty)->u.sym->name >= '1' && *(ty)->u.sym->name <= '9')

static Field check ARGS((Type, Type, Field, int));
static Field isfield ARGS((char *, Field));
static inline char * scname ARGS((int));

static struct entry {
	struct type type;
	struct entry *link;
} *typetable[128];
static int maxlevel;

static Symbol pointersym;
static Symbol cspecsym;
static Symbol vspecsym;

Type chartype;			/* char */
Type doubletype;		/* double */
Type floattype;			/* float */
Type inttype;			/* signed int */
Type longdouble;		/* long double */
Type longtype;			/* long */
Type shorttype;			/* signed short int */
Type signedchar;		/* signed char */
Type unsignedchar;		/* unsigned char */
Type unsignedlong;		/* unsigned long int */
Type unsignedshort;		/* unsigned short int */
Type unsignedtype;		/* unsigned int */
Type voidptype;			/* void* */
Type voidtype;			/* basic types: void */

/* default spec types */
Type intvspectype;
Type voidcspectype;

/* "valist" type, for compiling varargs in `C-C mode */
Type valisttype;

Type type(op, ty, size, align, sym)
int op, size, align; Type ty; void *sym; {
     unsigned h = (op^((unsigned)ty>>3))
	  &(NELEMS(typetable)-1);
     struct entry *tn;

     if (op != FUNCTION && (op != ARRAY || size > 0))
	  for (tn = typetable[h]; tn; tn = tn->link)
	       if (tn->type.op    == op   && tn->type.type  == ty
		   &&  tn->type.size  == size && tn->type.align == align
		   &&  tn->type.u.sym == sym)
		    return &tn->type;
     NEW(tn, PERM);
     tn->type.op = op;
     tn->type.type = ty;
     tn->type.size = size;
     tn->type.align = align;
     tn->type.u.sym = sym;
     tn->type.fullp = 0;
     memset(&tn->type.x, 0, sizeof tn->type.x);
     tn->link = typetable[h];
     typetable[h] = tn;
     return &tn->type;
}
void typeInit() {
#define xx(v,name,op,metrics) { \
		Symbol p = install(string(name), &types, GLOBAL, PERM);\
		v = type(op, 0, IR->metrics.size, IR->metrics.align, p);\
		assert(v->align == 0 || v->size%v->align == 0); \
		p->type = v; p->addressed = IR->metrics.outofline; }
     xx(chartype,     "char",          CHAR,    charmetric);
     xx(doubletype,   "double",        DOUBLE,  doublemetric);
     xx(floattype,    "float",         FLOAT,   floatmetric);
     xx(inttype,      "int",           INT,     intmetric);
     xx(longdouble,   "long double",   DOUBLE,  doublemetric);
     xx(longtype,     "long int",      INT,     intmetric);
     xx(shorttype,    "short",         SHORT,   shortmetric);
     xx(signedchar,   "signed char",   CHAR,    charmetric);
     xx(unsignedchar, "unsigned char", CHAR,    charmetric);
     xx(unsignedlong, "unsigned long", UNSIGNED,intmetric);
     xx(unsignedshort,"unsigned short",SHORT,   shortmetric);
     xx(unsignedtype, "unsigned int",  UNSIGNED,intmetric);
				/* Required in `C-C for varargs */
     xx(valisttype,   "va_list",       VALIST,  ptrmetric);
#undef xx
     {
	  Symbol p;
	  p = install(string("void"), &types, GLOBAL, PERM);
	  voidtype = type(VOID, NULL, 0, 0, p);
	  p->type = voidtype;
     }
     pointersym = install(string("T*"), &types, GLOBAL, PERM);
     pointersym->addressed = IR->ptrmetric.outofline;
		
     cspecsym = install(string("T__cs"), &types, GLOBAL, PERM);
     cspecsym->addressed = IR->ptrmetric.outofline;
     vspecsym = install(string("T__vs"), &types, GLOBAL, PERM);
     vspecsym->addressed = IR->ptrmetric.outofline;

     voidptype = ptr(voidtype);
     intvspectype = mkvspec(inttype);
     voidcspectype = mkcspec(voidtype);
     
     assert(voidptype->align > 0 && voidptype->size%voidptype->align == 0);
     assert(unsignedtype->size >= voidptype->size);
     assert(inttype->size >= voidptype->size);
}

void rmtypes(lev) int lev; {
     if (maxlevel >= lev) {
	  int i;
	  maxlevel = 0;
	  for (i = 0; i < NELEMS(typetable); i++) {
	       struct entry *tn, **tq = &typetable[i];
	       while ((tn = *tq) != NULL)
		    if (tn->type.op == FUNCTION)
			 tq = &tn->link;
		    else if (tn->type.u.sym && tn->type.u.sym->scope >= lev)
			 *tq = tn->link;
		    else {
			 if (tn->type.u.sym 
			     && tn->type.u.sym->scope > maxlevel)
			      maxlevel = tn->type.u.sym->scope;
			 tq = &tn->link;
		    }

	  }
     }
}
int checknestedtick(op, ty) int op; Type ty; {
     for (; ty; ty = ty->type)
	  if (ty->op == CSPEC || ty->op == VSPEC) {
	       error("illegal type `%k of %t'\n", op, ty);
	       return 1;
	  }
     return 0;
}
Type mkcspec(ty) Type ty; {
     if (checknestedtick(CSPEC, ty)) return ty;
     return type(CSPEC, ty, IR->ptrmetric.size, 
		 IR->ptrmetric.align, cspecsym);
}
Type mkvspec(ty) Type ty; {
     if (checknestedtick(VSPEC, ty)) return ty;
     return type(VSPEC, ty, IR->ptrmetric.size, 
		 IR->ptrmetric.align, vspecsym);
}
Type derefcvspec(ty) Type ty; {
     assert(isspec(ty));
     return ty->type;
}
Type ptr(ty) Type ty; {
     return type(POINTER, ty, IR->ptrmetric.size,
		 IR->ptrmetric.align, pointersym);
}
Type deref(ty) Type ty; {
     if (isptr(ty))
	  ty = ty->type;
     else
	  error("type error: %s\n", "pointer expected");
     return isenum(ty) ? unqual(ty)->type : ty;
}
Type array(ty, n, a) Type ty; int n, a; {
     assert(ty);
     if (isfunc(ty)) {
	  error("illegal type `array of %t'\n", ty);
	  return array(inttype, n, 0);
     }
     if (level > GLOBAL && isarray(ty) && ty->size == 0)
	  error("missing array size\n");
     if (ty->size == 0) {
	  if (unqual(ty) == voidtype)
	       error("illegal type `array of %t'\n", ty);
	  else if (Aflag >= 2)
	       warning("declaring type array of %t' is undefined\n", ty);

     } else if (n > INT_MAX/ty->size) {
	  error("size of `array of %t' exceeds %d bytes\n",
		ty, INT_MAX);
	  n = 1;
     }
     return type(ARRAY, ty, n*ty->size,
		 a ? a : ty->align, NULL);
}
Type atop(ty) Type ty; {
     if (isarray(ty))
	  return ptr(ty->type);
     error("type error: %s\n", "array expected");
     return ptr(ty);
}
Type qual(op, ty) int op; Type ty; {
     if (isarray(ty))
	  ty = type(ARRAY, qual(op, ty->type), ty->size,
		    ty->align, NULL);
     else if (isfunc(ty))
	  warning("qualified function type ignored\n");
     else if (isconst(ty)    && op == CONST
	      ||       isvolatile(ty) && op == VOLATILE)
	  error("illegal type `%k %t'\n", op, ty);
     else {
	  if (isqual(ty)) {
	       op += ty->op;
	       ty = ty->type;
	  }
	  ty = type(op, ty, ty->size, ty->align, NULL);
     }
     return ty;
}
Type func(ty, proto, pname, style) Type ty, *proto; char **pname; int style; {
     if (ty && (isarray(ty) || isfunc(ty)))
	  error("illegal return type `%t'\n", ty);
     ty = type(FUNCTION, ty, 0, 0, NULL);
     ty->u.f.proto = proto;
     ty->u.f.pname = pname;
     ty->u.f.oldstyle = style;
     return ty;
}
Type freturn(ty) Type ty; {
     if (isfunc(ty))
	  return ty->type;
     error("type error: %s\n", "function expected");
     return inttype;
}
int variadic(ty) Type ty; {
     if (isfunc(ty) && ty->u.f.proto) {
	  int i;
	  for (i = 0; ty->u.f.proto[i]; i++)
	       ;
	  return i > 1 && ty->u.f.proto[i-1] == voidtype;
     }
     return 0;
}
Type newstruct(op, tag) int op; char *tag; {
     Symbol p;

     assert(tag);
     if (*tag == 0) {
	  if (cbe.have && cbe.emit)
	       tag = stringf("_TS%d",genlabel(1));
	  else 
	       tag = stringd(genlabel(1));
     } else
	  if ((p = lookup(tag, types)) != NULL 
	      && (p->scope == level || p->scope == PARAM 
		  && level == PARAM+1)) {
	       if (p->type->op == op && !p->defined)
		    return p->type;
	       error("redefinition of `%s' previously defined at %w\n",
		     p->name, &p->src);
	  }
     p = install(tag, &types, level, PERM);
     p->type = type(op, NULL, 0, 0, p);
     if (p->scope > maxlevel)
	  maxlevel = p->scope;
     p->src = src;
     return p->type;
}
Field newfield(name, ty, fty) char *name; Type ty, fty; {
     Field p, *q = &ty->u.sym->u.s.flist;

     if (name == NULL) {
	  if (cbe.have && cbe.emit)
	       name = stringf("_TF%d",genlabel(1));
	  else 
	       name = stringd(genlabel(1));
     }
     for (p = *q; p; q = &p->link, p = *q)
	  if (p->name == name)
	       error("duplicate field name `%s' in `%t'\n",
		     name, ty);
     NEW0(p, PERM);
     *q = p;
     p->name = name;
     p->type = fty;
     if (xref) {						/* omit */
	  if (ty->u.sym->u.s.ftab == NULL)			/* omit */
	       ty->u.sym->u.s.ftab = table(NULL, level);	/* omit */
	  install(name, &ty->u.sym->u.s.ftab, 0, PERM)->src = src;/* omit */
     }								/* omit */
     return p;
}
int eqtype(ty1, ty2, ret) Type ty1, ty2; int ret; {
     if (ty1 == ty2)
	  return 1;
     if (ty1->op != ty2->op)
	  return 0;
     switch (ty1->op) {
     case CHAR: case SHORT: case UNSIGNED: case INT:
     case ENUM: case UNION: case STRUCT:   case DOUBLE:
	  return 0;
     case CSPEC:
     case VSPEC:
     case POINTER:  return eqtype(ty1->type, ty2->type, 1);
     case VOLATILE: case CONST+VOLATILE:
     case CONST:    return eqtype(ty1->type, ty2->type, 1);
     case ARRAY:    if (eqtype(ty1->type, ty2->type, 1)) {
	  if (ty1->size == ty2->size)
	       return 1;
	  if (ty1->size == 0 || ty2->size == 0)
	       return ret;
     }
     return 0;
     case FUNCTION: if (eqtype(ty1->type, ty2->type, 1)) {
	  Type *p1 = ty1->u.f.proto, *p2 = ty2->u.f.proto;
	  if (p1 == p2)
	       return 1;
	  if (p1 && p2) {
	       for ( ; *p1 && *p2; p1++, p2++)
		    if (eqtype(unqual(*p1), unqual(*p2), 1) == 0)
			 return 0;
	       if (*p1 == NULL && *p2 == NULL)
		    return 1;
	  } else {
	       if (variadic(p1 ? ty1 : ty2))
		    return 0;
	       if (p1 == NULL)
		    p1 = p2;
	       for ( ; *p1; p1++) {
		    Type ty = unqual(*p1);
		    if (promote(ty) != ty || ty == floattype)
			 return 0;
	       }
	       return 1;
	  }
     }
     return 0;
     }
     assert(0); return 0;
}
Type promote(ty) Type ty; {
     ty = unqual(ty);
     if (isunsigned(ty) || ty == longtype)
	  return ty;
     else if (isint(ty) || isenum(ty))
	  return inttype;
     return ty;
}
Type compose(ty1, ty2) Type ty1, ty2; {
     if (ty1 == ty2)
	  return ty1;
     assert(ty1->op == ty2->op);
     switch (ty1->op) {
     case CSPEC:
	  return mkcspec(compose(ty1->type, ty2->type));
     case VSPEC:
	  return mkvspec(compose(ty1->type, ty2->type));
     case POINTER:
	  return ptr(compose(ty1->type, ty2->type));
     case CONST+VOLATILE:
	  return qual(CONST, qual(VOLATILE,
				  compose(ty1->type, ty2->type)));
     case CONST: case VOLATILE:
	  return qual(ty1->op, compose(ty1->type, ty2->type));
     case ARRAY:    { 
	  Type ty = compose(ty1->type, ty2->type);
	  if (ty1->size && ty1->type->size && ty2->size == 0)
	       return array(ty, ty1->size/ty1->type->size, ty1->align);
	  if (ty2->size && ty2->type->size && ty1->size == 0)
	       return array(ty, ty2->size/ty2->type->size, ty2->align);
	  return array(ty, 0, 0);    }
     case FUNCTION: { 
	  Type *p1  = ty1->u.f.proto, *p2 = ty2->u.f.proto;
	  Type ty   = compose(ty1->type, ty2->type);
	  List tlist = NULL;
	  if (p1 == NULL && p2 == NULL)
	       return func(ty, NULL, NULL, 1);
	  if (p1 && p2 == NULL)
	       return func(ty, p1, ty1->u.f.pname, ty1->u.f.oldstyle);
	  if (p2 && p1 == NULL)
	       return func(ty, p2, ty2->u.f.pname, ty2->u.f.oldstyle);
	  for ( ; *p1 && *p2; p1++, p2++) {
	       Type ty = compose(unqual(*p1), unqual(*p2));
	       if (isconst(*p1)    || isconst(*p2))
		    ty = qual(CONST, ty);
	       if (isvolatile(*p1) || isvolatile(*p2))
		    ty = qual(VOLATILE, ty);
	       tlist = append(ty, tlist);
	  }
	  assert(*p1 == NULL && *p2 == NULL);
	  return func(ty, ltov(&tlist, PERM), NULL, 0); }
     }
     assert(0); return NULL;
}
int ttob(ty) Type ty; {
     switch (ty->op) {
     case CONST: case VOLATILE: case CONST+VOLATILE:
	  return ttob(ty->type);
     case CHAR: case INT:   case SHORT: case UNSIGNED: 
     case VOID: case FLOAT: case DOUBLE:  return ty->op;
     case CSPEC: return CS;
     case VSPEC: return VS;
     case POINTER: case FUNCTION:         return POINTER;
     case ARRAY: case STRUCT: case UNION: return STRUCT;
     case ENUM:                           return INT;
     case VALIST:
	  assert(cbe.have);
	  return VALIST;
     }
     assert(0); return INT;
}
Type btot(op) int op; {
     switch (optype(op)) {
     case F: return floattype;
     case D: return doubletype;
     case C: return chartype;
     case S: return shorttype;
     case I: return inttype;
     case U: return unsignedtype;
     case V: return voidtype;
     case CS: case VS: case P: 
	  return voidptype;
     case VALIST:
	  assert(cbe.have);
	  return valisttype;
     }
     assert(0); return 0;
}
int ttov(ty) Type ty; {
#ifndef __TCC__	/* To avoid duplicate enum error when tcc compiles itself */
     enum {			/* Vcode/icode type codes */
	  I_C,			/* char */
	  I_UC,			/* unsigned char */
	  I_S,			/* short */
	  I_US,			/* unsigned short */
	  I_I,			/* int */
	  I_U,			/* unsigned */
	  I_L,			/* long */
	  I_UL,			/* unsigned long */
	  I_P,			/* pointer */
	  I_F,			/* floating */
	  I_D,			/* double */
	  I_V,			/* void */
	  I_B,			/* block structure */
	  I_ERR			/* error condition */
     };
#endif
     switch (ty->op) {
     case CONST: case VOLATILE: 
     case CONST+VOLATILE:		return ttov(ty->type);
     case CHAR:				return I_C;
     case INT:				return I_I;
     case SHORT:			return I_S;
     case UNSIGNED:			return I_U;
     case VOID:				return I_V;
     case FLOAT:			return I_F;
     case DOUBLE:			return I_D;
     case CSPEC: case VSPEC:		assert(0); return I_I;
     case POINTER: case FUNCTION:	return I_P;
     case ARRAY: case STRUCT: 
     case UNION: 			return I_B;
     case ENUM:				return I_I;
     }
     assert(0); return I_I;
}
int hasproto(ty) Type ty; {
     if (ty == 0)
	  return 1;
     switch (ty->op) {
     case CONST: case VOLATILE: case CONST+VOLATILE: case POINTER:
     case ARRAY:
     case VSPEC: case CSPEC:
	  return hasproto(ty->type);
     case FUNCTION:
	  return hasproto(ty->type) && ty->u.f.proto;
     case STRUCT: case UNION:
     case CHAR:   case SHORT: case INT:  case DOUBLE:
     case VOID:   case FLOAT: case ENUM: case UNSIGNED:
	  return 1;
     }
     assert(0); return 0;
}
/* check - check ty for ambiguous inherited fields, 
   return augmented field set */
static Field check(ty, top, inherited, off)
Type ty, top; Field inherited; int off; {
     Field p;

     for (p = ty->u.sym->u.s.flist; p; p = p->link)
	  if (p->name && isfield(p->name, inherited))
	       error("ambiguous field `%s' of `%t' from `%t'\n", p->name, 
		     top, ty);
	  else if (p->name && !isfield(p->name, top->u.sym->u.s.flist)) {
	       Field new;
	       NEW(new, FUNC);
	       *new = *p;
	       new->offset = off + p->offset;
	       new->link = inherited;
	       inherited = new;
	  }
     for (p = ty->u.sym->u.s.flist; p; p = p->link)
	  if (p->name == 0)
	       inherited = check(p->type, top, inherited,
				 off + p->offset);
     return inherited;
}

/* checkfields - check for ambiguous inherited fields in struct/union ty */
void checkfields(ty) Type ty; {
     Field p, inherited = 0;

     for (p = ty->u.sym->u.s.flist; p; p = p->link)
	  if (p->name == 0)
	       inherited = check(p->type, ty, inherited, p->offset);
}

/* extends - if ty extends fty, return a pointer to field structure */
Field extends(ty, fty) Type ty, fty; {
     Field p, q;

     for (p = unqual(ty)->u.sym->u.s.flist; p; p = p->link)
	  if (p->name == 0 && unqual(p->type) == unqual(fty))
	       return p;
	  else if (p->name == 0 && (q = extends(p->type, fty)) != NULL) {
	       static struct field f;
	       f = *q;
	       f.offset = p->offset + q->offset;
	       return &f;
	  }
     return 0;
}

/* fieldlist - construct a flat list of fields in type ty */
Field fieldlist(ty) Type ty; {
     Field p, q, t, inherited = 0, *r;

     ty = unqual(ty);
     for (p = ty->u.sym->u.s.flist; p; p = p->link)
	  if (p->name == 0)
	       inherited = check(p->type, ty, inherited, p->offset);
     if (inherited == 0)
	  return ty->u.sym->u.s.flist;
     for (q = 0, p = inherited; p; q = p, p = t) {
	  t = p->link;
	  p->link = q;
     }
     for (r = &inherited, p = ty->u.sym->u.s.flist; p && q; )
	  if (p->name == 0)
	       p = p->link;
	  else if (p->offset <= q->offset) {
	       NEW(*r, FUNC);
	       **r = *p;
	       r = &(*r)->link;
	       p = p->link;
	  } else {
	       *r = q;
	       r = &q->link;
	       q = q->link;
	  }
     for ( ; p; p = p->link)
	  if (p->name) {
	       NEW(*r, FUNC);
	       **r = *p;
	       r = &(*r)->link;
	  }
     *r = q;
     return inherited;
}

/* fieldref - find field name of type ty, return entry */
Field fieldref(name, ty) char *name; Type ty; {
     Field p;

     if ((p = isfield(name, unqual(ty)->u.sym->u.s.flist)) != NULL) {
	  if (xref) {
	       Symbol q;
	       assert(unqual(ty)->u.sym->u.s.ftab);
	       q = lookup(name, unqual(ty)->u.sym->u.s.ftab);
	       assert(q);
	       use(q, src);
	  }
	  return p;
     }
     if (Xflag)
	  for (p = unqual(ty)->u.sym->u.s.flist; p; p = p->link) {
	       Field q;
	       if (p->name == NULL && isstruct(p->type)
		   && (q = fieldref(name, p->type)) != NULL) {
		    static struct field f;
		    f = *q;
		    f.offset = p->offset + q->offset;
		    return &f;
	       }
	  }
     return 0;
}

/* ftype - return a function type for rty function (ty,...)' */
Type ftype(rty, ty) Type rty, ty; {
     List list = append(ty, NULL);

     list = append(voidtype, list);
     return func(rty, ltov(&list, PERM), NULL, 0);
}

/* isfield - if name is a field in flist, 
   return pointer to the field structure */
static Field isfield(name, flist) char *name; Field flist; {
     for ( ; flist; flist = flist->link)
	  if (flist->name == name)
	       break;
     return flist;
}
/* outtype - output type ty */
void outtype(ty) Type ty; {
     switch (ty->op) {
     case CONST+VOLATILE:
	  print("%k %k %t", CONST, VOLATILE, ty->type);
	  break;
     case CONST: case VOLATILE:
	  print("%k %t", ty->op, ty->type);
	  break;
     case STRUCT: case UNION: case ENUM:
	  assert(ty->u.sym);
	  if (ty->size == 0)
	       print("incomplete ");
	  assert(ty->u.sym->name);
	  if (*ty->u.sym->name >= '1' && *ty->u.sym->name <= '9') {
	       Symbol p = findtype(ty);
	       if (p == 0)
		    print("%k defined at %w", ty->op, &ty->u.sym->src);
	       else
		    print(p->name);
	  } else {
	       print("%k %s", ty->op, ty->u.sym->name);
	       if (ty->size == 0)
		    print(" defined at %w", &ty->u.sym->src);
	  }
	  break;
     case VOID: case FLOAT: case DOUBLE:
     case CHAR: case SHORT: case INT: case UNSIGNED:
	  print(ty->u.sym->name);
	  break;
     case CSPEC:
	  print("cspec of %t", ty->type);
	  break;
     case VSPEC:
	  print("vspec of %t", ty->type);
	  break;
     case POINTER:
	  print("pointer to %t", ty->type);
	  break;
     case FUNCTION:
	  print("%t function", ty->type);
	  if (ty->u.f.proto && ty->u.f.proto[0]) {
	       int i;
	       print("(%t", ty->u.f.proto[0]);
	       for (i = 1; ty->u.f.proto[i]; i++)
		    if (ty->u.f.proto[i] == voidtype)
			 print(",...");
		    else
			 print(",%t", ty->u.f.proto[i]);
	       print(")");
	  } else if (ty->u.f.proto && ty->u.f.proto[0] == 0)
	       print("(void)");

	  break;
     case ARRAY:
	  if (ty->size > 0 && ty->type && ty->type->size > 0) {
	       print("array %d", ty->size/ty->type->size);
	       while (ty->type && isarray(ty->type) 
		      && ty->type->type->size > 0) {
		    ty = ty->type;
		    print(",%d", ty->size/ty->type->size);
	       }
	  } else
	       print("incomplete array");
	  if (ty->type)
	       print(" of %t", ty->type);
	  break;
     case VALIST:
	  assert(cbe.have);
	  print("va_list");
	  break;
     default: assert(0);
     }
}

/* printdecl - output a C declaration for symbol p of type ty */
void printdecl(p, ty) Symbol p; Type ty; {
     switch (p->sclass) {
     case AUTO:
	  fprint(2, "%s;\n", typestring(0, ty, p->name));
	  break;
     case STATIC: case EXTERN:
	  fprint(2, "%k %s;\n", p->sclass, typestring(0, ty, p->name));
	  break;
     case TYPEDEF: case ENUM:
	  break;
     default: assert(0);
     }
}
/* printproto - output a prototype declaration for function p */
void printproto(p, callee) Symbol p, callee[]; {
     if (p->type->u.f.proto)
	  printdecl(p, p->type);
     else {
	  int i;
	  List list = 0, nlist = 0;
	  if (callee[0] == 0)
	       list = append(voidtype, list);
	  else {
	       for (i = 0; callee[i]; i++)
		    list = append(callee[i]->type, list);
	       if (cbe.emit)
		    for (i = 0; callee[i]; i++)
			 nlist = append(callee[i]->name, nlist);
	  }
	  printdecl(p, func(freturn(p->type), ltov(&list, PERM), 
			    nlist ? ltov(&nlist, PERM) : NULL, 0));
     }
}
/* printtype - print details of type ty on fd */
void printtype(ty, fd) Type ty; int fd; {
     switch (ty->op) {
     case STRUCT: case UNION: {
	  Field p;
	  fprint(fd, "%k %s size=%d {\n", ty->op, ty->u.sym->name, ty->size);
	  for (p = ty->u.sym->u.s.flist; p; p = p->link) {
	       fprint(fd, "field %s: offset=%d", p->name, p->offset);
	       if (p->lsb)
		    fprint(fd, " bits=%d..%d",
			   fieldsize(p) + fieldright(p), fieldright(p));
	       fprint(fd, " type=%t ", p->type);
	  }
	  fprint(fd, "}\n");
	  break;
     }
     case ENUM: {
	  int i;
	  Symbol p;
	  fprint(fd, "enum %s {", ty->u.sym->name);
	  for (i = 0; (p = ty->u.sym->u.idlist[i]) != NULL; i++) {
	       if (i > 0)
		    fprint(fd, ",");
	       fprint(fd, "%s=%d", p->name, p->u.value);
	  }
	  fprint(fd, "}\n");
	  break;
     }
     default:
	  fprint(fd, "%t\n", ty);
     }
}

char *fulltypestring(ty, fname) Type ty; char *fname; {
     char *str;

     assert(ty->fullp);
     ty->fullp = 0;

     switch (ty->op) {
     case STRUCT: case UNION: {
	  Field p;
	  
	  str = stringf("%k %s {\n", ty->op, 
			isunnamed(ty) ? "" : ty->u.sym->name);

	  for (p = ty->u.sym->u.s.flist; p; p = p->link) {
	       if (p->lsb)
		    str = stringf("%s %s:%d;\n", str, 
				  typestring(0, p->type, p->name),
				  fieldsize(p));
	       else
		    str = stringf("%s %s;\n", str, 
				  typestring(0, p->type, p->name));
	  }
	  str = stringf("%s} %s", str, fname);
	  break;
     }
     case ENUM: {
	  int i;
	  Symbol p;
	  
	  str = stringf("enum %s {\n", isunnamed(ty) ? "" : ty->u.sym->name);
	  
	  for (i = 0; (p = ty->u.sym->u.idlist[i]) != NULL; i++) {
	       str = stringf("%s%c%s=%d", str, (i > 0) ? ',' : ' ',
			     p->name, p->u.value);
	  }
	  str = stringf("%s} %s", str, fname);
	  break;
     }
     default:
	  str = typestring(0, ty, " ");
     }
     return str;
}

static inline char * scname(int sclass) {
     switch (sclass) {
     case 0: case AUTO:	return "";
     case REGISTER:	return "register";
     case STATIC:	return "static";
     case EXTERN:	return "extern";
     case TYPEDEF:	return "typedef";
     default: assert(0);
     }
     return 0;
}

/* typestring - return ty as C declaration for str, which may be "" */
char *typestring(sc, ty, str) int sc; Type ty; char *str; {
#    define settd do { if (--depth == 0) td = 0; } while(0)

     static int depth = 0;	/* Recursion depth */
     static int td = 0;		/* True if inside a typedef */
     int sc2 = sc;
     Symbol p;

     if (++depth == 1 && sc == TYPEDEF) 
	  td = 1;

     for ( ; ty; ty = ty->type) {
	  switch (ty->op) {
	  case CONST+VOLATILE:
	       if (isindirect(ty->type))
		    str = stringf("%k %k %s", CONST, VOLATILE, str);
	       else {
		    str = stringf("%s %k %k %s", scname(sc2), CONST, VOLATILE,
				  typestring(0, ty->type, str));
		    settd;
		    return str;
	       }
	       break;
	  case CONST: case VOLATILE:
	       if (isindirect(ty->type))
		    str = stringf("%k %s", ty->op, str);
	       else {
		    str = stringf("%s %k %s", scname(sc2), ty->op,
				  typestring(0, ty->type, str));
		    settd;
		    return str;
	       }
	       break;
	  case CSPEC: case VSPEC:
	       if (cbe.have) {
		    settd;
		    return stringf("%s %s %s", scname(sc2),
				   (ty->op == CSPEC ?
				    tcname.Cspec_t : tcname.Local_t),
				   str);
	       } else
		    str = stringf(isarray(ty->type) || isfunc(ty->type) ? 
				  "(%k %s)" : "%k %s", ty->op, str);
	       break;
	  case STRUCT: case UNION: case ENUM:
	       assert(ty->u.sym);
	       if (ty->fullp) {
		    str = stringf("%s %s", scname(sc2),
				  fulltypestring(ty, str));
		    settd;
		    return str;
	       }
	       if ((p = findtype(ty)) != NULL && sc != TYPEDEF
		   && (isunnamed(ty) /* || !td */)) {
		    settd;
		    return *str ? 
			 stringf("%s %s %s", scname(sc2), p->name, str) : 
			 stringf("%s %s", scname(sc2), p->name);
	       }
	       if (isunnamed(ty))
		    warning("unnamed %k in prototype\n", ty->op);
	       settd;
	       if (*str)
		    return stringf("%s %k %s %s", 
				   scname(sc2), ty->op, ty->u.sym->name, str);
	       else
		    return stringf("%s %k %s", 
				   scname(sc2), ty->op, ty->u.sym->name);
	  case VOID: case FLOAT: case DOUBLE:
	  case CHAR: case SHORT: case INT: case UNSIGNED: {
	       settd;
	       return *str ? 
		    stringf("%s %s %s", scname(sc2), ty->u.sym->name, str) : 
		    stringf("%s %s", scname(sc2), ty->u.sym->name);
	  }
	  case POINTER:
	       if (unqual(ty->type)->op != CHAR && (p = findtype(ty)) != NULL
		   && sc != TYPEDEF
		   && (isstruct(ty->type) && isunnamed(ty->type) /*|| !td*/)) {
		    settd;
		    return *str ? 
			 stringf("%s %s %s", scname(sc2), p->name, str) : 
			 stringf("%s %s", scname(sc2), p->name);
	       }
	       str = stringf(isarray(ty->type) || isfunc(ty->type) ? "(*%s)" :
			     "*%s", str);
	       break;
	  case FUNCTION:
	       if ((p = findtype(ty)) != NULL && sc != TYPEDEF
		   && (isstruct(ty->type) && isunnamed(ty->type) /*|| !td*/)) {
		    settd;
		    return *str ? 
			 stringf("%s %s %s", scname(sc2), p->name, str) : 
			 stringf("%s %s", scname(sc2), p->name);
	       }
	       if (ty->u.f.proto == 0)
		    str = stringf("%s()", str);
	       else if (ty->u.f.proto[0]) {
		    int i;
		    char *n;

		    if (!cbe.emit || ty->u.f.pname == NULL)
			 n = "";
		    else {
			 n = ty->u.f.pname[0]; assert(n);
			 n = (*n < '0' || *n > '9') ? n : "";
		    }
		    str = stringf("%s(%s", str, 
				  typestring(0, ty->u.f.proto[0], n));
		    for (i = 1; ty->u.f.proto[i]; i++)
			 if (ty->u.f.proto[i] == voidtype)
			      str = stringf("%s, ...", str);
			 else {
			      if (!cbe.emit || ty->u.f.pname == NULL)
				   n = "";
			      else {
				   n = ty->u.f.pname[i]; assert(n);
				   n = (*n < '0' || *n > '9') ? n : "";
			      }
			      str = stringf("%s, %s", str, 
					    typestring(0, 
						       ty->u.f.proto[i], n));
			 }
		    str = stringf("%s)", str);
	       } else
		    str = stringf("%s(void)", str);
	       break;
	  case ARRAY:
	       if ((p = findtype(ty)) != NULL && sc != TYPEDEF
		   && (isstruct(ty->type) && isunnamed(ty->type) /*|| !td*/)) {
		    settd;
		    return *str ? 
			 stringf("%s %s %s", scname(sc2), p->name, str) : 
			 stringf("%s %s", scname(sc2), p->name);
	       }
	       if (ty->size > 0 && ty->type && ty->type->size > 0)
		    str = stringf("%s[%d]", str, ty->size/ty->type->size);
	       else
		    str = stringf("%s[]", str);
	       break;
	  case VALIST:
	       assert(cbe.have);
	       return stringf("va_list %s", str);
	  default: assert(0);
	  }
	  sc = 0;
     }
     assert(0); return 0;
}

/*
 * Routines to print vcode type suffixes
 */

char tfDense(t) Type t; {
     switch(t->op) {
     case CHAR:	    return 'c';
     case SHORT:    return 's';
     case INT:   
     case ENUM:     return 'i';
     case UNSIGNED: return 'u';
     case FLOAT:    return 'f';
     case DOUBLE:   return 'd';
     case POINTER:  return 'p';
     case VOID: 
     case STRUCT:
     default:
	  assert(0);
     }
     return 0;
}
char tfSparse(t) Type t; {
     switch(t->op) {
     case CHAR:
     case SHORT: 
     case INT:   
     case ENUM:     return 'i';
     case UNSIGNED: return 'u';
     case FLOAT:    return 'f';
     case DOUBLE:   return 'd';
     case ARRAY:
     case POINTER:  return 'p';
     case STRUCT:/* This is a bit of a hack: see emitasm */
	  return 'i';
     case CSPEC: case VSPEC: return tfSparse(t->type);
     default:
	  assert(0);
     }
     return 0;
}
char tfCaps(t) Type t; {
     switch(t->op) {
     case CHAR:
     case INT:   
     case SHORT: 
     case ENUM:
     case VOID:    return 'I';
     case UNSIGNED:return 'U';
     case FLOAT:   return 'F';
     case DOUBLE:  return 'D';
     case STRUCT:  /* is it ok for struct to be 'p' ? */
     case POINTER: return 'P';
     }
     return 0;
}
char* tfVerbose(t) int t; {
     switch (t) {
     case CHAR:	    return "I_C";
     case ENUM:
     case INT: 	    return "I_I";
     case SHORT:    return "I_S";
     case UNSIGNED: return "I_U";
     case FLOAT:    return "I_F";
     case DOUBLE:   return "I_D";
     case STRUCT:   return "I_B";
     case VOID: 
     case POINTER:  return "I_P";
     }
     return 0;
}

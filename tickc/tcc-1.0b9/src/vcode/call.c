/***********************************************************************
 * Call management.
 */
#define V_MAKE_REG_MAP
#include <stdio.h>
#include "vcode-internal.h"
#include "demand.h"


/* Allocate a register argument. */
static void v_arg(struct v_cstate *c, int type, v_reg_type r) {
	struct v_arg a;

	a.isreg_p = 1;
	a.type = type;
	a.u.r = r;

	v_mach_push(c, type, &a);
}

/* Allocate an immediate argument: poor-man's polymorphism. */
#define ARGI(c, t, imm) do {				\
	struct v_arg a;					\
	a.isreg_p = 0;					\
	a.type = t;					\
	a.u.imm = imm;					\
	v_mach_push(c, t, &a);				\
} while(0)

/* 
 * The different argument types. We have seperated these routines
 * to allow better type checking on immediates.  Once they were
 * provided for immediates, it seemed unnatural to not provide them
 * for registers.
 */
void v_push_argi(struct v_cstate *c, v_reg_type r) 	
	{ v_arg(c, V_I, r); }
void v_push_argii(struct v_cstate *c, int i) 	
	{ ARGI(c, V_I, i); }
void v_push_argu(struct v_cstate *c, v_reg_type r) 	
	{ v_arg(c, V_U, r); }
void v_push_argui(struct v_cstate *c, unsigned u) 	
	{ ARGI(c, V_U, u); }
void v_push_argl(struct v_cstate *c, v_reg_type r) 	
	{ v_arg(c, V_L, r); }
void v_push_argli(struct v_cstate *c, long l) 	
	{ ARGI(c, V_L, l); }
void v_push_argul(struct v_cstate *c, v_reg_type r)  
	{ v_arg(c, V_UL, r); }
void v_push_arguli(struct v_cstate *c, unsigned long ul) 
	{ ARGI(c, V_UL, ul); }
void v_push_argp(struct v_cstate *c, v_reg_type r) 	
	{ v_arg(c, V_P, r); }
void v_push_argpi(struct v_cstate *c, void *p) 	
	{ ARGI(c, V_P, p); }
void v_push_argf(struct v_cstate *c, v_reg_type r) 	
	{ v_arg(c, V_F, r); }
void v_push_argfi(struct v_cstate *c, float f) 	
	{ ARGI(c, V_F, f); }
void v_push_argd(struct v_cstate *c, v_reg_type r) 	
	{ v_arg(c, V_D, r); }
void v_push_argdi(struct v_cstate *c, double d) 	
	{ ARGI(c, V_D, d); }

/* generic push. */
void v_arg_push(struct v_cstate *c, int type, v_reg_type r) {
	v_arg(c, type, r);
}

static void ccall(struct v_cstate *c, void (*ptr)()) {
	v_mach_call(c, ptr);
}

inline void v_ccallv(struct v_cstate *c, v_vptr ptr) { 
	ccall(c, ptr); 
}

inline v_reg_type v_ccalli(struct v_cstate *c, v_iptr ptr) {
	ccall(c, (v_vptr)ptr);
	return v_caller_int_rr;
}

inline v_reg_type v_ccallu(struct v_cstate *c, v_uptr ptr) {
	return v_ccalli(c, (v_iptr)ptr);
}

inline v_reg_type v_ccallp(struct v_cstate *c, v_pptr ptr) {
	return v_ccalli(c, (v_iptr)ptr);
}

inline v_reg_type v_ccalll(struct v_cstate *c, v_lptr ptr) {
	return v_ccalli(c, (v_iptr)ptr);
}

inline v_reg_type v_ccallul(struct v_cstate *c, v_ulptr ptr) {
	return v_ccalli(c, (v_iptr)ptr);
}

inline v_reg_type v_ccallf(struct v_cstate *c, v_fptr ptr) {
	ccall(c, (v_vptr)ptr);
	return v_fp_rr;
}

inline v_reg_type v_ccalld(struct v_cstate *c, v_dptr ptr) {
	ccall(c, (v_vptr)ptr);
	return v_fp_rr;
}

/*
 * Machinery for simple calls.
 */

/* 
 * A bit of syntactic sugar: all scall does is pull the arguments off
 * of the stack and then call ccall. 
 */
static void scall(void (*ptr)(), char *fmt, va_list ap) {
	int nargs, i, *arglist;
	struct v_cstate c;

	if(!v_pseudo_call && v_isleaf)
		v_fatal("cannot do a call from a leaf!\n");


	v_calls++;
	arglist = v_xlatel(fmt, &nargs);
	
	if(nargs >= V_MAXARGS)
		v_fatal("scall: too many arguments. Maximum is %d, gave %d\n", 
					V_MAXARGS, nargs);
	v_push_init(&c);

	/* push all arguments */
	for(i = 0; i < nargs; i++) {
		if(!(arglist[i] & V_IMMEDIATE))
		  v_arg_push(&c, arglist[i], va_arg(ap, v_reg_type));
		else {
			/* mask out the immediate flag */
			switch(arglist[i] & ~V_IMMEDIATE) {
			case V_UC:
			case V_US:
			case V_U:   v_push_argui(&c, va_arg(ap, unsigned)); break;
			case V_C:
			case V_S:
			case V_I:   v_push_argii(&c, va_arg(ap, int)); break;
			case V_L:   v_push_argli(&c, va_arg(ap, long)); break;
			case V_UL:  v_push_arguli(&c, va_arg(ap, unsigned long)); break;
			case V_P:   v_push_argpi(&c, va_arg(ap, void *)); break;
			case V_F:   v_push_argfi(&c, (float)va_arg(ap, double)); break;
			case V_D:   v_push_argdi(&c, va_arg(ap, double)); break;
			default:    v_fatal("v_scall*: bogus type\n"); 
			}
		}
	}
	ccall(&c, ptr);
}


void v_scallv(v_vptr ptr, char *fmt, ...) {
        va_list ap;

        va_start(ap, fmt);
        scall((v_vptr)ptr, fmt, ap);
        va_end(ap);
}

v_reg_type v_scalli(v_iptr ptr, char *fmt, ...) {
        va_list ap;
        va_start(ap, fmt);
        scall((v_vptr)ptr, fmt, ap);
        va_end(ap);
        return v_caller_int_rr;
}
v_reg_type v_scallu(v_uptr ptr, char *fmt, ...) {
        va_list ap;
        va_start(ap, fmt);
        scall((v_vptr)ptr, fmt, ap);
        va_end(ap);
        return v_caller_int_rr;
}

v_reg_type v_scallp(v_pptr ptr, char *fmt, ...) {
        va_list ap;
        va_start(ap, fmt);
        scall((v_vptr)ptr, fmt, ap);
        va_end(ap);
        return v_caller_int_rr;
}

v_reg_type v_scallul(v_ulptr ptr, char *fmt, ...) {
        va_list ap;
        va_start(ap, fmt);
        scall((v_vptr)ptr, fmt, ap);
        va_end(ap);
        return v_caller_int_rr;
}

v_reg_type v_scalll(v_lptr ptr, char *fmt, ...) {
        va_list ap;
        va_start(ap, fmt);
        scall((v_vptr)ptr, fmt, ap);
        va_end(ap);
        return v_caller_int_rr;
}
v_reg_type v_scallf(v_fptr ptr, char *fmt, ...) {
        va_list ap;
        va_start(ap, fmt);
        scall((v_vptr)ptr, fmt, ap);
        va_end(ap);
        return v_fp_rr;
}

v_reg_type v_scalld(v_dptr ptr, char *fmt, ...) {
        va_list ap;
        va_start(ap, fmt);
        scall((v_vptr)ptr, fmt, ap);
        va_end(ap);
        return v_fp_rr;
}

void v_fatal(char *fmt, ...) {
	va_list ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	exit(1);
}

/* Map a symbolic register name to a physical register name. */
v_reg_type v_sym_to_phys(char *r) {
	struct reg_map *rm;

	if(!r)
		v_fatal("v_sym_to_phys: register name must be non-nil.\n");
		
	for(rm = sym_to_phys; rm->sym_name; rm++)
		if(strcmp(rm->sym_name,  r) == 0)
			return v_reg(rm->phys_name);

	v_fatal("v_sym_to_phys: unrecognized register name %s\n", r);
	/*NOTREACHED*/
	return v_reg(0);	/* to shut up -Wall */
}

void v_rccallv(struct v_cstate *c, v_reg_type r) {
     v_calls++;
     v_rmach_call(c, r);
}

inline v_reg_type v_rccalli(struct v_cstate *c, v_reg_type r) {
     v_calls++;
     v_rmach_call(c, r);
     return v_caller_int_rr;
}

v_reg_type v_rccallu(struct v_cstate *c, v_reg_type r) {
     return v_rccalli(c, r);
}

v_reg_type v_rccallp(struct v_cstate *c, v_reg_type r) {
     return v_rccalli(c, r);
}

v_reg_type v_rccalll(struct v_cstate *c, v_reg_type r) {
     return v_rccalli(c, r);
}

v_reg_type v_rccallul(struct v_cstate *c, v_reg_type r) {
     return v_rccalli(c, r);
}

inline v_reg_type v_rccallf(struct v_cstate *c, v_reg_type r) {
     v_calls++;
     v_rmach_call(c, r);
     return v_fp_rr;
}

v_reg_type v_rccalld(struct v_cstate *c, v_reg_type r) {
     return v_rccallf(c, r);
}

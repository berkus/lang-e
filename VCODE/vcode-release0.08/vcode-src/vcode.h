#ifndef __VCODE_H__
#define __VCODE_H__


/************************************************************************
 *
 * Exported vcode macros.
 *
 */

/* Useful macro used to shift a 1 into register x's position.  */
#define V_R(x) (1 << V_REG(x))
/* Prefered way of accessing the vreg register field. */
#define V_REG(r) ((r).reg)

/* 
 * Macro's to get around the safety net (see the .md files for definitions and
 * more discussion).
 */

/*
 * v_schedule_delay: schedule the insn in the branch delay slot.
 * insn must not depend on being executed before or after the
 * branch.
 *      branch - a branch macro invocation (e.g., `v_bnei(r1,r2,label)).
 *      insn - an instruction macro
 */

/*
 * v_raw_load:	Perform a memory operation without hardware interlocks for n 
 * cycles.  If the load latency is greater than n cycles, the backend will insert
 * them.  
 *	mem - macro
 *	n - number of cycles allowed before interlock is required.
 */


/************************************************************************
 *
 * Exported vcode enums.
 *
 */

/* 
 * The three register classes: TEMP (destroyed by function calls), VAR 
 * (persistent across function calls) and UNAVAIL (a bit bucket used to
 * remove registers from active use).
 */
enum { V_TEMP = 20, V_VAR, V_UNAVAIL };

/* vcode types */
enum {
        V_C,    /* char */
        V_UC,   /* unsigned char */
        V_S,    /* short */
        V_US,   /* unsigned short */
        V_I,    /* int */
        V_U,    /* unsigned */
        V_L,    /* long */
        V_UL,   /* unsigned long */
        V_P,    /* pointer */
        V_F,    /* floating */
        V_D,    /* double */
	V_V,	/* void */
        V_B,    /* block structure */
        V_ERR,   /* error condition */

	V_REGISTER  = (1 << 5), 	/* must not conflict with any types */
	V_IMMEDIATE = (1 << 6),		/* must not conflict with any types */
	V_MEMORY = (1 << 7)		/* must not conflict with any types */
	
};

/* non-leaf and leaf values (passed to v_lambda). */
enum { V_NLEAF, V_LEAF };


/************************************************************************
 *
 * Exported vcode data structures.
 *
 */

/*
 * v_reg: register structure.  Basically, we use a structure to 
 * get type checking out of the macros (i.e., to ensure bald integers
 * will not be silently treated as registers).  The downside of this
 * trick is that some compilers are stupid and don't allocate registers
 * to word-sized stuctures.  A future version may make the v_reg type
 * parameterized.
 */
#ifndef __fast__
	typedef struct { 
		/* this leads to the fastest code (no conversions) */
		unsigned reg; 	
	} v_reg_type;
	typedef v_reg_type v_reg_t;

	/* V_Reg_type_Reference_Pointer */
#	define _vrrp(r) ((r)->reg)
	/* V_Reg_type_Reference */
#	define _vrr(r) ((r).reg)
	/* V_Reg_Init */
#	define _vri(r) { (r) }

	extern v_reg_type v_reg(int reg);

#else /* __fast__ */

	typedef unsigned v_reg_type;
	typedef unsigned v_reg_t;
	/* V_Reg_type_Reference_Pointer */
#	define _vrrp(r) (*(r))
	/* V_Reg_type_Reference */
#	define _vrr(r) (r)
	/* V_Reg_Init */
#	define _vri(r) (r)
#	define v_reg(reg) (reg)

#endif

struct v_cstate;

/* 
 * Calling convention. This structure is site-specific.  
 * (Should be moved to a different header file). 
 */
typedef struct {
        int type; 	/* Type of register.  if V_V, is on the stack.  */

	/* union: either holds a stack location or a register. */
	union {
        	struct { 
			short offset;           /* offset */
        		v_reg_type base;	/* base register */
		} mem;
		v_reg_type reg;			/* register */
	} u;
} v_carg;

/* vcode type union */
union v_types {
        v_reg_type r;

        char c;
        unsigned char uc;
        short s;
        unsigned short us;
        int i;
        unsigned u;
        long l;
        unsigned long ul;
        void *p;
        float f;
        double d;
};

/*
 * v_label: label structure.  Holds the value of the label.
 */
#ifndef __fast__

	/* Label type. */
	typedef struct { unsigned short label; } v_label_type;
	typedef v_label_type v_label_t;

	/* V_Label_Reference_Pointer */
#	define _vlrp(l) ((l)->label)
	/* V_Label_Reference */
#	define _vlr(l) ((l).label)

#else /* __fast__ */

	/* Label type. */
	typedef unsigned short v_label_type;
	typedef unsigned short v_label_t;

	/* V_Label_Reference_Pointer */
#	define _vlrp(l) (*(l))
	/* V_Label_Reference */
#	define _vlr(l) (l)
#endif


/* 
 * Each possible function pointer type (ignoring parameters).
 * NOTE: to prevent C's default argument promotion these
 * must be cast to having the correct parameter types (e.g.,
 * char, short, float).
 */

typedef unsigned v_code;	/* base instruction type */

typedef void (*v_vptr)();		/* V */
typedef char (*v_cptr)();		/* C */
typedef unsigned char (*v_ucptr)();	/* UC */
typedef short (*v_sptr)();		/* S */
typedef unsigned short (*v_usptr)();	/* US */
typedef int (*v_iptr)();		/* I */
typedef unsigned (*v_uptr)();		/* U */
typedef long	(*v_lptr)();		/* L */
typedef unsigned long (*v_ulptr)();	/* UL */
typedef void *(*v_pptr)();		/* P */
typedef float (*v_fptr)();		/* F */
typedef double (*v_dptr)();		/* D */


/* 
 * v_fp: union of all vcode function pointer types.  Returned
 * by v_end.
 */
union v_fp {	
	v_vptr	v;
	v_ucptr	uc;
	v_cptr	c;
	v_usptr	us;
	v_sptr	s;
	v_iptr	i;
	v_uptr	u;
	v_lptr	l;
	v_ulptr	ul;
	v_pptr	p;
	v_fptr	f;
	v_dptr	d;
};

/* 
 * Well-known register functions (stack, frame-pointer, etc.). 
 * These should not be public, but are needed by the vcode macros.
 */
extern const v_reg_type	
			v_dat,/* double register available for backend computations */
			v_at, /* integer register available for backend computations*/
			v_caller_int_rr,	/* return integer result */
			v_callee_int_rr,	/* return integer result */
			v_fp_rr, /* fp return register */
			v_zero,
			v_lp;	/* local pointer */
extern const unsigned 	v_lpr;	/* register of local pointer */

/* instruction pointer */
extern v_code *v_ip;

#include "vcode-config.h"

/* Other compilers may have inline: alter accordingly. */
#if  !defined(__GNUC__) || defined(__STRICT_ANSI__)
#       define inline
#endif


/*****************************************************************************
 *
 *  Exported vcode function interface.  
 *
 */

/* 
 * v_lambda: called to begin creating a function at runtime.
 *	name - name of the function.
 *	fmt  - printf-style format string giving the types of incoming 
 *	       	parameters
 *	args - pointer to list of register cells corresponding to each
 *	 	incoming parameter.  v_lambda fills in the associated 
 *		register value.
 *	leaf - boolean that tells vcode whether the function to create is
 *		a leaf or not.
 *	ip   - pointer to data vector that can be used to hold instructions.
 *		Currently, the vector must be ``big enough'' to hold all
 *		the generated instructions.
 *	nbytes - size of the storage pointed to by ip.
 */
int v_lambda(char *name, char *fmt, v_reg_type *args, int leaf, v_code *ip, int nbytes);

/*
 * v_clambda: a more complex lambda.  Identical to v_lambda in functionality,
 * except for the fact that its incoming parameter list is built up 
 * at runtime using v_param_alloc.
 */
void v_clambda(char *name, int leaf, v_code *ip, int nbytes);

/*
 * v_param_alloc: used to allocate parameters on the fly. 
 *	argno - the parameter number that is to be allocated.
 *	type - type of the parameter.
 *	r - pointer to a register structure to fill in.
 */
void v_param_alloc(int argno, int type, v_reg_type *r);

/*
 * v_param_push: identical to v_param_alloc except that the
 * argument number is implicit.  
 *
 * NOTE: Since the arguments are pushed in order it is possible
 * to allocate the argument register, allowing it to be targeted.
 * We don't do this.
 */
void v_param_push(int type, v_reg_type *r);

/*
 * v_end: called to close a v_lambda invocation.  It returns a pointer to
 * the created function.
 *
 *	nbytes - number of bytes required to hold the code.
 */
union v_fp v_end(int *nbytes);

/*
 * v_scall: used to generate a call to a function at runtime.  It
 * returns a vcode register of the specified type that will hold
 * the return value after the call.
 * 	ptr - address of the function to call.
 *	fmt - printf-style format string giving the types of outgoin
 *		parameters.
 *	... - variadic list of argument registers to send as parameters.
 *		scall copies into the required registers (or onto the
 *		stack) as dictated by the native calling conventions.
 */

/* Each flavor of simple call.  Returns a register of the given type. */
void v_scallv(v_vptr ptr, char *fmt, ...);
v_reg_type v_scalli(v_iptr ptr, char *fmt, ...);
v_reg_type v_scallu(v_uptr ptr, char *fmt, ...);
v_reg_type v_scallp(v_pptr ptr, char *fmt, ...);
v_reg_type v_scallul(v_ulptr ptr, char *fmt, ...);
v_reg_type v_scalll(v_lptr ptr, char *fmt, ...);
v_reg_type v_scallf(v_fptr ptr, char *fmt, ...);
v_reg_type v_scalld(v_dptr ptr, char *fmt, ...);

/*
 * v_ccall: a more complex call that allows its argument list
 * to be built up dynamically.  
 *	ptr - function pointer for the routine to call.
 */
void v_ccallv(struct v_cstate *c, v_vptr ptr);
v_reg_type v_ccalli(struct v_cstate *c, v_iptr ptr);
v_reg_type v_ccallu(struct v_cstate *c, v_uptr ptr);
v_reg_type v_ccallp(struct v_cstate *c, v_pptr ptr);
v_reg_type v_ccalll(struct v_cstate *c, v_lptr ptr);
v_reg_type v_ccallul(struct v_cstate *c, v_ulptr ptr);
v_reg_type v_ccallf(struct v_cstate *c, v_fptr ptr);
v_reg_type v_ccalld(struct v_cstate *c, v_dptr ptr);

/*
 * v_rccall: like v_ccall, but the pointer of the routine
 * to call is stored in a register, r
 */

void v_rccallv(struct v_cstate *c, v_reg_type r);
v_reg_type v_rccallr(struct v_cstate *c, v_reg_type r);

/*
 * v_arg_push: identical to v_arg_alloc except that the argument
 * position is implicit. (Don't make this a macro.)
 */
void v_arg_push(struct v_cstate *c, int type, v_reg_type r);


/*
 * v_push_arg*: used to allocate parameters on the fly.  Identical
 * to v_arg* (which they call) except that the argument number
 * is implicit.  The standard vcode naming conventions apply.
 *	r     - register to send
 * Immediates are named using the standard convention:
 *	i - integer
 * 	u - unsigned etc.
 */

/* Initialize a push context. */
void v_push_init(struct v_cstate *c);

void v_push_argi(struct v_cstate *c, v_reg_type r);
void v_push_argii(struct v_cstate *c, int i);
void v_push_argu(struct v_cstate *c, v_reg_type r);
void v_push_argui(struct v_cstate *c, unsigned u);
void v_push_argl(struct v_cstate *c, v_reg_type r);
void v_push_argli(struct v_cstate *c, long l);
void v_push_argul(struct v_cstate *c, v_reg_type r);
void v_push_arguli(struct v_cstate *c, unsigned long ul);
void v_push_argp(struct v_cstate *c, v_reg_type r);
void v_push_argpi(struct v_cstate *c, void *p);
void v_push_argf(struct v_cstate *c, v_reg_type r);
void v_push_argfi(struct v_cstate *c, float f);
void v_push_argd(struct v_cstate *c, v_reg_type r);
void v_push_argdi(struct v_cstate *c, double d);


/* 
 * v_local: allocate space for local scalers (local) and blocks (localb). 
 *	type - standard vcode type specifying the type of the 
 *		scaler to create.
 *	sz - size of the block data to create.
 */
int v_local(int type);
int v_localb(unsigned sz);

/* Label routines. */

/*
 * v_genlabel: generate a valid label.  The generated label is returned.
 */
v_label_type v_genlabel(void);

/*
 * v_labeleq: are the two labels equal? 
 */
int v_labeleq(v_label_t l1, v_label_t l2);

/*
 * v_label: marks the current instruction stream position as the location of
 * the given label.
 */ 
void v_label(v_label_type l);

/*
 * v_dlabel: 
 */
void v_dlabel(void *addr, v_label_type l);

/* Register allocation and deallocation. */

/*
 * v_getreg: allocate a register.  Returns 0 on failure, 1 on success.
 *	r - pointer to the register structure to fill in.
 *	type - type of the register (given below: V_I, V_U, etc.).
 *	class - class of register: either variable or temporary.
 */
int v_getreg(v_reg_type *r, int type, int class);

/*
 * v_putreg: free a register allocated by v_getreg.
 *	r - register to free.
 *	type - type of the register (V_I, V_U, etc.)
 */
void v_putreg(v_reg_type r, int type);

/* 
 * v_save: save register of given type.
 *	r - register to save.
 */
void v_saveu(v_reg_type r);
void v_savel(v_reg_type r);
void v_saveul(v_reg_type r);
void v_savep(v_reg_type r);
void v_savei(v_reg_type r);
void v_savef(v_reg_type r);
void v_saved(v_reg_type r);

/* 
 * v_restore: restore register of given type.
 *	r - register to restore.
 */
void v_restoreu(v_reg_type r);
void v_restorel(v_reg_type r);
void v_restoreul(v_reg_type r);
void v_restorep(v_reg_type r);
void v_restorei(v_reg_type r);
void v_restoref(v_reg_type r);
void v_restored(v_reg_type r);

/*
 * v_fatal: semi-public function that prints an error message to
 * stderr and exits.
 */
void v_fatal(char *fmt, ...);

/*
 * v_dump: disassemble and dump the given instruction stream to stdout.
 */
void v_dump(v_code *code);

/*
 * The remaining functions perform machine dependent operations.  They
 * may be nonsensical on some machines.
 */

/*
 * Make a register r of type unavailable for allocation.  Usually used
 * in conjunction with hard-coded physical registers.  Is a fatal
 * error to attempt to make an already unavailable register unavailable.
 */
void v_mk_unavail(int type, v_reg_type r);

/* 
 * Map a symbolic register name to a physical register name. 
 *	r - character string of symbolic register (e.g., "a0", "t0", etc.).
 */
v_reg_type v_sym_to_phys(char *r);

/* Polymorphic instructions. */
void v_pset(int type, v_reg_type rd, union v_types tu);
void v_pmov(int type, v_reg_type rd, v_reg_type rs);
void v_pst(int type, v_reg_type rd, v_reg_type base, int offset);
void v_pld(int type, v_reg_type rd, v_reg_type base, int offset);

/*****
 * Sleazy routines to do very limited incremental linking. 
 * Will behave badly if epilogue code needs to be modified
 * (e.g., AR size changes via v_local, or additional callee-saved
 * registers are allocated).
 */

/* Initiate incremental link: sets code pointer and remembers nybtes.  */
void v_begin_incremental(v_code *ip, int nbytes);
/* Called when code for fragment is complete. */
int v_end_incremental(void);

/******************************************************************************
 * Internal data structures that are needed by macros.
 */

/* ARGH: more stuff needed by the macros. */
void v_jmark(void *addr, v_label_type l);
void v_bmark(void *addr, v_label_type l);
void v_dmark(void *addr, v_label_type l);
void v_lmark(void *addr, v_label_type l);
void v_smark(void *addr, v_label_type l, v_reg_t rd);
/* Mark where a single precision immediate load will occur. */
void v_float_imm(v_reg_type rd, float imm) ;
/* Mark where a double precision immediate load will occur. */
void v_double_imm(v_reg_type rd, double imm);

void v_flushcache(void *ptr, int size);
v_code *v_swapcp(v_code *x);

/* Macro used to cleave registers into floats and everything else. */
#define v_is_fp(type) ((type) <= V_D && (type) >= V_F)

#include "vcode-macros.h"
#include "vcode-portable-insts.h"
#endif /* __VCODE_H__ */

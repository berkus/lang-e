#include <assert.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "demand.h"
#include "vcode-internal.h"
#include "dis-asm.h"

#define MAXLENGTH (1<<23) /* Max length of function that can be disassembled */

v_code *v_ip; 		/* pointer to current instruction stream. */
unsigned v_calls;	/* number of calls made so far. */
int v_ar_size;		/* activation record size. */
static v_code *vptr;
int v_isleaf;		/* is the current procedure a leaf? */

/* machine independent register names */
const v_reg_type
                v_zero  = _vri(_r0),      /* register always returns zero */
                v_sp    = _vri(_sp),      /* stack pointer */
                v_at    = _vri(_at),      /* used by vcode to load constants, etc. */
                v_dat   = _vri(_dat),      /* used by vcode for synthetic fp ops. */
                v_ra    = _vri(_ra),      /* return register */
                v_caller_int_rr = _vri(_v0), /* return register (what caller sees) */
                v_callee_int_rr = _vri(_v0), /* return register (what callee sees) */
                v_fp_rr  = _vri(_f0),     /* fp return register */
                v_fp    = _vri(_sp),      /* frame-pointer */
                v_lp    = _vri(_sp);      /* register to give for local references */

/* Max number of instructions required to load a single-precision immediate. */
const unsigned v_float_imm_insns = 3;
/* Max number of instructions required to load a double-precision immediate. */
const unsigned v_double_imm_insns = 5;

const unsigned  v_lpr   = _sp; /* register to give for local references */

/* classes */
enum { NULREG, IREG, FREG};

/* type and size */
static struct typeinfo { signed char size, align, class; } tinfo[] = {
        { 1, 1, IREG},  /* C */
        { 1, 1, IREG},  /* UC */
        { 2, 2, IREG},  /* S */
        { 2, 2, IREG},  /* US */
        { 4, 4, IREG},  /* I */
        { 4, 4, IREG},  /* U */
        { 4, 4, IREG},  /* UL */
        { 4, 4, IREG},  /* L */
        { 4, 4, IREG},  /* P */
        { 4, 4, FREG},  /* F */
        { 8, 8, FREG},  /* D */
        { -1, 8, IREG}, /* B */
};

/* offsets up or down? */
#define leaf_offset(x) ((v_isleaf) ? -(x) : (x))

static void push_arg(int type, v_reg_type arg, v_reg_type base, int offset);
static void pop_args(int offset);

static int v_lalloc_p; /* did we allocate any locals? */
static int v_saved_p; /* did we save any registers? */

/*****************************************************************************
 * Argument handling.
 *
 * Extremely simplistic at the moment.
 */
struct reg { signed char reg, class; };


/* lifted from lcc */
static struct reg argreg(int argno, int offset, int ty, int ty0) {
        assert((offset&3) == 0);

        if (offset > 12) {
                struct reg tmpreg = { 0, NULREG};
                return tmpreg;
        } else if (argno == 0 && (ty == V_F || ty == V_D)) {
                struct reg tmpreg = { _f12, FREG};
                return tmpreg;
        } else if (argno == 1 && (ty == V_F || ty == V_D)
           && (ty0 == V_F || ty0 == V_D)) {
                struct reg tmpreg = { _f14,  FREG };
                return tmpreg;
        } else if (argno == 1 && ty == V_D) {
                struct reg tmpreg = { _a2, IREG };
                return tmpreg;
        } else {
                struct reg tmpreg;
                tmpreg.reg = (offset/4) + 4;
                tmpreg.class = IREG;
                return tmpreg;
        }
}

/*
 * Move to a register, or store into memory.  We want to defer it
 * for some things.  need to make deference easy.
 */
void v_mach_push(struct v_cstate *c, int ty, struct v_arg *arg) {
        v_reg_type arg_reg;
	struct reg r;
        int offset;
        struct typeinfo *ti;

	demand(c->cookie == V_COOKIE, Uninitialized call state.);

        if(c->argno == 0)
                c->ty0 = ty;

        offset = c->offset;
        ti = &tinfo[ty];

        /*
         * If we have a register allocated for the argument,
         * we are done.  Otherwise we must provide a temporary
         * that it can be temporarily loaded into before a store.
         * We use at or dat depending on the argument type.
         * This is dangerous, but since no offset will be larger
         * than 16 bits, we should never need at to store an argument.
         */
         if(arg->isreg_p)
                arg_reg = arg->u.r;
         else if(ty == V_F || ty == V_D)
                arg_reg = v_dat;
         else
                arg_reg = v_at;

         offset = v_roundup(offset, ti->align);
         r = argreg(c->argno, offset, ty, c->ty0);

         /* If the destination is NULREG, push the arg onto the stack */
        if(r.class == NULREG) {
                /* if it is an immediate,  load it */
                if(!arg->isreg_p)
                        v_pset(ty, arg_reg, arg->u);
                v_pst(ty, arg_reg, v_sp, offset);
        } else if(r.class == ti->class) {
                if(!arg->isreg_p)
                        v_pset(ty, v_reg(r.reg), arg->u);
                else
                        v_pmov(ty, v_reg(r.reg), arg_reg);
         /* have to convert between the two types */
         } else {
                demand(r.class == IREG && ti->class == FREG,bogus type);
               if(!arg->isreg_p)
                        v_pset(ty, arg_reg, arg->u);
		/* Note: r.reg will be undefined for a subsequent instruction
		   however, we do not need a nop since there will be at least
		   a single instruction (i.e., a jal) before it will be used.  */
                if(ty != V_D)
                        movf2i(r.reg, _vrr(arg_reg));
                else
                        movd2i(r.reg, _vrr(arg_reg));
        }
        c->offset = v_roundup(offset + ti->size, 4);
        c->argno++;
}

/* Possibly site-specific initialization. */
void v_push_init(struct v_cstate *c) {
        memset(c, 0, sizeof *c);
	c->cookie = V_COOKIE;
}


/*****************************************************************************
 *
 * Activation record management.  Basically, allocate a space on the stack
 * for the given type (with appropriate alignment).
 */

/* 
 * In the base case, we must be able to generate code without even
 * making a single pass --- this constraint prevents us from accurately
 * computing the amount of memory needed to save floating point and temporary
 * registers as well the maximum number of arguments for any call.  
 * An inelegant, but workable solution is to simply allocate the maximum
 * amount of space that could possibly be required.  This can be a
 * real pain for recursive routines (i.e., they will consume a lot of
 * stack space) but allows very efficient code generation.
 *
 * The AR is laid out as follows:
 *
 *	+-------------------------------+
 *	|	local variables		|
 *	+-------------------------------+
 *	|	saved fp regs		|
 *	+-------------------------------+
 *	| 	saved gp regs		|
 *	+-------------------------------+
 *	| 	arg build area		|
 *	+-------------------------------+	<----- sp
 *
 * Local variables can be allocated at any time, therefore we can only
 * emit the instruction to adjust the stack pointer after all code has
 * been emitted.  (i.e., it must be backpatched).  Additionally,
 * the generated code must have an initial pad so that saves of
 * callee saved registers can be inserted as they are allocated.
 * Finally, the epilogue code must be deferred for the same reason:
 */
#define	FSAVESIZE	(32*4)		/* area to save all fp regs */
#define RSAVESIZE	(32*4)		/* area to save all gp regs */
#define ARGBUILDSIZE	(V_MAXARGS * 8)	/* the maximum number of bytes that
					 * can be consumed by the maximum
					 * number of arguments (basically 
					 * MAXARG doubles). 
					 */
static v_code *prologue_ip; /* Pointer to beginning of space reserved for saving 
			   * callee-saved registers.  */
v_label_type v_epilogue_l;	/* label for epilogue */


/* a simple call */
void v_mach_call(struct v_cstate *c, void (*ptr)()) {

	demand(c->cookie == V_COOKIE, Uninitialized call state.);

	/* call */
	v_jalpi(v_ra, ptr);
	_nop();

	c->cookie = ~V_COOKIE;
}

/* 
 * v_rmach_call: like v_mach_call, but the function pointer is stored in a 
 * register, fr.
 */
void v_rmach_call(struct v_cstate *c, v_reg_type fr) {
	demand(c->cookie == V_COOKIE, Uninitialized call state.);

	v_jalp(v_ra, fr);
	_nop();

	c->cookie = ~V_COOKIE;
}

/* 
 * Procedure management.  mainly shuffles incoming arguments into
 * registers.
 */
void v_mach_lambda(int nargs, int *arglist, v_reg_type *args, int leaf, v_code *ip) {
	int offset, last_mtc1, ty0, i;
	struct reg r;

	/*
	 * We overallocate the space required to save all callee-saved 
	 * registers.
	 */
	ip += NFP_VARS + N_VARS + (leaf == 0);	/* space for ra */
	ip += 1;	/* stack pointer adjustment */
	prologue_ip = ip;

	/* didn't allocate any locals or save any registers. (yet) */
	v_saved_p = v_lalloc_p = 0;

	v_begin(ip);
	v_isleaf = leaf;

	/* allocate a label for the epilogue */
	v_epilogue_l = v_genlabel();

	offset = 0;

	ty0 = *arglist;		/* initial type */

	/*
	 * Due to MIPS bogosity, we have to call a DFA engine to figure 
	 * out which register a given argument goes in.
	 */
	for(i = 0, last_mtc1 = -2; i < nargs; i++, args++, arglist++) {
		struct typeinfo *ti = &tinfo[*arglist];

                offset = v_roundup(offset, ti->align);
                r = argreg(i, offset, *arglist, ty0);


		/* 
		 * If it is in a register of the appropriate type, and we are 
		 * in a leaf, leave it there.  IF it is on the stack, load it
	 	 * into a register.  If it is a register of the wrong type,
		 * move it.
		 */
		if(r.class == NULREG) {
			if(!v_getreg(args, *arglist, leaf ? V_TEMP : V_VAR))
				v_fatal("v_lambda: out of registers");

			/* Record argument to be loaded off of stack. */
			push_arg(*arglist, *args, v_sp, offset);


		} else if(r.class == ti->class) {
			/* we can leave it in place */
			if(v_isleaf)
				*args = v_reg(r.reg);
			else {
				if(!v_getreg(args, *arglist, V_VAR))
					v_fatal("v_lambda: out of registers");

				v_pmov(*arglist, *args, v_reg(r.reg));

				if(*arglist == V_F || *arglist == V_D)
					v_rawput(r.reg, V_TEMPF);
				else
					v_rawput(r.reg, V_TEMPI);
			}
                } else {
			demand(r.class == IREG && ti->class == FREG,bogus type);
			if(!v_getreg(args, *arglist, leaf ? V_TEMP : V_VAR))
				v_fatal("v_lambda: out of registers");
			last_mtc1 = i;
			v_rawput(r.reg, V_TEMPI);
 			if(*arglist != V_D)
				mtc1(_vrrp(args), r.reg);
			else {
				movi2d(_vrrp(args), r.reg);
				v_rawput(r.reg+1, V_TEMPI);
			}
		}
                offset = v_roundup(offset + ti->size, 4);
	}

 	/* allocate space for everything */
	v_ar_size = offset + FSAVESIZE + RSAVESIZE + ARGBUILDSIZE;

	/* 
	 * The final instruction stream is as follows:
	 *	+-------------------------------+
	 *	| insn to adjust stack: constant|
	 *	| is assumed to fit; not done   |
	 *	| for leaf procedures.		|
	 *	+-------------------------------+
	 *	| insns to save callee regs	|
	 *	+-------------------------------+
	 *	| insns to move & load args	|
	 *	+-------------------------------+
	 *	| body of function		|
	 *	+-------------------------------+
	 *	| epilogue: restore registers   |
	 *	+-------------------------------+
	 */

	/* 
	 * Argument registers are managed by us (because on the
	 * sparc and such machines, they do not need to be saved
	 * or restored across function calls.
	 */

	/* 
   	 * Should deallocate the rest of the argument registers.
	 */
}

static int vi, vf;	/* total number of allocated int and fp registers */

/* 
 * We are overconservative at the moment.  We disable leaf optimizations
 * if:
 * 	1. any callee-saved registers were used (have to alloc ar).
 *	2. the function is a non-leaf procedure (have to save->alloc ar).
 *	3. any locals were allocated (have to alloc ar).
 *	4. any registers were saved (have to alloc ar).
 * It is not difficult to make this process more efficient.  Just adds
 * more cases.
 */
int v_restore_p(void) {
	return (vi + vf > 0) || !v_isleaf || v_lalloc_p || v_saved_p;
}

/* mark end: needs to go and emit the instructions to save 
 * all callee registers. */
union v_fp v_mach_end(int *nbytes) { 
	int offset, r;
	v_code *tmp;
	union v_fp fp;
	unsigned restore_p;

	assert_active(v_end);
	vi = v_naccum(V_VARI);		/* get callee-saved registers */
	vf = v_naccum(V_VARF);
	restore_p = v_restore_p();	/* can we special case? */
	tmp = v_ip; 			/* Save old ip for later. */
	offset = ARGBUILDSIZE;	/* get over the argbuild area */

	/* compute the number of instructions needed to save regs */
	vptr = v_ip = prologue_ip - (vi + (v_isleaf == 0) + vf + restore_p);

	/* 
	 * At this point we adjust the stack pointer, save and
	 * restore all registers, and pop arguments. 
	 */

	if(restore_p)
		addiu(_sp, _sp, -v_ar_size); /* Allocate AR */

	/* Need to save return address? */
	if(!v_isleaf)
		sw(_ra, _sp, _ra * 4 + offset);

	for(r = -1; (r = v_reg_iter(r, V_VARI)) >= 0; )
		sw(r, _sp, r * 4 + offset);
	for(r = -1; (r = v_reg_iter(r, V_VARF)) >= 0; )
		/* should track if allocated as single or double */
		sd(r, _sp, r * 4 + RSAVESIZE + offset); 

	/* Load all arguments */
	pop_args(!restore_p ? 0 : v_ar_size);

	/* Done constructing prologue code. */

	v_ip = tmp;		/* reset v_ip to end of client's code */
	v_label(v_epilogue_l);	/* mark where epilogue resides */
	/* link before any instructions have been emitted.  */
	v_link();		

	/* Contruct epilogue code. */

	/* Restore return address */
	if(!v_isleaf)
		lw(_ra, _sp, _ra * 4 + offset);

	/* Restore the rest of the callee's. */
	for(r = -1; (r = v_reg_iter(r, V_VARI)) >= 0; )
		lw(r, _sp, r * 4 + offset);
	for(r = -1; (r = v_reg_iter(r, V_VARF)) >= 0; )
		ld(r, _sp, r * 4 + RSAVESIZE + offset); 


	/* 
	 * If this is a non-leaf procedure, and we restored any registers, 
	 * place the deallocation instruction in the delay slot.
	 */
	if(!restore_p) {
		jr(_ra);
	} else {
		addiu(_sp, _sp, v_ar_size);
		jr(_ra);
	}

	/* 
	 * Bump vptr forward over nop: nops happen for functions that
	 * do not contain any instructions. (I believe this is the
	 * only case.) 
	 */
	if(!vptr)
		vptr++;

	/* pattern of nops looked for by v_dump */
	v_nop();
	v_nop();
	v_nop();
	v_nop();
	v_set_fp_imms(v_ip);

#ifndef __no_bpo__
        if (v_bpop)
	     v_ip = v_bpo(v_ip);
#endif

	*nbytes = (v_ip - vptr) * sizeof *v_ip;
	v_flushcache(vptr, *nbytes);

	fp.v = (v_vptr) vptr;

	return fp;
}

/************************************************************************
 *
 * Instruction flush and dump routines.
 */

void v_flushcache(void *ptr, int nbytes);

/* Dump code pointed to by code. */
void v_dump (v_code *code) {
     struct disassemble_info info;
     unsigned int *p;
     int l;

     assert(sizeof(unsigned int) == 4);

     INIT_DISASSEMBLE_INFO(info, stdout);
     info.buffer = (bfd_byte *)code;
     info.buffer_vma = (bfd_vma)code;
     info.buffer_length = MAXLENGTH;

     printf("disassembled:\n");
     p = (unsigned *)info.buffer;
     while (! (p[0] == 0x1 && p[2] == 0x1 && p[4] == 0x1)) {
	  printf("%p:\t0x%08x 0x%08x\t", p, p[0], p[1]);
#if defined(__SSEL__)
	  l = print_insn_little_ss((bfd_vma)p, &info);
#elif defined(__SSEB__)
	  l = print_insn_big_ss((bfd_vma)p, &info);
#else
#error "Must specify __SSEL__ (little-endian) or __SSEB__ (big-endian)"
#endif
	  if (l <= 0) break;
	  assert(l == 8);
	  printf("\n");
	  p += 2;
     }
}


/*****************************************************************************
 *
 * Activation record management.  Basically, allocate a space on the stack
 * for the given type (with appropriate alignment).
 */

/* allocate space for a scalar */
int v_local(int type) {
        int old_offset;
        struct typeinfo *ti;

        /* need to check that it is a valid type */
        if(type == V_B)
          v_fatal("v_local: must call v_localb to allocate block structures\n");

        ti = &tinfo[type];


	/* make 4 byte alignment minimum. */
        if(ti->align < 4)
                v_ar_size = v_roundup(v_ar_size, 4);
        else
                v_ar_size = v_roundup(v_ar_size, ti->align);

	/* I think we just use v_ar_size.  Why did we ever not use it? */
	old_offset = v_ar_size;

	/* add in the size of the object.  roundup again. */
	v_ar_size += v_roundup(ti->size, 4);
	v_lalloc_p++;		/* allocated a local */ 

        return old_offset; 	/* return previous offset.  */
}

/* allocate space on the stack for a block structure; we 8-byte align */
int v_localb(unsigned sz) {
       int old_offset;

	/* I think we just use v_ar_size.  Why did we ever not use it? */
	old_offset = v_ar_size;

        v_ar_size = v_roundup(v_ar_size, 8);
	old_offset = v_ar_size;
	v_lalloc_p++;		/* allocated a local */ 

	v_ar_size += sz;
        v_ar_size = v_roundup(v_ar_size, 8);

        return old_offset;
}

/**********************************************************************
 * Code to manage arguments: solves the problem of loading args off
 * of the stack when we do not know the AR size.
 *
 */

static struct lazy_arg {
	int type;
	v_reg_type arg;
	v_reg_type base;
	int offset;
} l_args[V_MAXARGS], *lp = &l_args[0];

static v_code *first_arg;

static void push_arg(int type, v_reg_type arg, v_reg_type base, int offset) {
	demand(lp && lp < &l_args[V_MAXARGS], out of arg space);
	lp->type = type;
	lp->arg = arg;
	lp->base = base;
	lp->offset = offset;
	lp++;

	if(!first_arg)
		first_arg = v_ip;

	v_nop();	/* emit a place holder */
	if(type == V_D)	/* need two insns */
		v_nop();
}

/* Put the arguments in the right place. */
static void pop_args(int offset) {
	v_code *tmp;
	int n;
	struct lazy_arg *p, *e;

	if(!(n = lp -  &l_args[0]))
		return;

	tmp = v_ip;
	v_ip = first_arg;
		
	for(e = lp, p = lp = &l_args[0]; p < e; p++) {
		v_pld(p->type, p->arg, p->base, p->offset + offset);
		v_nuke_nop();
	}

	first_arg = 0;
	v_ip = tmp;
}


/*******************************************************************
 *  Allow user to save an restore registers easily.
 *
 */

inline void v_savep(v_reg_type r) { v_savei(r); }
inline void v_saveu(v_reg_type r) { v_savei(r); }
inline void v_savel(v_reg_type r) { v_savei(r); }
inline void v_saveul(v_reg_type r) { v_savei(r); }
inline void v_restorep(v_reg_type r) { v_restorei(r); }
inline void v_restoreu(v_reg_type r) { v_restorei(r); }
inline void v_restorel(v_reg_type r) { v_restorei(r); }
inline void v_restoreul(v_reg_type r) { v_restorei(r); }

extern int v_istemp(v_reg_type r, int type);

/* should ensure that it is indeed a caller-saved register */
void v_savei(v_reg_type r) {
	v_saved_p = 1;
	if(v_istemp(r, V_I))
		v_stii(r, v_sp, ARGBUILDSIZE + _vrr(r) * 4);
}
void v_restorei(v_reg_type r) {
	if(v_istemp(r, V_I))
		v_ldii(r, v_sp, ARGBUILDSIZE + _vrr(r) * 4);
}

void v_savef(v_reg_type r) {
	v_saved_p = 1;
	if(v_istemp(r, V_F))
		v_stfi(r, v_sp, ARGBUILDSIZE + RSAVESIZE + _vrr(r) * 4);
}

void v_restoref(v_reg_type r) {
	if(v_istemp(r, V_F))
		v_ldfi(r, v_sp, ARGBUILDSIZE + RSAVESIZE + _vrr(r) * 4);
}

void v_saved(v_reg_type r) {
	v_saved_p = 1;
	if(v_istemp(r, V_D))
		v_stdi(r, v_sp, ARGBUILDSIZE + RSAVESIZE + _vrr(r) * 4);
}

void v_restored(v_reg_type r) {
	if(v_istemp(r, V_D))
		v_lddi(r, v_sp, ARGBUILDSIZE + RSAVESIZE + _vrr(r) * 4);
}

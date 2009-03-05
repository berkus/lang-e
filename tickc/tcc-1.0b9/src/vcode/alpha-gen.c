#include <assert.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "demand.h"
#include "vcode-internal.h"

/* holds pointer to current instruction stream. */
v_code *v_ip;
unsigned v_calls;

int v_l_offset;
int v_ar_size;

int v_isleaf;
static int v_saved_p; 	/* did we save any registers? */
static int v_lalloc_p; /* did we allocate any locals? */

const v_reg_type      
		v_zero  = _vri(_zero),    /* register always returns zero */
                v_sp    = _vri(_sp),      /* stack pointer */
                v_at    = _vri(_at),      /* used by vcode to load constants, etc. */
                v_dat    = _vri(_f10),    /* used by vcode to load constants, etc. */
                v_ra    = _vri(_ra),      /* return register */
                v_caller_int_rr = _vri(_v0),     /* return register (what caller sees) */
                v_callee_int_rr = _vri(_v0),     /* return register (what callee sees) */
                v_fp_rr  = _vri(_f0),     /* fp return register */
                v_fp    = _vri(_fp),      /* frame-pointer */
                v_lp    = _vri(_sp);      /* register to give for local references */

/* Max number of instructions required to load a single-precision immediate. */
const unsigned v_float_imm_insns = 7;
/* Max number of instructions required to load a double-precision immediate. */
const unsigned v_double_imm_insns = 7;

const unsigned  v_lpr   = _sp; 		/* register to give for local references */

inline void addli(int rd, int rs, int imm);
inline void addqi(int rd, int rs, int imm);
inline void subqi(int rd, int rs, int imm);
inline void subli(int rd, int rs, int imm);

static void push_arg(int type, v_reg_type arg, v_reg_type base, int offset);
static void pop_args(int offset);

int v_carea; 	/* pointer to space to perform int <--> fp conversions. */

/* classes */
enum { NULREG, IREG, FREG};

/* type and size */
static struct typeinfo { signed char size, align, class; } tinfo[] = {
        { 1, 1, IREG},  /* C */
        { 1, 1, IREG},  /* UC */
        { 2, 2, IREG},  /* S */
        { 2, 2, IREG},  /* US */
        { 4, 4, IREG},  /* I */
        { 8, 8, IREG},  /* U */
        { 8, 8, IREG},  /* UL */
        { 8, 8, IREG},  /* L */
        { 8, 8, IREG},  /* P */
        { 4, 4, FREG},  /* F */
        { 8, 8, FREG},  /* D */
        { -1, 8, IREG}, /* B */
};

/*****************************************************************************
 * Argument handling.
 */

struct reg { signed char reg, class; };

/* 
 * Simple calling convention: everything upto 6 is passed in registers
 * (int reg if it is an int, fp if it is fp).  otherwise, on the stack.
 */
static struct reg argreg(int argno, int ty) {
  if(argno >= 6) {
    struct reg tmpreg = { 0, NULREG};
    return tmpreg;
  } else if(ty == V_F || ty == V_D) {
    struct reg tmpreg;
    tmpreg.reg = _f16 + argno;
    tmpreg.class = FREG;
    return tmpreg;
  } else {
    struct reg tmpreg;
    tmpreg.reg = _a0 + argno;
    tmpreg.class = IREG;
    return tmpreg;
  }
}

/* move arg into its proper position. */
void v_mach_push(struct v_cstate *c, int ty, struct v_arg *arg) {
        v_reg_type arg_reg;
        struct reg r;
        struct typeinfo *ti;

        demand(c->cookie == V_COOKIE, Uninitialized call state.);
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

        r = argreg(c->argno, ty);

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

	/* If the destination is NULREG, push the arg onto the stack */
	if(r.class == NULREG) {
		/* if it is an immediate,  first load it */
		if(!arg->isreg_p)
			v_pset(ty, arg_reg, arg->u);
		v_pst(ty, arg_reg, v_sp, c->offset);

		/* modify offset: *** don't miss this */
		c->offset += 8;	/* everything is 8 bytes big */
	} else {
		if(!arg->isreg_p)
			v_pset(ty, v_reg(r.reg), arg->u);
		else
			v_pmov(ty, v_reg(r.reg), arg_reg);
	}
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
 *      +-----------------------------------------------+  high memory
 *      |                                               |
 *      |               nth argument                    |
 *      |                       .                       |
 *      |                       .                       |
 *      |                       .                       |
 *      |               7th argument                    |
 *      +-----------------------------------------------+ <-- virtual fp
 *      |               local and temporaries           |
 *      +-----------------------------------------------+
 *      |               saved registers (ra as well)    |
 *      +-----------------------------------------------+
 *      |               arg build area                  |
 *      +-----------------------------------------------+ <-- sp
 *
 * We adjust the AR as follows:
 *
 *      +-----------------------------------------------+  high memory
 *      |                                               |
 *      |               nth argument                    |
 *      |                       .                       |
 *      |                       .                       |
 *      |                       .                       |
 *      |               7th argument                    |
 *      +-----------------------------------------------+ <-- virtual fp
 *      |               locals				|
 *      +-----------------------------------------------+
 *      |            	fp save area			|
 *      +-----------------------------------------------+
 *	|		int save area 			|
 *      +-----------------------------------------------+
 *      |               arg build area                  |
 *      +-----------------------------------------------+ <-- sp
 *
 *
 * There are three variables that are not known until the entire function
 * has been generated: locals, arg build area and saved registers.
 * The way this is handled is that we allocate the maximum space for
 * saved registers and arg build (making these values constants, 
 * albeit somewhat large ones) and pull out the arguments *before* 
 * adjusting the stack pointer.  With these measures, we can address
 * any point of the AR.
 *
 * Local variables can be allocated at any time, therefore we can only
 * emit the instruction to adjust the stack pointer after all code has
 * been emitted.  (i.e., it must be backpatched).  Additionally,
 * the generated code must have an initial pad so that saves of
 * callee saved registers can be inserted as they are allocated.
 * Finally, the epilogue code must be deferred for the same reason:
 */
#define	FSAVESIZE	(32*8)		/* area to save all fp regs */
#define RSAVESIZE	(32*8)		/* area to save all gp regs */
#define ARGBUILDSIZE	(V_MAXARGS * 8)	/* the maximum number of bytes that
					 * can be consumed by the maximum
					 * number of arguments (basically 
					 * MAXARG doubles). 
					 */

static v_code *prologue_ip; /* Pointer to beginning of space reserved for saving 
			   * callee-saved registers.  */
v_label_type v_epilogue_l;	/* label for epilogue */

/* we do whole 64 bit loads and stores, since the contents of the
 * memory do not have to be preserved. */
void v_savep(v_reg_type r) { v_savel(r); }
void v_saveu(v_reg_type r) { v_savel(r); }
void v_savei(v_reg_type r) { v_savel(r); }
void v_saveul(v_reg_type r) { v_savel(r); }
void v_restorep(v_reg_type r) { v_restorel(r); }
void v_restoreu(v_reg_type r) { v_restorel(r); }
void v_restorei(v_reg_type r) { v_restorel(r); }
void v_restoreul(v_reg_type r) { v_restorel(r); }

/* should ensure that it is indeed a caller-saved register */
void v_savel(v_reg_type r) {
	v_saved_p = 1;
	v_stli(r, v_sp, ARGBUILDSIZE + _vrr(r) * 8);
}
void v_restorel(v_reg_type r) {
	v_ldli(r, v_sp, ARGBUILDSIZE + _vrr(r) * 8);
}
void v_savef(v_reg_type r) {
	v_saved_p = 1;
	v_stfi(r, v_sp, ARGBUILDSIZE + RSAVESIZE + _vrr(r) * 8);
}
void v_restoref(v_reg_type r) {
	v_ldfi(r, v_sp, ARGBUILDSIZE + RSAVESIZE + _vrr(r) * 8);
}

void v_saved(v_reg_type r) {
	v_saved_p = 1;
	v_stdi(r, v_sp, ARGBUILDSIZE + RSAVESIZE + _vrr(r) * 8);
}
void v_restored(v_reg_type r) {
	v_lddi(r, v_sp, ARGBUILDSIZE + RSAVESIZE + _vrr(r) * 8);
}


/* Call given pointer */
void v_mach_call(struct v_cstate *c, void (*ptr)()) {
        demand(c->cookie == V_COOKIE, Uninitialized call state.);
	v_jalpi(v_ra, (void*)ptr); 	/* call */
    	c->cookie = ~V_COOKIE;
}

/*
 * v_rmach_call: like v_mach_call, but the function pointer is stored in a
 * register, fr.
 */
void v_rmach_call(struct v_cstate *c, v_reg_type fr) {
        demand(c->cookie == V_COOKIE, Uninitialized call state.);
        /* v_jalp(fr); */
	v_jalp(v_ra, fr);
        c->cookie = ~V_COOKIE;
}


/* 
 * Procedure management.  mainly shuffles incoming arguments into
 * registers.
 */
void v_mach_lambda(int nargs, int *arglist, v_reg_type *args, int leaf, v_code *ip) {
	int offset, framesize, i;
	struct reg r;

	/*
	 * We overallocate the space required to save all callee-saved 
	 * registers.
	 */
	ip += N_FP_VARS + N_VARS + (leaf == 0);	/* space for ra */
	ip += 1; /* stack pointer adjustment */
	prologue_ip = ip;

	v_begin(ip);
	v_isleaf = leaf;
	v_saved_p = v_lalloc_p = 0;

	/* allocate a label for the epilogue */
	v_epilogue_l = v_genlabel();

	offset = framesize = 0;

	for(i = 0; i < nargs; i++, args++, arglist++) {
                r = argreg(i, *arglist);

		/* 
		 * If it is in a register and we are in a leaf, leave it there.  
		 * If it is on the stack, load it into a register.  In either
		 * case, deallocate argument registers that are not used.
		 */
		if(r.class == NULREG) {
                       if(!v_getreg(args, *arglist, leaf ? V_TEMP : V_VAR))
                                v_fatal("v_lambda: out of registers");

			/* Record argument to be loaded off of stack. */
			push_arg(*arglist, *args, v_sp, offset);

			offset += 8;	/* everything is 8-byte offsets */
		/* floating-point */
		} else if(*arglist == V_F || *arglist == V_D) {
			/* deallocate corresponding integer register */
			v_rawput(_a0 + i, V_TEMPI);

			/* we can leave it in place */
			if(v_isleaf)
				*args = v_reg(r.reg);
			/* have to move it */
			else {
				if(!v_getreg(args, V_D, V_VAR))
                                	v_fatal("v_lambda: out of registers");
				v_pmov(*arglist, *args, v_reg(r.reg));
			}
		/* integer */
                } else {
			/* deallocate corresponding floating point register */
			v_rawput(_fa0 + i, V_TEMPF);

			if(v_isleaf)
				/* we can leave it in place. */
				*args = v_reg(r.reg);
			else {
				/* is not a leaf, we must move it */
				/* deallocate argument register */
				v_rawput(r.reg, V_TEMPI);

				/* allocate argument registers destination */
				if(!v_getreg(args, V_I, V_VAR))
                                	v_fatal("v_lambda: out of registers");
				v_pmov(*arglist, *args, v_reg(r.reg));
			}
		}
	}
 	/* allocate space for everything */
	v_ar_size = /* offset + */ FSAVESIZE + RSAVESIZE + ARGBUILDSIZE;
	/* Allocate space to perform fp -> int; int -> fp conversions. */
	v_carea = 0;


	/* 
	 * The final instruction stream is as follows:
	 *	+-------------------------------+
	 *	| insns to save callee regs	|
	 *	+-------------------------------+
	 *	| insns to move & load args	|
	 *	+-------------------------------+
	 *	| insn to adjust stack: constant|
	 *	| is assumed to fit; not done   |
	 *	| for leaf procedures.		|
	 *	+-------------------------------+
	 *	|				|
	 *	| body of function		|
	 *	+-------------------------------+
	 *	| epilogue: restore registers   |
	 *	+-------------------------------+
	 */

	/* 
	 * Argument registers are managed by us (because on the
	 * sparc and such machines, they do not need to be saved
	 * nor restored across function calls.
	 */

        /* allow incoming parameters to be used as temporaries */
        for(; nargs < 6; nargs++) {
                v_rawput(nargs + _a0, V_TEMPI);
                v_rawput(nargs + _fa0, V_TEMPF);
	}
}

static unsigned vi, vf;      /* total number of allocated int and fp registers */

/* does the function have to restore any registers? */
int v_restore_p(void) {
        return (vi + vf > 0) || !v_isleaf || v_lalloc_p || v_saved_p;
}

/* mark end: needs to go and emit the instructions to save 
 * all callee registers. */
union v_fp v_mach_end(int *nbytes) {
	int offset, r, restore_p;
	v_code *tmp, *vptr;
	union v_fp fp;

        vi = v_naccum(V_VARI);	/* get callee-saved registers */
        vf = v_naccum(V_VARF);

	restore_p = v_restore_p();
	offset = ARGBUILDSIZE;	/* get over the argbuild area */
	tmp = v_ip;

	/* compute the number of instructions needed to save regs */
	vptr = v_ip = prologue_ip - (vi + (v_isleaf == 0) + vf + restore_p);

	/* must be 8 byte alignment */
	v_ar_size = v_roundup(v_ar_size, 8);

	/*
	 * At this point we adjust the stack pointer, save and restore
	 * all registers and pop arguments. 
	 */

	if(restore_p)
		v_addpi(v_sp, v_sp, -v_ar_size);	/* allocate ar */

	/* Save return address? */
	if(!v_isleaf) 
		v_stpi(v_reg(_ra), v_sp, _ra * 8 + offset);

        for(r = -1; (r = v_reg_iter(r, V_VARI)) >= 0; )
                v_stli(v_reg(r), v_sp, r * 8 + offset);
        for(r = -1; (r = v_reg_iter(r, V_VARF)) >= 0; )
                /* should track whether allocated as single or double precision */
                v_stdi(v_reg(r), v_sp, r * 8 + RSAVESIZE + offset);

	/* load all arguments. */
	pop_args(!restore_p ? 0 : v_ar_size);

	/* Done with prologue */

	v_ip = tmp;		/* reset v_ip to end of client's code */
	v_label(v_epilogue_l);	/* mark where epilogue resides */
	v_link();		/* link before any instructions have been emitted. */

	/* restore all callee's */
	if(!v_isleaf)
		v_ldpi(v_reg(_ra), v_sp, _ra * 8 + offset);

        for(r = -1; (r = v_reg_iter(r, V_VARI)) >= 0; )
                v_ldli(v_reg(r), v_sp, r * 8 + offset);
        for(r = -1; (r = v_reg_iter(r, V_VARF)) >= 0; )
                v_lddi(v_reg(r), v_sp, r * 8 + RSAVESIZE + offset);

	/* deallocate ar */
	if(restore_p)
		v_addpi(v_sp, v_sp, v_ar_size);	/* deallocate ar */
	ret();

	/* v_j(v_ra); */

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


/*
 * Call a pseudo-instruction: they preserve all temporary registers.
 * Before calling it we must save a0 and a1 in the argbuild area.
 * the call must store the return address in t9.
 *
 * XXX: we cannot rely on being in a leaf procedure, so we save
 * registers at a negative stack offset.  However, I'm not sure
 * how this will interact with demand stack growth --- the vm
 * system probably determines whether to send a seg fault on
 * where the stack pointer is.  For complete safety we 
 * adjust the stackpointer first (if it is a non-leaf).
 */
void pseudo_call(v_vptr insn, int rd, int rs1, int rs2) {
	if(v_isleaf)
		addqi(_sp, _sp, -48);
        stq(_t10, _sp, 0);     /* save argument registers */
        stq(_t11, _sp, 8);
        stq(_t12, _sp, 16);
        stq(_t9, _sp, 24);

        mov(_t10, rs1);                 /* load arguments */
        mov(_t11, rs2);
        set(_t12, (unsigned long)insn);
        jsr(_t9, _t12, 0);
        mov(rd, _t12);          /* move result into destination */

        ldq(_t10, _sp, 0);     /* restore argument registers */
        ldq(_t11, _sp, 8);
        ldq(_t12, _sp, 16);
        ldq(_t9, _sp, 24);
	if(v_isleaf)
		addqi(_sp, _sp, 48);
}

#if 0
/* 
 * Load a 32-bit immediate. 
 * See a-13.
 */

static inline long get_gp(void) { 
	asm("addq $29, $31, $0");
}
#endif


#define HI32(x) ((unsigned)( (unsigned long)(x) >> 32) ) 

int FITS(unsigned long x) {
	unsigned hi = HI32(x);

	return hi == 0xffffffff || hi == 0; 
}

/* modify this to use gp */
void setu(unsigned Rdst, unsigned long val) {
	void set64u(unsigned Rdst, unsigned long val) ;
        int lo, hi, base_reg;

	if(FITS(val))
		base_reg = _zero;
	else {
		/* I don't think we can rely on the gp */
		set64u(Rdst, val);
		return;
	}

        lo = (unsigned)val & 0xffff;
        hi = ((val >> 16)  + ((val >> 15) & 1)) & 0xffff;
	if(!lo) {
		unsafe_ldah(Rdst, base_reg, hi);	
	} else {
		unsafe_lda(Rdst, base_reg, lo);
		if(hi) unsafe_ldah(Rdst, Rdst, hi);	
	}
	/* XXX move this up */
	if(HI32(val) != 0xffffffff) zapnoti(Rdst, Rdst, 0xf);
}

void set64u(unsigned Rdst, unsigned long val) {
	unsigned lo;
	int rd;
	int hi;

	lo = (unsigned)val & 0xffffffff;
	hi = val >> 32;

	/* we use at, so need to allocate a different register. */
	if(Rdst != _at)  
		rd = Rdst;
	else {
  		if(v_get_temps(&rd, 1, V_TEMPI) < 0)
                	v_fatal("usti: out of registers\n");
	}

	demand(FITS(lo), bogus lo!);
	demand(FITS(hi), bogus hi!);

	setu(rd, lo);
	/* XXX don't think you have to do this one. */
	zapnoti(rd, rd, 0xf);
	setu(_at,  hi);
	slli(_at, _at, 32);
	or(Rdst, rd, _at);
}

void set64(unsigned Rdst, long val) {
	unsigned lo;
	int hi, rd;

	lo = (unsigned)val & 0xffffffff;
	hi = val >> 32;

	/* we use at, so need to allocate a different register. */
	if(Rdst != _at)  
		rd = Rdst;
	else {
  		if(v_get_temps(&rd, 1, V_TEMPI) < 0)
                	v_fatal("usti: out of registers\n");
	}

	demand(FITS(lo), bogus lo!);
	demand(FITS(hi), bogus hi!);

	set(rd, lo);
	zapnoti(rd, rd, 0xf);
	set(_at,  hi);
	slli(_at, _at, 32);
	or(Rdst, rd, _at);
}

/* modify this to use gp */
void set(unsigned Rdst, long val) {
        int lo, hi, base_reg;

	if(FITS(val))
		base_reg = _zero;
	else {
		/* I don't think we can rely on the gp */
		set64(Rdst, val);
		return;
	}

        lo = (unsigned)val & 0xffff;
        hi = ((val >> 16)  + ((val >> 15) & 1)) & 0xffff;
	if(!lo) {
		unsafe_ldah(Rdst, base_reg, hi);	
	} else {
		unsafe_lda(Rdst, base_reg, lo);
		if(hi) unsafe_ldah(Rdst, Rdst, hi);	
	}
}


#if 0
void set(unsigned Rdst, long val) {
        int low, tmp1, high, tmp2, extra;

        low = LOW16(val);               /* get lower 16 bits */
        tmp1 = val - SEXT(low);         /* account for LDA instruction */
        high = HI(tmp1);
        tmp2 = tmp1 - (SEXT(high) << 16);
        if(!tmp2)
                extra = 0;
        else {
                /* original value was in range 7fff8000 .. 7fffffff */
                extra = 0x4000;
                tmp1 = tmp1 - 0x40000000;
                high = HI(tmp1);
        }
        lda(Rdst, _r31, low);
        if(extra) ldah(Rdst, Rdst, extra);
        if(high)  ldah(Rdst, Rdst, high);
}

        sts:
  [d.c:   5] 0x80:      22107ffe        lda     a0, 32766(a0)
  [d.c:   5] 0x84:      4600d102        bic     a0, 0x6, t1
  [d.c:   5] 0x88:      a4220000        ldq     t0, 0(t1)
  [d.c:   5] 0x8c:      46003110        bic     a0, 0x1, a0
  [d.c:   5] 0x90:      48300241        mskwl   t0, a0, t0
  [d.c:   5] 0x94:      b4220000        stq     t0, 0(t1)
#endif

#if 0

a1 = value to store

  t0 holds the value to store.

  [d.c:   5] 0xc:       47e07401        bis     zero, 0x3, t0

        ldus:
  [d.c:   5] 0x0:       42310411        addq    a1, a1, a1
  [d.c:   5] 0x4:       42300411        addq    a1, a0, a1

  [d.c:   5] 0x8:       4620d103        bic     a1, 0x6, t2
  [d.c:   5] 0x10:      a4430000        ldq     t1, 0(t2)
  [d.c:   5] 0x14:      46203111        bic     a1, 0x1, a1
  [d.c:   5] 0x18:      48310361        inswl   t0, a1, t0
  [d.c:   5] 0x1c:      48510242        mskwl   t1, a1, t1
  [d.c:   5] 0x20:      44410402        bis     t1, t0, t1
  [d.c:   5] 0x24:      b4430000        stq     t1, 0(t2)
  [d.c:   5] 0x28:      6bfa8001        ret     zero, (ra), 1
  [d.c:   5] 0x2c:      47ff041f        bis     zero, zero, zero


#endif

void v_jalpi(v_reg_type ra, void *ip) {
	set(_t12, (unsigned long)ip);
	v_jalp(v_ra, v_reg(_t12));
	/* jsr(_vrr(ra), _t12, 0); */
}

extern int v_get_temps(int *rv, int n, int class);

inline void addli(int rd, int rs, int imm) {
	if(imm < 0) {
		subli(rd, rs, -imm);
	} else if(isu8bit(imm)) {
		_addli(rd, rs, imm);
	} else if(is16bit(imm)) {
		lda(rd, rs, imm);
		addl(rd, _zero, rd);	/* sign extension */
	} else { 
		set(rd, imm);
		addl(rd, rs, rd);
	}
}

inline void addqi(int rd, int rs, int imm) {
	if(imm < 0) {
		subqi(rd, rs, -imm);
	} else if(isu8bit(imm)) {
		_addqi(rd, rs, imm);
	} else if(is16bit(imm)) {
		lda(rd, rs, imm);
	} else { 
		set(rd, imm);
		addq(rd, rs, rd);
	}
}

inline void subqi(int rd, int rs, int imm) {
	if(imm < 0) {
		addqi(rd, rs, -imm);
	} else if(isu8bit(imm)) {
		_subqi(rd, rs, imm);
	} else if(is16bit(-imm)) {
		lda(rd, rs, -imm);
	} else { 
		set(rd, imm);
		subq(rd, rs, rd);
	}
}

inline void subli(int rd, int rs, int imm) {
	if(imm < 0) {
		addli(rd, rs, -imm);
	} else if(isu8bit(imm)) {
		_subli(rd, rs, imm);
	} else if(is16bit(-imm)) {
		lda(rd, rs, -imm);
		subl(rd, rd, _zero);	/* sign extension */
	} else { 
		set(rd, imm);
		subl(rd, rs, rd);
	}
}


void stus(int t0, int a0, long offset) {
	int temps[4], t1, t2, a1, t3;

	if(v_get_temps(temps, 4, V_TEMPI) < 0)
		v_fatal("usts: out of registers\n");

	t2 = temps[0];
	t1 = temps[1];
	a1 = temps[2];
	t3 = temps[3];

	if(is8bit(offset)) {
		addqi(a1, a0, offset);
	} else if(is16bit(offset)) {
		lda(a1, a0, offset);
	} else { 
		set(a1, offset);
		addq(a1, a0, a1);
	}

	bici(t2, a1, 0x6);
	ldq(t1, t2, 0);
	bici(a1, a1, 0x1);

	inswl(t3, t0, a1);
	mskwl(t1, t1, a1);
	bis(t1, t1, t3);
	stq(t1, t2, 0);
}
#if 0

void stus(int rd, int rs, long offset) {
	int temps[4], t0, t1, a0, a1;


	if(v_get_temps(temps, 4, V_TEMPI) < 0)
		v_fatal("usts: out of registers\n");

	t0 = temps[0];
	t1 = temps[1];
	a0 = temps[2];
	a1 = temps[3];

	if(is8bit(offset)) {
		addqi(a0, rs, offset);
	} else if(is16bit(offset)) {
		lda(a0, rs, offset);
	} else { 
		set(a0, offset);
		addq(a0, rs, a0);
	}

	bici(t1, a0, 0x6);
	ldq(t0, t1, 0);
	bici(a0, a0, 0x1);
	inswl(a1, rd, a1);
	mskwl(t0, t0, a0);
	bis(t0, t0, a1);
	stq(t0, t1, 0);
}
  [d.c:   5] 0xd0:      20107ffe        lda     v0, 32766(a0)
  [d.c:   5] 0xd4:      4400d101        bic     v0, 0x6, t0
  [d.c:   5] 0xd8:      a4210000        ldq     t0, 0(t0)
  [d.c:   5] 0xdc:      44003100        bic     v0, 0x1, v0
  [d.c:   5] 0xe0:      482002c0        extwl   t0, v0, v0
#endif

void ldus(int rd, int rs, long offset) {
	int t0;

	if(v_get_temps(&t0, 1, V_TEMPI) < 0)
		v_fatal("usts: out of registers\n");

	if(is8bit(offset)) {
		addqi(rd, rs, offset);
	} else if(is16bit(offset)) {
		lda(rd, rs, offset);
	} else { 
		set(rd, offset);
		addq(rd, rs, rd);
	}
	
	bici(t0, rd, 0x6);
	ldq(t0, t0, 0);
	bici(rd, rd, 0x1);
	extwl(rd, t0, rd);
}

/*******************************************************************
 * Unaligned loads: note we have to guard against rd == rs.
 */

void uldus(int rd, int rs, int offset) {
	int at2, d, regs[2];

	if(v_get_temps(regs, 2, V_TEMPI) < 0)
		v_fatal("uldq: out of registers\n");
	d = regs[0];
	at2 = regs[1];

        ldq_u(d, rs, offset);
        ldq_u(at2, rs, offset+1);
        lda(_at, rs, offset);
        extwl(d, d, _at);
        extwh(at2, at2, _at);
        or(rd, at2, d);
}

/* load unsigned unaligned long word. */
void uldul(int rd, int rs, int offset) {
	int at2, d, regs[2];

	if(v_get_temps(regs, 2, V_TEMPI) < 0)
		v_fatal("uldq: out of registers\n");
	d = regs[0];
	at2 = regs[1];

	ldq_u(d, rs, offset);
	ldq_u(at2, rs, offset + 3);
	lda(_at, rs, offset);
	extll(d, d, _at);	/* (*DANGER*): using _at */
	extlh(at2, at2, _at);
	or(rd, at2, d);
}

/* load unaligned long word. */
void uldl(int rd, int rs, int offset) {
	uldul(rd, rs, offset);
	/* Sign extend. */
	slli(rd, rd, 32);
	srai(rd, rd, 32);
}


void ulds(int rd, int rs, int offset) {
        uldus(rd, rs, offset);
        slli(rd, rd, 48);
        srai(rd, rd, 48);
}

/* see 4-45 */
/* there is the possibility for many duplicate loads of too-large offsets.
 * should check for this. */

/* unaligned quad word load */
void uldq(int rd, int rs, int offset) {
	int at2, d, regs[2];

	if(v_get_temps(regs, 2, V_TEMPI) < 0)
		v_fatal("uldq: out of registers\n");
	d = regs[0];
	at2 = regs[1];

        ldq_u(d, rs, offset);
        ldq_u(at2, rs, offset+7);
        lda(_at, rs, offset);
        extql(d, _at, d);
        extqh(at2, _at, at2);
        or(rd, at2, d);
}

/* unaligned long word load (4 bytes) */
void uldu(int rd, int rs, int offset) {
	int at2, d, regs[2];

	if(v_get_temps(regs, 2, V_TEMPI) < 0)
		v_fatal("uldq: out of registers\n");
	d = regs[0];
	at2 = regs[1];

        ldq_u(d, rs, offset);
        ldq_u(at2, rs, offset+3);
        lda(_at, rs, offset);
        extll(d, _at, d);
        extlh(at2, _at, at2);
        or(rd, at2, d);
}

/* unaligned long word + sign ext */
void uldi(int rd, int rs, int offset) {
        uldu(rd, rs, offset);
        slli(rd, rd, 32);
        srai(rd, rd, 32);
}


void ulduc(int rd, int rs, int offset) {
	int d;

	if(v_get_temps(&d, 1, V_TEMPI) < 0)
		v_fatal("uldq: out of registers\n");

        ldq_u(d, rs, offset);
        lda(_at, rs, offset);
        extbl(rd, d, _at);
}

void uldc(int rd, int rs, int offset) {
	int d;

	if(v_get_temps(&d, 1, V_TEMPI) < 0)
		v_fatal("uldq: out of registers\n");

        ldq_u(d, rs, offset);
        lda(_at, rs, offset+1);
        extqh(rd, d, _at);
        srai(rd, rd, 56);
}

/*****************************************************************
 *  Unaligned stores.
 */

/* 4-50 */

/* I can't believe this requires 5 temproraries. */
void ustq(int rd, int rs, int offset) {
        int r1, r2, r3, r4, r6, temps[5];

	if(v_get_temps(temps, 5, V_TEMPI) < 0)
		v_fatal("ustq: out of registers\n");

        r1 = temps[0];
        r2 = temps[1];
        r3 = temps[2];
        r4 = temps[3];
        r6 = temps[4];

        lda(r6, rs, offset);
        ldq_u(r2, rs, offset+7);
        ldq_u(r1, rs, offset);
        insqh(r4, r6, rd);
        insql(r3, r6, rd);
        mskqh(r2, r6, r2);
        mskql(r1, r6, r1);
        or(r2, r4, r2);
        or(r1, r3, r1);
        stq_u(r2, rs, offset+7);
        stq_u(r1, rs, offset);
}
void usti(int rd, int rs, int offset) {
        int r1, r2, r3, r4, r6, temps[5];

	if(v_get_temps(temps, 5, V_TEMPI) < 0)
		v_fatal("usti: out of registers\n");

        r1 = temps[0];
        r2 = temps[1];
        r3 = temps[2];
        r4 = temps[3];
        r6 = temps[4];

        lda(r6, rs, offset);
        ldq_u(r2, rs, offset+3);
        ldq_u(r1, rs, offset);
        inslh(r4, r6, rd);
        insll(r3, r6, rd);
        msklh(r2, r6, r2);
        mskll(r1, r6, r1);
        or(r2, r4, r2);
        or(r1, r3, r1);
        stq_u(r2, rs, offset+3);
        stq_u(r1, rs, offset);
}

void usts(int rd, int rs, int offset) {
        int r1, r2, r3, r4, r6, temps[5];

	if(v_get_temps(temps, 5, V_TEMPI) < 0)
		v_fatal("usti: out of registers\n");

        r1 = temps[0];
        r2 = temps[1];
        r3 = temps[2];
        r4 = temps[3];
        r6 = temps[4];

#if 0
	insbl r5, r6, r3 

	shift right by one
		r3, r5, r6
#endif

        lda(r6, rs, offset);
        ldq_u(r2, rs, offset+1);
        ldq_u(r1, rs, offset);
        inswh(r4, rd, r6);
        inswl(r3, rd, r6);
        mskwh(r2, r2, r6);
        mskwl(r1, r1, r6);
        or(r2, r4, r2);
        or(r1, r3, r1);
        stq_u(r2, rs, offset+1);
        stq_u(r1, rs, offset);
}

void ustb(int rd, int rs, int offset) {
        int r1, r3, r6, temps[3];

	if(v_get_temps(temps, 3, V_TEMPI) < 0)
		v_fatal("usti: out of registers\n");

        r1 = temps[0];
        r3 = temps[1];
        r6 = temps[2];

	
        lda(r6, rs, offset);
        ldq_u(r1, rs, offset);
        insbl(r3, rd, r6);
        mskbl(r1, r1, r6);
        or(r1, r3, r1);
        stq_u(r1, rs, offset);
}

/******************************************************************
 *  Loads of a guarenteed alignment.  Identical to normal loads except
 * they take a lower-bound on the alignment of the base pointer.
 *
 * Note: alignment of 0 is treated as infinitely aligned.  Probably
 * not a good idea.
 *
 * Align is also taken to be a lower bound rather than exact.
 * this means alignment of 2 w/ offset of 2 will be treated as
 * 2 byte aligned rather than 4.
 */

/* Choose the most restrictive alignment. */
static inline int alignm(int x, int y) {
	if(!x)
		return y;
	if(!y)
		return x;
 	return x < y ? x : y;
}

void aldus(int rd, int rs, long offset, long align) {
	switch(alignm(offset % 4, align % 4)) {
	case 0: 
		/* taken from A-11 */
		ldl(rd, rs, (offset / 4) * 4); 
		extwli(rd, rd, offset % 4);
		break; 	
	case 1: 
	case 2:
	case 3: uldus(rd, rs, offset);
	}
}

/* Give an alignment for the base pointer */
void alds(int rd, int rs, long offset, long align) {
	switch(alignm(offset % 4, align % 4)) {
	case 0: 
		/* taken from A-11 */
		ldl(rd, rs, (offset / 4) * 4); 
		slli(rd, rd, (48 - 8 * offset % 4)); 
		srai(rd, rd, 48); 
		break; 	
	case 1: 
	case 2:
	case 3: ulds(rd, rs, offset);
	}
}

/* Give an alignment for the base pointer */
void aldc(int rd, int rs, long offset, long align) {
	switch(alignm(offset % 4, align % 4)) {
	case 0: 
		/* taken from A-11 */
		ldl(rd, rs, (offset / 4) * 4); 
		slli(rd, rd, (56 - 8 * offset % 4));
		srai(rd, rd, 56);
		break; 	
	case 1: 
	case 2:
	case 3: uldc(rd, rs, offset);
	}
}

/* Give an alignment for the base pointer */
void alduc(int rd, int rs, long offset, long align) {
	switch(alignm(offset % 4, align % 4)) {
	case 0: 
		/* taken from A-11 */
		ldl(rd, rs, (offset / 4) * 4); 
		extbli(rd, rd, offset % 4);
		break; 	
	case 1: 
	case 2:
	case 3: ulduc(rd, rs, offset);
	}
}

void aldl(int rd, int rs, long offset, long align) {
	if(alignm(offset % 4, align % 4))
		uldl(rd, rs, offset);
	else
		ldl(rd, rs, offset);
}

void aldq(int rd, int rs, long offset, long align) {
	if(alignm(offset % 8, align % 8))
		uldq(rd, rs, offset);
	else
		ldq(rd, rs, offset);
}


/* we allocate the conversion area only on demand, to allow stack
   pointer adjustments to be elided. */
static int carea_alloc(void) {
	if(!v_carea)
		v_carea = v_local(V_D);
	return v_carea;
}

/*
  convert float to long (both single and double).

  [c.c:   5] 0x4:       5bf005f0        cvttqc  $f16,$f16
  [c.c:   5] 0x8:       9e1e0000        stt     $f16, 0(sp)
  [c.c:   5] 0xc:       a41e0000        ldq     v0, 0(sp)
*/
void cvd2l(int rd, int rs) {
	int carea = carea_alloc();
	cvttqc(_dat, rs);
	stt(_dat, _sp, carea);
	ldq(rd, _sp, carea);
}

/*
  convert float to int (both single and double).

  [c.c:   5] 0x34:      5bf005f0        cvttqc  $f16,$f16
  [c.c:   5] 0x38:      9e1e0000        stt     $f16, 0(sp)
  [c.c:   5] 0x3c:      a43e0000        ldq     t0, 0(sp)
  [c.c:   5] 0x40:      403f0000        addl    t0, zero, v0


*/
void cvd2i(int rd, int rs) {
	int carea = carea_alloc();
	cvttqc(_dat, rs);
	stt(rs, _sp, carea);
	ldq(rd, _sp, carea);
	addl(rd, rd, _zero);
}


/*
        cvl2f:
  [c.c:   5] 0x34:      b61e0000        stq     a0, 0(sp)
  [c.c:   5] 0x38:      8c3e0000        ldt     $f1, 0(sp)
  [c.c:   5] 0x3c:      5be11780        cvtqs   $f1,$f0
*/
void cvl2f(int rd, int rs) {
	int carea = carea_alloc();
	stq(rs, _sp, carea);
	ldt(rd, _sp, carea);
	cvtqs(rd, rd);
}

/*
        cvl2d:
  [c.c:   5] 0x9c:      b61e0000        stq     a0, 0(sp)
  [c.c:   5] 0xa0:      8c3e0000        ldt     $f1, 0(sp)
  [c.c:   5] 0xa4:      5be117c0        cvtqt   $f1,$f0

*/
void cvl2d(int rd, int rs) {
	int carea = carea_alloc();
	stq(rs, _sp, carea);
	ldt(rd, _sp, carea);
	cvtqt(rd, rd);
}



/************************************************************************
 *
 * Instruction flush and dump routines.
 */

static unsigned getbytes(void);
void v_flushcache(void *ptr, int size);

int cacheflush(char *addr, int nbytes, int cache);
int disassembler();

/* for dumping the instruction stream to stdout  */
static unsigned *loc;
static unsigned getbytes() {
        printf("0x%p\t",loc);
        return *loc++;
}

/* Dump code pointed to by code. */
void v_dump (v_code *code) {
        printf("disassembled:\n");
        for (loc=code; loc[0] || loc[1] || loc[2]; )
                if (disassembler(loc, 0, NULL, NULL, getbytes, NULL) == -1)
                        demand(0,0);
        printf("\n");
}

/* flush instruction stream from both data & instr caches  */
void v_flushcache(void *ptr, int nbytes) {
	asm ("call_pal 0x86");
#if 0
        if (cacheflush(ptr, nbytes, BCACHE) != 0)
                fatal(Cacheflush failed);
#endif
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

        /* make 8 byte alignment minimum. */
        if(ti->align < 8)
                v_ar_size = v_roundup(v_ar_size, 8);
        else
                v_ar_size = v_roundup(v_ar_size, ti->align);

        /* I think we just use v_ar_size.  Why did we ever not use it? */
        old_offset = v_ar_size;

        /* add in the size of the object.  roundup again. */
        v_ar_size += v_roundup(ti->size, 4);
        v_lalloc_p++;           /* allocated a local */

        /* return previous offset.  this is the one. */
        return old_offset;
}

/* allocate space on the stack for a block structure; we 8-byte align */
int v_localb(unsigned sz) {
       int old_offset;

        /* I think we just use v_ar_size.  Why did we ever not use it? */
        old_offset = v_ar_size;

        if(sz < 8)
                sz = v_roundup(sz, 8);

        v_ar_size = v_roundup(v_ar_size, 8);
        old_offset = v_ar_size;
        v_lalloc_p++;           /* allocated a local */

        v_ar_size += sz;

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

        v_nop();        /* emit a place holder */
        if(type == V_D) /* need to insns */
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

#if 0
        MIPS

        /*
         * We know that we will move argument registers to saved
         * registers in non-leaf procedures.  Therefore, there
         * is always a move instruction before we start this
         * prologue.  To hide our delay slot, we swap instructions.
         */
        if(!v_isleaf) {
                v_code t;
                t = first_arg[-1];
                first_arg[-1] = v_ip[-1];
                v_ip[-1] = t;
        }
#endif
}


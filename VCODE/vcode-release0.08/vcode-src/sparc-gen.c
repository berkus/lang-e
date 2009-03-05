#include <assert.h>
#include <stdio.h>
#include <memory.h>

#include "vcode-internal.h"
#include "demand.h"

#define MAX(x,y) ((x) > (y) ? (x) : (y))

/* holds pointer to current instruction stream. */
v_code *v_ip;
unsigned v_calls;
static int max_arg;	/* maximum number of outgoing parameters */
/* machine independent register names */
const v_reg_type
		v_zero 	= _vri(_g0), 	/* register always returns zero */
		v_sp	= _vri(_sp),	/* stack pointer */
		v_at	= _vri(_g1),	/* used by vcode to load constants, etc. */
		v_dat	= _vri(_f2),	/* used by vcode for synthetic fp ops. */
		v_ra 	= _vri(_ra),	/* return register */
		v_caller_int_rr = _vri(_o0), /* return register (what caller sees) */
		v_callee_int_rr = _vri(_i0), /* return register (what callee sees) */
		v_fp_rr	 = _vri(_f0),	/* fp return register */
		v_fp	= _vri(_i6),	/* frame-pointer */
		v_lp	= _vri(_sp);	/* register to give for local references */

const unsigned	v_lpr	= _sp; /* register to give for local references */

/* Max number of instructions required to load a single-precision immediate. */
const unsigned v_float_imm_insns = 3;	
/* Max number of instructions required to load a double-precision immediate. */
const unsigned v_double_imm_insns = 3;

/* 
 * Stack offset used to perform conversions between int and fp (by loading 
 * and storing).
 */
int v_carea;	

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


v_label_type v_epilogue_l; /* label for epilogue */
int v_isleaf;

/*****************************************************************************
 * Argument handling & calling conventions.
 *
 * Extremely simplistic at the moment.
 */

struct reg { char reg, class; };

/* lifted from lcc */
static struct reg argreg(int argno, int offset, int ty, int out) {
	/* we get space for 6 4-byte parameters */
	if(offset >= (4 * 6))
	  {
	    struct reg tmpreg = { 0, NULREG };
	    return tmpreg;
	  }
	else if(out)
	  {
	    struct reg tmpreg;
	    tmpreg.reg = _o0 + offset / 4;
	    tmpreg.class = IREG;
	    return tmpreg;
	  }
	else 
	  {
	    struct reg tmpreg;
	    tmpreg.reg = _i0 + offset / 4;
	    tmpreg.class = IREG;
	    return tmpreg;
	  }
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
 * The SPARC AR is laid out as follows:
 *
 *	+-------------------------------+	<----- fp
 *	|   space for local variables	|
 *	+-------------------------------+
 *	|	saved fp regs		|
 *	+-------------------------------+
 *	| 	saved global regs	|
 *	+-------------------------------+
 *	|      outgoing parameters	|
 *	+-------------------------------+
 *	| 6 words to store incoming args|
 *	+-------------------------------+
 *	| 1 word hidden param (unused)  |
 *	+-------------------------------+
 *	| 16 words to dump r-window	|
 *	+-------------------------------+	<----- sp
 *
 * Local variables can be allocated at any time, therefore we can only
 * emit the instruction to adjust the stack pointer after all code has
 * been emitted.  (i.e., it must be backpatched).  
 */
#define	FSAVESIZE	(32*4)		/* area to save all fp regs */
#define RSAVESIZE	(8*4)		/* area to save all gp regs */
#define ARGBUILDSIZE	(V_MAXARGS * 8)	/* the maximum number of bytes that
					 * can be consumed by the maximum
					 * number of arguments (basically 
					 * MAXARG doubles). 
					 */
#define WINDOW_SIZE	(16 * 4)
#define INCOMING_SIZE   (6 * 4)
#define HIDDEN_PARAM    (1 * 4)

/* get to the outgoing parameter slot */
#define ARG_OFFSET	(WINDOW_SIZE + INCOMING_SIZE + HIDDEN_PARAM) 
#define FP_SAVE_OFFSET	(GP_SAVE_OFFSET+RSAVESIZE+4)
#define GP_SAVE_OFFSET  (ARG_OFFSET + ARGBUILDSIZE)
#define LOCAL_OFFSET	(FSAVESIZE + RSAVESIZE)

int v_ar_size;

static v_code *sp_ip; /* location where the stack-pointer must be adjusted */
/* 
 * Unlike the mips, the SPARC ABI does not define callee-saved registers.
 * This fact eliminates the need for epilogue codes that restore them and
 * pads at the begining of the generated code to save them.  Yea.
 */

/* need to check and make sure that these temps actually need to be saved. */
void v_savep(v_reg_type r) { v_savei(r); }
void v_saveu(v_reg_type r) { v_savei(r); }
void v_savel(v_reg_type r) { v_savei(r); }
void v_saveul(v_reg_type r) { v_savei(r); }
void v_restorep(v_reg_type r) { v_restorei(r); }
void v_restoreu(v_reg_type r) { v_restorei(r); }
void v_restorel(v_reg_type r) { v_restorei(r); }
void v_restoreul(v_reg_type r) { v_restorei(r); }

void v_savef(v_reg_type r) { 
	if(v_istemp(r, V_F))
		v_stfi(r, v_sp, FP_SAVE_OFFSET + _vrr(r) * 4); 
}
void v_restoref(v_reg_type r) { 
	if(v_istemp(r, V_F))
		v_ldfi(r, v_sp, FP_SAVE_OFFSET + _vrr(r) * 4); 
}
void v_saved(v_reg_type r) { 
	if(v_istemp(r, V_D))
		v_stdi(r, v_sp, FP_SAVE_OFFSET + _vrr(r) * 4); 
}
void v_restored(v_reg_type r) { 
	if(v_istemp(r, V_D))
		v_lddi(r, v_sp, FP_SAVE_OFFSET + _vrr(r) * 4); 
}

/* should ensure that it is indeed a caller-saved register */
void v_savei(v_reg_type r) {
	if(v_istemp(r, V_I))
		v_stii(r, v_sp, (GP_SAVE_OFFSET + (_vrr(r) - _g0) * 4));
}
void v_restorei(v_reg_type r) {
	if(v_istemp(r, V_I))
		v_ldii(r, v_sp, (GP_SAVE_OFFSET + (_vrr(r) - _g0) * 4));
}

/* Possibly site-specific initialization. */
void v_push_init(struct v_cstate *c) {
        memset(c, 0, sizeof *c);
        c->cookie = V_COOKIE;
}

void v_mach_push(struct v_cstate *c, int ty, struct v_arg *arg) {
	int offset;
	struct reg r;
	v_reg_type arg_reg;
	struct typeinfo *ti;

        demand(c->cookie == V_COOKIE, Uninitialized call state.);

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

	offset = c->offset;
        r = argreg(c->argno, offset, ty, 1);
	ti = &tinfo[ty];

	/* If the destination is NULREG, push the arg onto the stack */
	if(r.class == NULREG) {
		if(offset < ARG_OFFSET)
			offset = ARG_OFFSET;

                /* if it is an immediate,  load it */
                if(!arg->isreg_p)
                      v_pset(ty, arg_reg, arg->u);
		if(ty != V_D)
                       	v_pst(ty, arg_reg, v_sp, offset);
		else {
			/* guard against unaligned accesses */
			if(offset % 8 == 0) 
				v_stdi(arg_reg, v_sp, offset); 
			else {
				stf(_vrr(arg_reg), _vrr(v_sp), offset); 
				stf(_vrr(arg_reg)+1, _vrr(v_sp), offset+4); 
			}
		}
	} else if(r.class == ti->class) {
        	if(!arg->isreg_p)
        		v_pset(ty, v_reg(r.reg), arg->u);
        	else
        		v_pmov(ty, v_reg(r.reg), arg_reg);
	/* have to convert between the two types */
	} else if(ty == V_F) {
		/* if it is an immediate,  load it */
		if(!arg->isreg_p)
			v_setf(arg_reg, arg->u.f);
		movf2i(r.reg, _vrr(arg_reg)); 
	} else {
		demand(ty == V_D, bogus class);
                if(!arg->isreg_p)
			v_setd(arg_reg, arg->u.d);
		/* 
		 * a boundary condition: the double is half in o5, the other
	 	 * half is on the stack 
		 */
		if(c->argno == 5) {
			offset = ARG_OFFSET - 4;
			/* move first fp reg to int */
			movf2i(r.reg, _vrr(arg_reg));
			/* stuff second fp reg on the callstack */
			stf(_vrr(arg_reg)+1, _sp, offset);
		} else {
			movd2i(r.reg, _vrr(arg_reg));
		}
	}
        c->offset = v_roundup(offset + ti->size, 4);
	c->argno++;
}

/* a simple call */
void v_mach_call(struct v_cstate *c, void (*ptr)()) {
        demand(c->cookie == V_COOKIE, Uninitialized call state.);

       	/* save all temporaries that are standing in for vars */
        v_move_regs(V_STANDINF, V_NO_DOUBLE_LD, v_savef, v_saved);
        v_move_regs(V_STANDINI, V_NO_DOUBLE_LD, v_savei, v_savel);


	/* call */
	call((unsigned)ptr);
	/* v_jali((unsigned)ptr); */
	max_arg = MAX(max_arg, c->argno);

       	/* restore all temporaries that are standing in for vars */
        v_move_regs(V_STANDINF, V_NO_DOUBLE_LD, v_restoref, v_restored);
        v_move_regs(V_STANDINI, V_NO_DOUBLE_LD, v_restorei, v_restorel);

        c->cookie = ~V_COOKIE;
}

/*
 * v_rmach_call: like mach_call, but uses a register than a fn ptr as argument.
 */
void v_rmach_call(struct v_cstate *c, v_reg_type fr) {
        demand(c->cookie == V_COOKIE, Uninitialized call state.);

       	/* save all temporaries that are standing in for vars */
        v_move_regs(V_STANDINF, V_NO_DOUBLE_LD, v_savef, v_saved);
        v_move_regs(V_STANDINI, V_NO_DOUBLE_LD, v_savei, v_savel);

	/* call */
	call_reg(_vrr(fr));
        /* v_jal(fr); */

	max_arg = MAX(max_arg, c->argno);

       	/* restore all temporaries that are standing in for vars */
        v_move_regs(V_STANDINF, V_NO_DOUBLE_LD, v_restoref, v_restored);
        v_move_regs(V_STANDINI, V_NO_DOUBLE_LD, v_restorei, v_restorel);

    	c->cookie = ~V_COOKIE;
}

/* 
 * Machine specific portion of lambda.  Procedure management that 
 * mainly shuffles incoming arguments into registers.
 */
void v_mach_lambda(int nargs, int *arglist, v_reg_type *args, int leaf, v_code *ip) {
	int offset, framesize, i, res;
	struct reg r;
	struct typeinfo *ti;

	sp_ip = ip;	/* mark where save instruction is issued */
	ip += 1;

	v_isleaf = leaf;
	v_begin(ip);

	max_arg = offset = framesize = 0;

	demand(FP_SAVE_OFFSET % 8 == 0, fp save area must be 8 byte aligned!);

 	/* 
	 * Allocate space for everything; at this point we do not know how much space is
	 * required for outgoing parameters, or local variables.   For this reason, we
	 * defer emitting the save instruction.
	 */
	v_ar_size = FSAVESIZE + RSAVESIZE + ARGBUILDSIZE + WINDOW_SIZE + INCOMING_SIZE + HIDDEN_PARAM;

	/* Allocate space to perform conversions */
	v_carea = v_local(V_D);


	/* 
	 * Map incoming parameters to registers.  Basically, if it is
	 * a FP or on the stack, we move it.
	 */
	for(i = 0; i < nargs; 
			args++, arglist++, i++, 
			offset = v_roundup(offset + ti->size, 4)) {

		ti = &tinfo[*arglist];

                /* offset = v_roundup(offset, ti->align); */
                r = argreg(i, offset, *arglist, 0);

		/* If it is in a register of the appropriate type, leave it.  */
		if(r.class == ti->class) {
			/* we can leave it in place */
			*args = v_reg(r.reg);
			continue;
		}
		/* Allocate a VAR register of the given type */
		if(!(res = v_getreg(args, *arglist, V_VAR)))
			v_fatal("v_lambda: not enough registers to hold result\n");

		/* 
		 * If it is on the stack, load it into a register.  
		 * If it is a register of the wrong type, move it.
		 */
		if(r.class == NULREG) {
			/* hack.  */
			if(offset < ARG_OFFSET)
				offset = ARG_OFFSET;

			switch(*arglist) {
			case V_UC: 	v_lduci((*args), v_fp, offset); break;
			case V_C: 	v_ldci((*args), v_fp, offset); break;
			case V_US: 	v_ldusi((*args), v_fp, offset); break;
			case V_S: 	v_ldsi((*args), v_fp, offset); break;
			case V_I: case V_U: 
			case V_L: case V_UL: 
			case V_P: 	v_ldii((*args), v_fp, offset); break;
			case V_F: 	v_ldfi((*args), v_fp, offset); break;
			case V_D:
				if(offset % 8 == 0) 
					v_lddi((*args), v_fp, offset);
				else {
					ldf(_vrrp(args), _vrr(v_fp), offset); 
					ldf(_vrrp(args)+1, _vrr(v_fp), offset+4); 
				}
				break;
			default:	demand(0, bogus type);
			}
                } else if(*arglist == V_F) {
			movi2f(_vrrp(args), r.reg);
			v_rawput(r.reg, V_VARI);	/* don't need this register */
		} else {
			demand(*arglist == V_D, bogus type);
			demand(r.class == IREG, bogus class);
			/* 
			 * a boundary condition: the double is half in o5, the other
		 	 * half is on the stack 
			 */
			if(offset == 5*4) {
				offset = ARG_OFFSET - 4;
				/* store the int register in memory and then load the 
				 * two words containing the argument */
				st(_i5, _fp, offset);
				lddf(_vrrp(args), _fp, offset);
			} else {
				movi2d(_vrrp(args), r.reg);
				v_rawput(r.reg+1, V_VARI);
			}
			v_rawput(r.reg, V_VARI); /* don't need these incoming regs */
		}
	}

	/* 
	 * The final instruction stream is as follows:
	 *	+-------------------------------+
	 *	| insn to adjust stack: constant|
	 *	| is assumed to fit 		|
	 *	+-------------------------------+
	 *	| insns to move & load args	|
	 *	+-------------------------------+
	 *	| body of function		|
	 *	+-------------------------------+
	 *	| epilogue 			|
	 *	+-------------------------------+
	 */


	/* 
	 * Argument registers are managed by us (because on the
	 * sparc and such machines, they do not need to be saved
	 * or restored across function calls.
	 */

	/* allow incoming parameters to be used as temporaries */
	for(i = offset / 4; i < 6; i++)
		v_rawput(i + _i0, V_VARI);

	/* If we are a leaf, make outgoing parameter registers available. */
	if(v_isleaf) {
		/* We leave o0, o6, o7 available for pseudo-calls. */
		for(i = _o1; i <= _o5; i++) {
			if(i == _sp)		/* sp is an out reg */
				continue;
			v_rawput(i, V_VARI);
		}
	}
}

/* should see if fp was actually used and omit space in the ar if not */
union v_fp v_mach_end(int *nbytes) { 
	v_code *tmp;
	union v_fp fp;

	if(max_arg > 6)	/* args were passed on the stack */
		v_ar_size += 4 * max_arg;
	/* back-patch save */
	tmp = v_ip;
	v_ip = sp_ip;
	v_ar_size = v_roundup(v_ar_size, 8);	/* sp must be 8-byte aligned */
	save(-v_ar_size);
	v_ip = tmp;

	v_link();		/* link before any instructions have been emitted.  */

	/* emit last return (in case they did not) */
	ret();
	restore();

	/* pattern of nops looked for by v_dump */
	v_nop();
	v_nop();
	v_nop();
	v_set_fp_imms(v_ip);

	*nbytes = (v_ip - sp_ip) * sizeof *v_ip;
	v_flushcache(sp_ip, *nbytes);

	fp.v = (v_vptr) sp_ip;
	return fp;
}


#ifdef SPARC_NO_SYNTHETIC
void v__mul(v_reg_type rd, v_reg_type rs1, v_reg_type rs2) {
	smul(_vrr(rd), _vrr(rs1), _vrr(rs2));
}

void v__umul(v_reg_type rd, v_reg_type rs1, v_reg_type rs2) {
	umul(_vrr(rd), _vrr(rs1), _vrr(rs2));
}

void v__div(v_reg_type rd, v_reg_type rs1, v_reg_type rs2) {
	sdiv(_vrr(rd), _vrr(rs1), _vrr(rs2));
}

void v__udiv(v_reg_type rd, v_reg_type rs1, v_reg_type rs2) {
	udiv(_vrr(rd), _vrr(rs1), _vrr(rs2));
}


#else

void v__mul(v_reg_type rd, v_reg_type rs1, v_reg_type rs2) {
	int __mul();
	v_reg_type r;
	v_pseudo_call = 1;
	r = v_scalli(__mul, "%i%i", rs1, rs2);
	v_pseudo_call = 0;
	v_movi(rd, r);
}
void v__div(v_reg_type rd, v_reg_type rs1, v_reg_type rs2) {
	int __div();
	v_reg_type r;
	v_pseudo_call = 1;
	r = v_scalli(__div, "%i%i", rs1, rs2);
	v_pseudo_call = 0;
	v_movi(rd, r);
}
void v__umul(v_reg_type rd, v_reg_type rs1, v_reg_type rs2) {
	unsigned __umul();
	v_reg_type r;
	v_pseudo_call = 1;
	r = v_scallu(__umul, "%u%u", rs1, rs2);
	v_pseudo_call = 0;
	v_movu(rd, r);
}
void v__udiv(v_reg_type rd, v_reg_type rs1, v_reg_type rs2) {
	unsigned __udiv();
	v_reg_type r;
	v_pseudo_call = 1;
	r = v_scallu(__udiv, "%u%u", rs1, rs2);
	v_pseudo_call = 0;
	v_movu(rd, r);
}
#endif

void v__rem(v_reg_type rd, v_reg_type rs1, v_reg_type rs2) {
	int __rem();
	v_reg_type r;
	v_pseudo_call = 1;
	r = v_scalli(__rem, "%i%i", rs1, rs2);
	v_pseudo_call = 0;
	v_movi(rd, r);
}

void v__urem(v_reg_type rd, v_reg_type rs1, v_reg_type rs2) {
	unsigned __urem();
	v_reg_type r;
	v_pseudo_call = 1;
	r = v_scallu(__urem, "%u%u", rs1, rs2);
	v_pseudo_call = 0;
	v_movu(rd, r);
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

        /* return previous offset.  this is the one. */
        return old_offset;
}

/* allocate space on the stack for a block structure; we 8-byte align */
int v_localb(unsigned sz) {
       int old_offset;

        /* I think we just use v_ar_size.  Why did we ever not use it? */
        old_offset = v_ar_size;

        if(sz < 4)
                sz = v_roundup(sz, 4);

        v_ar_size = v_roundup(v_ar_size, 8);
        old_offset = v_ar_size;

        v_ar_size += sz;

        return old_offset;
}

#if 0
/* 
 * Brain-dead sparc does mul,div,rem in software, and then
 * dynamically links them.  in short, I cannot get their address
 * and so must use these goddamn wrappers -- we could actually
 * screw around w/ asm stmts to load a jtable of the addrs of .mul,
 * .div, etc and it would save a couple of cycles 
 */
static int __mul(int a, int b)                    { return a * b; }
static int __div(int a, int b)                    { return a / b; }
static int __rem(int a, int b)                    { return a % b; }
static unsigned __umul(unsigned a, unsigned b)    { return a * b; }
static unsigned __udiv(unsigned a, unsigned b)    { return a / b; }
static unsigned __urem(unsigned a, unsigned b)    { return a % b; }
#endif


/*
 * This file holds quasi-machine-indenpendent source.  Assumes two different
 * sizes for float and double, and (at most) two sizes for integer registers
 * (and that they cleave into two classes: V_L, V_UL, V_P are big, everything
 * else small).  Need to integrate save/restore functions better: currently
 * they will have too many nops or not use any cool double save abilities.
 * Assumes only callee and caller saved.
 */
#include <assert.h>
#include <stdio.h>

#include "vcode-internal.h"
#include "demand.h"

static void lreset(void);
void register_reset(void);

#define MAX(a,b) ((a) > (b) ? (a) : (b))

/* holds pointer to current instruction stream. */
v_code *v_ip;
unsigned v_calls;
int v_pseudo_call;		/* used to tell scall when leaf calls are ok */
int v_nbytes;	/* number of bytes available to hold instructions */

static int elock;

#ifndef __fast__
v_reg_type v_reg(int reg) {
  v_reg_type tmpreg;
  tmpreg.reg = reg;
  return tmpreg;
}
#endif

/* begin emitting asm */
void v_begin(v_code *i) { 
	if(elock)
		v_fatal("do not support concurrent v_lambda calls.\n");
	elock = 1;
	v_ip = i; 
	v_calls = 0;
	register_reset() ;
	lreset();
}

/* is a procedure being created at the moment? */
int v_locked_p(void) {
	return elock == 1;
}

int v_unlock(void) {
	int el = elock;
	elock = 0;
	return el == 1;
}

void v_init(void) {
	v_calls = 0;
	register_reset() ;
	lreset();
}

v_code *v_stop(void) {
	v_code *ip = v_ip;
	if(!elock)
		v_fatal("v_stop: not paired with an emit begin\n");
	elock = 0;
	v_ip = 0;
	return ip;
}

/* set the ip so that the backend can generate some instructions. */
void v_set_ip(v_code *i) {
	v_ip = i;
}

v_code *v_swapcp(v_code *x) {
        v_code *old;

        old = v_ip;
        v_ip = x;
        return old;
}

/****************************************************************
 * Sleazy incremental linking routines.
 */

/*
 * All we do at the moment is reestablish the epilogue label.
 * To do this for real, would have to track:
 *      1. register allocations
 *      2. activation record allocations
 *      3. ensure that state is not reset by v_end.
 * and, if (1) or (2) change, regenerate epilogue code.
 *
 * We are sleazy right now.
 */
static v_code *frag_begin;
static int frag_bytes;
int v_incremental = 0;
void v_begin_incremental(v_code *ip, int nbytes) {
        extern v_label_type v_epilogue_l;

	v_incremental = 1;
        frag_begin = ip;
        frag_bytes = nbytes;

	if(elock)
		v_fatal("do not support concurrent v_lambda calls.\n");
	elock = 1;
	v_ip = ip; 
	lreset();

        if(!v_labeleq(v_genlabel(), v_epilogue_l))
                v_fatal("v_begin_incremental: Bogus epilogue label");
}

/* 
 * Links fragment and returns size of fragment (in bytes).
 */
int v_end_incremental(void) {
        int nbytes;

        v_link();
        v_nop(); v_nop(); v_nop(); v_nop();
        nbytes = (char *)v_stop() - (char *)frag_begin;
        if(nbytes > frag_bytes) 
                v_fatal("v_end_incremental: buffer overflow\n");
        v_flushcache(frag_begin, nbytes);
	v_incremental = 0;
        return nbytes;
}

/********************************************************************
 * Label management.  (put in machine independ?)
 */
static v_label_type l;

/* 
 * Generate a unique label --- note that the way we allocate prevents
 * `0' from being a valid label.
 */
v_label_type v_genlabel(void) { 
	_vlr(l)++;
	return l;
}

int v_labeleq(v_label_t l1, v_label_t l2) {
	return _vlr(l1) == _vlr(l2);
}

void v_label(v_label_type l)    { v_lmark(v_ip, l); }
void v_dlabel(void *addr, v_label_type l)   { 
	if(!addr)
		v_fatal("v_dlabel: given a nil addr for label %d\n", _vlr(l));
	v_dmark(addr, l); 
}

/* reset label information */
static void lreset(void) {
	_vlr(l) = 0;
	v_link_reset();
}

/*******************************************************************
 * Handle procedure construction.
 */

static char *v_name;
static  v_reg_type *param[V_MAXARGS];
static int v_ptypes[V_MAXARGS], *v_typep = &v_ptypes[0];
static int v_max_params;

/* simple default lambda; called with static values. */
int v_lambda(char *name, char *fmt, v_reg_type *args, int leaf, v_code *ip, int nbytes) {
        int *arglist, nargs;

        if(v_max_params)
                v_fatal("v_lambda: started a v_lambda.\n");

	v_nbytes = nbytes;
        v_name = name;
        arglist = v_xlatel(fmt, &nargs);
        /* call the machine-specific lambda */
        v_mach_lambda(nargs, arglist, args, leaf, ip);
	return 0;	/* Success */
}

union v_fp v_end(int *nbytes) {
	int sz;
	union v_fp fp;

	assert_active(v_end);

	if(!nbytes)
		nbytes = &sz;
		
       	fp = v_mach_end(nbytes);

	if(*nbytes > v_nbytes)
		v_fatal("v_end: instruction buffer overflow.\n");	
        if(!v_unlock())
                v_fatal("v_end not paired with v_lambda\n");

	return fp;
}



/* allocate the given parameter number */
void v_param_alloc(int argno, int type, v_reg_type *r) {
        if(argno >= V_MAXARGS)
                v_fatal("v_param_alloc: argno %d is too large; \
                           can only have %d parameters\n" , argno, V_MAXARGS);

        if(param[argno])
                v_fatal("param: argno %d has already been allocated.\n", argno);

        if(!(param[argno] = r))
                v_fatal("v_param_alloc: bogus register pointer.\n");

	*v_typep++ = type;

	argno++;

        if(argno > v_max_params)
                v_max_params = argno;
}

/* Push an argument in order.  Only big deal is that the parameter
 * number is implicit.  Can be useful in some situations. */
void v_param_push(int type, v_reg_type *r) {
	v_param_alloc(v_max_params, type, r);
}


/* complex-lambda: its incoming parameter list was built up ``on the
   fly.'' */
void v_clambda(char *name, int leaf, v_code *ip, int nbytes) {
        v_reg_type args[V_MAXARGS];
	int i;

        for(i =0; i < v_max_params; i++)
                if(!param[i])
                        v_fatal("v_clambda: param %d isn't allocated.\n", i);

        v_name = name;
	v_nbytes = nbytes;
        v_mach_lambda(v_max_params, v_ptypes, args, leaf, ip);

        /* set the pre-allocated register structures to the values given
           by clambda. */
        for(i =0; i < v_max_params; i++) {
                *param[i] = args[i];
                param[i] = 0;   /* reset */
        }
	v_max_params = 0;
	v_typep = &v_ptypes[0];
}

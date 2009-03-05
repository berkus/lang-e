/*******************************************************************
 * Handle loads of immediate floating point values.
 */
#include "vcode-internal.h"

#define MAX_FP_IMMS 128
static struct imm_entry { 
	unsigned char type; 
	v_reg_type reg;
	unsigned char nnops;
	void *ip;
	union fp_union { float f; double d; } u; 
} imm_table[MAX_FP_IMMS], *imm_p = &imm_table[0];

/*
 * We store each double or float and, at the end of code generation,
 * mark where each one is permentantly stored.  These routines
 * don't really belong here.
 */

static inline void im_enqueue(struct imm_entry *ie, char type, void *ip, v_reg_type reg, int nnops) {
	int i;

	if(imm_p >= &imm_table[MAX_FP_IMMS])
		v_fatal("immediate table is full: need more than %d entries.\n", MAX_FP_IMMS);
	ie->nnops = nnops;
	ie->type = type; ie->ip = ip; ie->reg = reg;
	*imm_p++ = *ie;

        for(i = 0; i < nnops; i++)
                v_nop();        /* emit nops as place-holders */
}

/* Mark where a single precision immediate load will occur. */
void v_float_imm(v_reg_type rd, float imm) {
	struct imm_entry ie;

	ie.u.f = imm;
	im_enqueue(&ie, 'f', v_ip, rd, v_float_imm_insns);
}

/* Mark where a double precision immediate load will occur. */
void v_double_imm(v_reg_type rd, double imm) {
	struct imm_entry ie;

	ie.u.d = imm;
	im_enqueue(&ie, 'd', v_ip, rd, v_double_imm_insns);
}

/*
 * Load a floating point immediate on machines that do not allow it to be
 * encoded in the instruction stream.  It assumes that the v_code
 * instruction that registered the imm has set aside sufficient space
 * to load it.
 *
 * Places the floating point immediates in the code stream.
 */
void v_set_fp_imms(void *ip) {
	union fp_union *fu;
	int n;
	void *v_ip_old;

        v_ip_old = v_ip;

	fu = (void *)v_roundup(ip, sizeof(double));

	for(imm_p--; imm_p >= &imm_table[0]; imm_p--, fu++) {
		/* store the immediate in memory. */
		*fu = imm_p->u;
		/* now create the instruction to load it. */
        	v_ip = imm_p->ip;

        	/* Generate the code to do the load. */
        	if(imm_p->type == 'd') {
                	v_lddi(imm_p->reg, v_zero, (unsigned long)&fu->d);
        	} else
                	v_ldfi(imm_p->reg, v_zero, (unsigned long)&fu->f);

        	/*
         	 * We reserve a finite amount of instructions to do the load: 
		 * assert that this limit was not exceeded.
         	 */
        	if((n = v_ip - (v_code*)imm_p->ip) > imm_p->nnops)
                	v_fatal("load-fp-imm: must consume less than %d insns; consumed %d\n", 
			imm_p->nnops, n);
		for(; n  < imm_p->nnops; n++)
			v_nop();
	}
        v_ip = v_ip_old;
	imm_p = &imm_table[0];
}

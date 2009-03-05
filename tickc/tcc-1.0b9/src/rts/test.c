#include <tickc-rts-internal.h>

/* allocate a v_spec structure */
v_spec * local(int type, ...);
/* allocate an incoming parameter */
v_spec * param(int type,  int argno); 
/* compile a cspec and return a void function ptr */
v_vptr compile(c_spec * cs, int type);

/* create a function with random arguments and parameters */
int main() { 
#	define MAXLOCALS 128
	v_spec *locals[MAXLOCALS];
#	define MAXPARAMS 8
	v_spec *params[MAXPARAMS];
	int n, i;
	struct v_reg r;

	n = 4;

trials:
	printf("TRIAL %d\n", n);

	/* a wimpy test of parameter construction. */
	{
		struct v_reg r1, r2;
		v_code insn[1024];
		v_iptr ip;

		v_param_alloc(0, V_I, &r1);
		v_param_alloc(1, V_I, &r2);

		v_clambda("foo", V_LEAF, insn);
			v_addi(r1, r1, r2);
			v_reti(r1);
		ip = v_end().i;

		if(ip(10,12) != 22)
			v_fatal("param alloc fucked up, returned %d.\n", ip(10,12));
	}
	/* a wimpy test of argument construction. */
	{
		struct v_reg r1, r2, r3, r4;
		v_code insn[1024];
		v_vptr vp;

		v_param_alloc(0, V_I, &r1);
		v_param_alloc(1, V_I, &r2);
		v_param_alloc(2, V_I, &r3);
		v_param_alloc(3, V_I, &r4);

		v_clambda("foo", V_NLEAF, insn);

		v_arg_alloc(0, V_I, r1);
		v_arg_alloc(1, V_I, r2);
		v_arg_alloc(2, V_I, r3);
		v_arg_alloc(3, V_I, r4);
		v_ccallv((v_vptr)printf);

		vp = v_end().v;
		vp("%d + %d = %d\n", 1, 2, 3);
	}



	for(i = 0; i < MAXLOCALS; i++) {
		switch(random() % 4) {
		case 0: locals[i] = local(TC_I); break;
		case 1: locals[i] = local(TC_D); break;
		case 2: locals[i] = local(TC_US); break;
		case 3: locals[i] = local(TC_P); break;
		}
	}

	for(i = 0; i < MAXPARAMS; i++) {
		switch(random() % 4) {
		case 0: params[i] = param(TC_I, i); break;
		case 1: params[i] = param(TC_D, i); break;
		case 2: params[i] = param(TC_US, i); break;
		case 3: params[i] = param(TC_P, i); break;
		}
	}

	compile(0, TC_P); /* might want to have TC_V */

	for(i = 0; i < MAXLOCALS; i++)
		printf("local[%d] at offset %d\n", i, locals[i]->offset);
	for(i = 0; i < MAXPARAMS; i++) 
		printf("params[%d] has reg %d\n", i, params[i]->reg.reg); 
	
	/* need to have tests that push down more than one. */
	v_push_rcontext();
	for(; v_getreg(&r, V_I, V_TEMP); i++)
		printf("Got tempi reg %d\n", r.reg);
	for(; v_getreg(&r, V_I, V_VAR); i++)
		printf("Got vari reg %d\n", r.reg);
	for(; v_getreg(&r, V_D, V_TEMP); i++)
		printf("Got tempf reg %d\n", r.reg);
	for(; v_getreg(&r, V_D, V_VAR); i++)
		printf("Got varf reg %d\n", r.reg);
	v_pop_rcontext();
	printf("allocated %d total registers\n", i);

	if(n-- > 0) goto trials;
	return 0;
}

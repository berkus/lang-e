/* Dynamically generate marshalling and unmarshalling code.  Would be more
   realistic to add support for different return types. */
#include <stdio.h>
#include <malloc.h>
#include "vcode.h"              /* This header file defines all vcode insns. */

typedef void (*marshal_p)();
typedef void (*unmarshal_p)(void *);

/* Generate a routine to marshal its arguments into a memory vector and
   then call function v. */
marshal_p gen_marshal(v_vptr v, char *fmt) {
#	define MAXARG	10
	unsigned csize, nargs, offset, mem, i;
	v_reg_type mr, arg[MAXARG];
	v_code *code;

	nargs = strlen(fmt) / 2;
	assert(nargs < MAXARG);

	/* uch.  a hacked guess. */
	csize = (128 + nargs * 4 * sizeof *code);
	code = malloc(csize);
	assert(code);

	v_lambda("marshal", fmt, arg, V_NLEAF, code, csize);

	if(!v_getreg(&mr, V_P, V_TEMP))
		v_fatal("out of registers\n");

	/* Allocate an array of largest size */
	mem = v_localb(nargs * sizeof(double));
	v_addpi(mr, v_lp, mem);

	for(i = offset = 0; *fmt; fmt++, i++) {
		if(*fmt != '%')
			v_fatal("expecting '%', got '%c'\n", *fmt);
		switch(*++fmt) {
		case 'i':	
			v_stii(arg[i], mr, offset);
			offset += sizeof(int);
			break;
		case 'd':
			v_stdi(arg[i], mr, offset);
			offset += sizeof(double);
			break;
		case 'p':
			v_stpi(arg[i], mr, offset);
			offset += sizeof(void *);
			break;
		default:	v_fatal("unknown argument type '%c'\n", *fmt);
		}
	}
	v_scallv(v, "%p", mr);

	return (marshal_p) v_end(0).i;
}


/* Generate a routine to unmarshal its arguments and call a function. */
unmarshal_p gen_unmarshal(v_vptr v, char *fmt) {
	unsigned csize, nargs, offset, i;
	v_reg_type mr, t0, p0, d0;
	v_code *code;
	struct v_cstate c;

	nargs = strlen(fmt) / 2;

	/* uch.  a hacked guess. */
	csize = (128 + nargs * 4 * sizeof *code);
	code = malloc(csize);
	assert(code);

	v_lambda("unmarshal", "%p", &mr, V_NLEAF, code, csize);
	v_push_init(&c);

	if(!v_getreg(&p0, V_P, V_TEMP))
		v_fatal("out of registers\n");
	if(!v_getreg(&t0, V_I, V_TEMP))
		v_fatal("out of registers\n");
	if(!v_getreg(&d0, V_D, V_TEMP))
		v_fatal("out of registers\n");

	for(i = offset = 0; *fmt; fmt++, i++) {
		if(*fmt != '%')
			v_fatal("expecting '%', got '%c'\n", *fmt);
		switch(*++fmt) {
		case 'i':	
			v_ldii(t0, mr, offset);
			v_push_argi(&c, t0);
			offset += sizeof(int);
			break;
		case 'd':
			v_lddi(d0, mr, offset);
			v_push_argd(&c, d0);
			offset += sizeof(double);
			break;
		case 'p':
			v_ldpi(p0, mr, offset);
			v_push_argp(&c, p0);
			offset += sizeof(void *);
			break;
		default:	v_fatal("unknown argument type '%c'\n", *fmt);
		}
	}
	v_ccallv(&c, (v_vptr)v);

	return (unmarshal_p) v_end(0).i;
}

/* hack. */
void call_printf(unsigned *args) {
	char *str = *(void **)args;

	args = (void *)((char *)args + sizeof str);
	printf(str, args[0], args[1]);
}

void call6(unsigned *args) {
	printf("(%d,%d,%d,%d,%d,%d)\n", 
		args[0], args[1], args[2], args[3], args[4], args[5]);
}

int main(int argc, char *argv[]) {
	marshal_p mp, mp2;
	unmarshal_p up;
	int verbose;

	verbose = argc > 1;

	mp = gen_marshal((v_vptr)call_printf, "%p%i%i");
	if(verbose) v_dump((void *)mp);
	mp("hello: arg1 = %d, arg2 = %d\n", 10, 20);

	mp2 = gen_marshal((v_vptr)call6, "%i%i%i%i%i%i%i");
	if(verbose) v_dump((void *)mp2);
	mp2(1,2,3,4,5,6);

	up = gen_unmarshal((v_vptr)printf, "%p%i%i");
	if(verbose) v_dump((void *)up);
	mp = gen_marshal((v_vptr)up, "%p%i%i");
	mp("hello: arg1 = %d, arg2 = %d\n", 10, 20);

        return 0;
}

/* FUTURE WORK. */

#if 0
	p0 = t0;	/* Uch.  Should have something that tells us if 
				register in is same class */
#endif

	/* V_TAIL: function is called at the end after all registers are
		used.  Cannot use any register that was passed in as
		a parameter. */

	/* Need to add facilities so that it does not allocate parameters
		to registers. */

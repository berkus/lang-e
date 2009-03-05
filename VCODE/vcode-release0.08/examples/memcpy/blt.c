/* Create a specialized memcpy loop for a particular input. */
#include <stdio.h>
#include <malloc.h>
#include "vcode.h"              /* This header file defines all vcode insns. */

typedef void (*blt_p)(void *dst);

/* Generate a simple word-aligned memcpy */
blt_p gen_simple(void *p, int n) {
	unsigned csize, i;
	v_code *code;
	v_reg_type arg[2], t0, dst;
	long *src;

	/* uch.  a hacked guess. */
	csize = (128 + n * 2 * sizeof *code);
	code = malloc(csize);
	assert(code);

	v_lambda("gen", "%p", arg, V_LEAF, code, csize);
	dst = arg[0];

	/* crude alignment check */
	assert((unsigned long)p % sizeof *src == 0);
	src = p;

	assert(n % sizeof(long) == 0);	/* long align */
	n /= sizeof(long);

	if(!v_getreg(&t0, V_L, V_TEMP))
		v_fatal("memcpy: out of registers\n");

	/* should we unroll this to allow for better superscaler perf? */
	for(i = 0; i < n; i++) {
		v_setl(t0, src[i]);
		v_stli(t0, dst, (i + 0) * sizeof(long));
	}
	return (blt_p) v_end(0).i;
}

int main(int argc, char *argv[]) {
	int n = argc == 2 ? atoi(argv[1]) : 100, i, msize;
	unsigned *mem0, *mem1, *src;
	blt_p bp;

	msize = n * sizeof *mem0;
	mem0 = malloc(msize);
	mem1 = malloc(msize);
	src = malloc(msize + 2 * sizeof *mem0);

	for(i = 0; i < n; i++)
		src[i] = rand();

	memcpy(mem0, src, msize);
	bp = gen_simple(src, msize);
	bp(mem1);
	/* v_dump(bp); */

	if(memcmp(mem0, mem1, msize))
		assert(0);

        return 0;
}

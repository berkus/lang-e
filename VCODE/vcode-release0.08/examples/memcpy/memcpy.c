/* Create a specialized memcpy loop for a given size.  */
#include <stdio.h>
#include <malloc.h>
#include "vcode.h"              /* This header file defines all vcode insns. */

typedef void (*memcpy_p)(void *dst, void *src);

/* Generate a simple word-aligned memcpy */
memcpy_p gen_simple(int n) {
	unsigned csize, i, rem;
	v_code *code;
	v_reg_type arg[2], t0, t1, t2, t3, dst, src;

	/* uch.  a hacked guess. */
	csize = (128 + n * 2 * sizeof *code);
	code = malloc(csize);
	assert(code);

	v_lambda("gen", "%p%p", arg, V_LEAF, code, csize);
	dst = arg[0];
	src = arg[1];

	assert(n % sizeof(long) == 0);	/* long align */
	n /= sizeof(long);

	if(!v_getreg(&t0, V_L, V_TEMP) || !v_getreg(&t1, V_L, V_TEMP)
	|| !v_getreg(&t2, V_L, V_TEMP) || !v_getreg(&t3, V_L, V_TEMP))
		v_fatal("memcpy: out of registers\n");

	rem = n % 4;
	n = n - rem;

	for(i = 0; i < n; i += 4) {
		v_raw_load(v_ldli(t0, src, (i+0) * sizeof(long)), 3);
		v_raw_load(v_ldli(t1, src, (i+1) * sizeof(long)), 3);
		v_raw_load(v_ldli(t2, src, (i+2) * sizeof(long)), 3);
		v_raw_load(v_ldli(t3, src, (i+3) * sizeof(long)), 3);

		v_stli(t0, dst, (i + 0) * sizeof(long));
		v_stli(t1, dst, (i + 1) * sizeof(long));
		v_stli(t2, dst, (i + 2) * sizeof(long));
		v_stli(t3, dst, (i + 3) * sizeof(long));
	}
		
	/* left-over */
        switch(rem)  {
        case 3: v_raw_load(v_ldli(t2, src, (i+2) * sizeof(long)), rem-1);
        case 2: v_raw_load(v_ldli(t1, src, (i+1) * sizeof(long)), rem-1);
        case 1: v_raw_load(v_ldli(t0, src, (i+0) * sizeof(long)), rem-1);
        }

        switch(rem) {
        case 3: v_stli(t2, dst, (i + 2) * sizeof(long));
        case 2: v_stli(t1, dst, (i + 1) * sizeof(long));
        case 1: v_stli(t0, dst, (i + 0) * sizeof(long));
        }

	return (memcpy_p) v_end(0).i;
}

int main(int argc, char *argv[]) {
	int n = argc == 2 ? atoi(argv[1]) : 100, i, msize;
	unsigned *mem0, *mem1, *src;
	memcpy_p mp;

	msize = n * sizeof *mem0;
	mem0 = malloc(msize);
	mem1 = malloc(msize);
	src = malloc(msize + 2 * sizeof *mem0);

	for(i = 0; i < n; i++)
		src[i] = rand();

	memcpy(mem0, src, msize);
	mp = gen_simple(msize);
	mp(mem1, src);
	/* v_dump(mp); */

	if(memcmp(mem0, mem1, msize))
		assert(0);

        return 0;
}

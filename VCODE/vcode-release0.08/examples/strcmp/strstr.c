/* Create a specialized memcpy loop for a particular input. */
#include <stdio.h>
#include <malloc.h>
#include "vcode.h"              /* This header file defines all vcode insns. */

typedef int (*strcmp_p)(char *dst);

/* Generate a specialized string compare.  
   Should optimize for word-aligned comparisions. */
strcmp_p gen_strcmp(char *p) {
	unsigned csize, n;
	int i;
	v_code *code;
	v_reg_type t0, t1, dst;
	v_label_type out;

	/* uch.  a hacked guess. */
	n = strlen(p);
	csize = (128 + n * 9 * sizeof *code);
	code = malloc(csize);
	assert(code);

	v_lambda("gen", "%p", &dst, V_LEAF, code, csize);

	if(!v_getreg(&t0, V_I, V_TEMP))
		v_fatal("out of registers");
	if(!v_getreg(&t1, V_P, V_TEMP))
		v_fatal("out of registers");

	out = v_genlabel();

	for(i = 0; *p; i++, p++) {
		v_ldci(t0, dst, i); 
		v_schedule_delay(
			v_bneii(t0, (int)*p, out), 
			v_addpi(t0, dst, i);
	}
	/* done */
	v_retii(0);

	v_label(out);
		v_reti(t0);

	return (strcmp_p) v_end(0).i;
}

int main(int argc, char *argv[]) {
	int n = argc == 2 ? atoi(argv[1]) : 100, i, msize;
	char *str0, *str1, *str;
	strcmp_p sp;

	msize = n * sizeof *str1;
	str0 = malloc(msize);
	str1 = malloc(msize);
	str = malloc(msize);
		
	for(i = 0; i < n-1; i++)
		str[i] = (char)random();

	str[n-1] = 0;
	strcpy(str0, str);
	strcpy(str1, str);
	str1[0]++;

	sp = gen_strcmp(str);

	/* v_dump(sp); */
	if(sp(str0))
		assert(0);
	if(!sp(str1))
		assert(0);

	sp = gen_strcmp("hello, world\n");
	if(sp("hello, world\n") != strcmp("hello, world\n", "hello, world\n"))
		assert(0);
	if(sp("hello world\n") != strcmp("hello, world\n", "hello world\n"))
		assert(0);

        return 0;
}

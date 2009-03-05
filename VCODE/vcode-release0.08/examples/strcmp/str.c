/* Create a specialized memcpy loop for a particular input. */
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include "vcode.h"              /* This header file defines all vcode insns. */

typedef int (*strcmp_p)(char *dst);

/* Generate a specialized string compare.  
   Should optimize for word-aligned comparisions. */
strcmp_p gen_strcmp(unsigned char *p, v_code *code, int nbytes) {
	v_reg_type t0, t1, dst;
	v_label_type out;
	int i;

	v_lambda("gen", "%p", &dst, V_LEAF, code, nbytes);

	if(!v_getreg(&t0, V_U, V_TEMP))
		v_fatal("out of registers");
	if(!v_getreg(&t1, V_I, V_TEMP))
		v_fatal("out of registers");

	out = v_genlabel();

	i = 0;
	do {
		v_lduci(t0, dst, i); 
		v_seti(t1, (int)*p); 
		v_bneui(t0, *p, out); 
		i++;
	} while(*p++);
	
	/* done */
	v_retii(0);

	v_label(out);
		/* need to make signed before subtraction */
		v_lshui(t0, t0, 24);
		v_rshii(t0, t0, 24);

		v_subi(t0, t1, t0);
		v_reti(t0);

	return (strcmp_p) v_end(0).i;
}

int main(int argc, char *argv[]) {
#define MAXSTR 2
	int n = argc == 2 ? atoi(argv[1]) : 100, i, j, res1, res2, m;
	strcmp_p sp;
	static v_code code[MAXSTR * 20 + 128];
	char str[MAXSTR], str2[MAXSTR];

	for(i = 0; i < n; i++) {
		m = random() % MAXSTR;

		for(j = 0; j < m; j++)
			str[j] = (char)random();
		str[j] = 0;

		m = random() % MAXSTR;
		for(j = 0; j < m; j++)
			str2[j] = (char)random();
		str2[j] = 0;
		sp = gen_strcmp(str, code, sizeof code);
		v_dump(sp);

		if(sp(str))
			assert(0);
		res1 = sp(str2);
		res2 = strcmp(str, str2);
		printf("res1 = %d, res2 = %d\n", res1, res2);
		if(res1 < 0)
			assert(res2 < 0);
		else if(res1 > 0)
			assert(res2 > 0);
		else
			assert(!res2);
	}
        return 0;
}

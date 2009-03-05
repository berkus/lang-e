/* Create a specialized memcpy loop for a particular input. */
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include "vcode.h"              /* This header file defines all vcode insns. */

typedef int (*strcmp_p)(char *dst);

/* Recursively create entries for each string character.  */
void mk_strcmp(char *p, v_reg_type dst, v_reg_type t0, int i) {
	v_label_type out;

	out = v_genlabel();
	v_lduci(t0, dst, i); 
	v_bneui(t0, *p, out);

	/* More to do? */
	if(*p)
		mk_strcmp(p+1, dst, t0, i+1);
	/* Done. */
	else
		v_retii(0);

	v_label(out);
		v_subii(t0, t0, *p);
		v_negu(t0, t0);
		v_reti(t0);
}

/* Generate a specialized string compare.  
   Should optimize for word-aligned comparisions. */
strcmp_p gen_strcmp(unsigned char *p, v_code *code, int nbytes) {
	v_reg_type t0, dst;

	v_lambda("gen", "%p", &dst, V_LEAF, code, nbytes);

	if(!v_getreg(&t0, V_U, V_TEMP))
		v_fatal("out of registers");

	mk_strcmp(p, dst, t0, 0);

	return (strcmp_p) v_end(0).i;
}

int main(int argc, char *argv[]) {
#define MAXSTR 2 /* 32 */
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
		if(res1 < 0)
			assert(res2 < 0);
		else if(res1 > 0)
			assert(res2 > 0);
		else
			assert(!res2);
	}
        return 0;
}

/* Test that save/restore and locals do not overlap in the activation record. */
#include <assert.h>
#include <stdio.h>

#include "vcode.h"


v_iptr mk_test(int leaf) {
	static unsigned insn[1000];
	int i, regs, fregs;
	v_reg_type al[10];
	int l[100];
	v_label_type abortl;

	v_reg_type r[32];
	v_reg_type fr[32];

	v_lambda("foo", "%i%d", al, leaf, insn, sizeof insn);

	abortl = v_genlabel();

	for(i = 0; i < 5 && v_getreg(&r[i], V_I, V_TEMP); i++) {
		(void)("allocated register %d\n", i);
		v_seti(r[i], i);
		v_savei(r[i]); 
	}
	regs = i;

	for(i = 0; v_getreg(&fr[i], V_D, V_TEMP); i++) {
		(void)("allocated register %d\n", i);
		v_setd(fr[i], (double)i);
		v_saved(fr[i]); 
	}
	fregs = i;

	for(i = 0; i < 10; i++) {
		l[i] = v_local(V_I);
			
		v_seti(al[0], 100 + i);
		v_stii(al[0], v_lp, l[i]);
	}

	if(leaf == V_NLEAF)
		/* call a procedure, then verify that everything is in place. */
		v_scallv((v_vptr)printf, "%P", "hello world!\n");


	for(i = 0; i < regs; i++) {
		v_restorei(r[i]); 
		v_bneii(r[i], i, abortl);
	}

	for(i = 0; i < fregs; i++) {
		(void)("allocated register %d\n", i);
		v_restored(fr[i]); 
		v_setd(al[1], (double)i);
		v_bned(fr[i], al[1], abortl);	
	}

	for(i = 0; i < 10; i++) {
		v_ldii(al[0], v_lp, l[i]);
		v_bneii(al[0], 100 + i, abortl);
	}
	v_retii(1);

	v_label(abortl);
	v_retii(0);		/* failure. */

	return v_end(0).i;
}

int main() { 
	v_iptr ip;

	ip = mk_test(V_LEAF);

	if(ip()) {
		printf("sucess!\n");
	} else {
		v_dump((void*)ip);
		printf("failure!\n");
	}
	ip = mk_test(V_NLEAF);
	if(ip())
		printf("sucess!\n");
	else {
		v_dump((void*)ip);
		printf("failure!\n");
	}

	return 0;
}

/* Test that we can make registers unavailable. */
#include <assert.h>
#include "vcode.h"

#ifndef __fast__
extern v_reg_type v_reg(int reg);
#endif

int main() { 
	v_code insn[1000];
	v_reg_type r[100];
	int i,j;
	int iters;

	v_mk_unavail(V_I, V_TI0);
	v_mk_unavail(V_I, V_TI1);
	v_mk_unavail(V_I, V_TI2);

	iters = 0;
loop:

 	v_lambda("foo", 0, 0, V_NLEAF, insn, sizeof insn);

	for(i=0; v_getreg(&r[i], V_I, V_TEMP); i++) {
		/* check that we haven't seen it yet. */
		for(j = 0; j < i; j++)
			assert(_vrr(r[i]) != _vrr(r[j]));

		(void)("allocated %d, %d\n", i, _vrr(r[i]));
		assert(_vrr(r[i]) != _vrr(V_TI0));
		assert(_vrr(r[i]) != _vrr(V_TI1));
		assert(_vrr(r[i]) != _vrr(V_TI2));
	}

	v_end(0);

	if(iters++ < 4) goto loop;
	printf("Success!\n");

	return 0;
}

/*
 * Test that calls with pushes and immediates work.  Also tests that 
 * vcode catches uninitialized cstates.
 */
#include <assert.h>
#include <stdio.h>
#include "vcode.h"

int main() { 
	static unsigned insn[1000];
	v_reg_type r;
	struct v_cstate c;
	v_vptr vp;

	v_clambda("foo", V_NLEAF, insn, sizeof insn);
	v_getreg(&r, V_I, V_TEMP);

	v_push_init(&c);
	v_push_argpi(&c, "Hello: %d %d %d %d\n");
	v_seti(r, 10);
	v_push_argi(&c, r);
	v_seti(r,20);
	v_push_argi(&c, r);
	v_seti(r,30);
	v_push_argi(&c, r);
	v_seti(r,40);
	v_push_argi(&c, r);
	v_ccallv(&c, (v_vptr)printf);
	vp = v_end(0).v;
#if 0
	v_dump((void*)vp);
#endif
	vp();

	v_clambda("foo", V_NLEAF, insn, sizeof insn);
	v_push_init(&c);
	v_push_argpi(&c, "Hello: %d %d %d %d\n");
	v_push_argii(&c, 10);
	v_push_argii(&c, 20);
	v_push_argii(&c, 30);
	v_push_argii(&c, 40);
	v_ccallv(&c, (v_vptr)printf);
	vp = v_end(0).v;
#if 0
	v_dump((void*)vp);
#endif
	vp();

#if 0
	v_clambda("foo", V_NLEAF, insn, sizeof insn);
	v_push_argpi(&c, "Hello: %d %d %d %d\n");
#endif
	return 0;
}

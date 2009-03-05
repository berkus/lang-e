#include <stdio.h>
#include "vcode.h"		/* This header file defines all vcode insns. */

int main(void) {
	struct v_cstate c;

	/* Create a function to print standard greeting. */
        v_lambda("hello-world", "", 0, V_NLEAF, malloc(512), 512);
		v_push_init(&c);
		v_push_argpi(&c, "hello, world\n");
		v_ccallv(&c, (v_vptr)printf);
        v_end(0).v(); 	/* Compile & call */
	return 0;
}

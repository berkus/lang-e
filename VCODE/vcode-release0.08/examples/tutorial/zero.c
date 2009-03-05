#include <stdio.h>
#include "vcode.h"		/* This header file defines all vcode insns. */

int main(void) {
	static v_code insn[1000]; 	/* Memory to hold code in. */
        v_iptr ip;

	/* Create a leaf function taking no arguments that returns 0. */
        v_lambda("zero", "", 0, V_LEAF, insn, sizeof insn);
		v_retii(0);  /* Return Integer Immediate. */
        ip = v_end(0).i;      /* Compile and capture Integer function pointer. */
        printf("zero returned %d\n", ip());	/* Run it. */
	return 0;
}

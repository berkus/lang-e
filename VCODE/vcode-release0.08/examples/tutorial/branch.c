#include <stdio.h>
#include "vcode.h"		/* This header file defines all vcode insns. */

int main(void) {
	static v_code insn[1000]; 	/* Memory to hold code in. */
	v_vptr vp;
	v_reg_type x;

        /* Test branch instruction */
        v_lambda("branch", "%i", &x, V_NLEAF, insn, sizeof insn);
        {
		v_label_type true, end;

        	true = v_genlabel();    /* allocate two labels */
        	end = v_genlabel();

        	/* test whether x is equal to 5 */
        	v_beqii(x, 5, true);
                        v_scallv((v_vptr)printf, "%P", "Arg is not equal to 5\n");
                	v_jv(end);      /* jump over else */
        	v_label(true);
                        v_scallv((v_vptr)printf, "%P", "Arg is equal to 5\n");
        	v_label(end);
		v_retv();
        }
        vp = v_end(0).v;
	vp(5);          /* test */
	vp(6);
	return 0;
}

#include <stdio.h>
#include "vcode.h"		/* This header file defines all vcode insns. */

int main(void) {
	static v_code insn[1000]; 	/* Memory to hold code in. */

        /* Test jump and link instruction */
        v_lambda("jump-and-link", "", 0, V_NLEAF, insn, sizeof insn);
        {
                static v_code *linked_addr;
                v_reg_type rdp, rr;
                v_label_type l;

		/* Allocate two registers persistent accross procedure calls. */
                v_getreg(&rdp, V_P, V_VAR);
		/* Allocate register to hold return pointer */
                v_getreg(&rr, V_P, V_VAR);

                l = v_genlabel(); 	  /* Allocate label */
                v_dlabel(&linked_addr, l); /* mark memory to hold target address */
                v_setp(rdp, &linked_addr); /* Load address */
                v_ldpi(rdp, rdp, 0);

                v_scallv((v_vptr)printf, "%P", "Jumping!\n");
                v_jalp(rr, rdp); 	   /* Jump to it. */

                v_scallv((v_vptr)printf, "%P", "Returning!\n");
                v_retv();		/* Done */

                v_label(l);
                        v_scallv((v_vptr)printf, "%P", "Jumping back!\n");
                        v_jp(rr);	/* Jump back to caller. */
        }
        v_end(0).v();
#if 0
{
	v_vptr vp = v_end(0).v;
	v_dump(vp);
	vp();
}
#endif

	return 0;
}

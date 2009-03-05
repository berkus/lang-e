#include <assert.h>
#include <stdio.h>
#include "vcode.h"

int main() { 
	static v_code insn[1000];

        /* jal reg  */
        v_lambda("v_jal", "", 0, V_NLEAF, insn, sizeof insn);
        {
        	static void * linked_addr;
        	v_reg_type rdp;
        	v_reg_type rr;
		v_label_type l;

		v_getreg(&rdp, V_P, V_VAR);
		v_getreg(&rr, V_P, V_VAR);

                l = v_genlabel();
                v_dmark(&linked_addr, l);

                v_ldpi(rdp, v_zero, (unsigned long)&linked_addr);
		v_scallv((v_vptr)printf, "%P", "Jumping!\n");
                v_jalp(rr, rdp);
		v_scallv((v_vptr)printf, "%P", "Returning.\n");	
               	v_retii(13);

                v_label(l);
			v_scallv((v_vptr)printf, "%P", "Jumping back!\n");
			v_jp(rr);
        }
	printf("Testing jalr\n");
        if(v_end(0).i() != 13)
		demand(0, bogus value!);
	return 0;
}

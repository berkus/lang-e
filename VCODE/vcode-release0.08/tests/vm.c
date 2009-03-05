#include <assert.h>
#include <stdio.h>
#include "vcode.h"

int main() { 
	static unsigned insn[100];
	v_iptr ip;
	v_reg_type arg_list[10];
	v_label_type l;
	unsigned s2u, s1u;

	s1u =  3422929224;
	s2u = 4205332841;

        /* reg <- (reg < imm) */
        v_lambda("bltuli", "%u", arg_list, V_LEAF, insn, sizeof insn);
                l = v_genlabel();
                v_bltui(arg_list[0], s2u, l);
                        v_retii(0);
                v_label(l);
                        v_retii(1);

        ip = v_end(0).i;

	v_dump((void*)ip);
	printf("ip returned %d, should be %d\n", ((int (*)(unsigned))ip)(s1u), s1u < s2u);
	return 0;
}

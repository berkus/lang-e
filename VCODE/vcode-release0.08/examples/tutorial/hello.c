#include <stdio.h>
#include "vcode.h"              /* This header file defines all vcode insns. */

int main(void) {
        static v_code insn[1000];      /* Memory to hold code in. */

        /* Create a function to print standard greeting. */
        v_lambda("hello-world", "", 0, V_NLEAF, insn, sizeof insn);
                /* Generate simple call to printf. */
                v_scallv((v_vptr)printf, "%P", "hello, world\n");
        v_end(0).v();   /* Compile & call */
        return 0;
}

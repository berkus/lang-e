#include "vcode-internal.h"

/* Dump code pointed to by code. */
void v_dump (v_code *code) {
#ifdef _V_SOLARIS_
	fprintf(stderr, "vcode does not have a disassembler for solaris.\n");
	fprintf(stderr, "email engler@lcs.mit.edu if you need one.\n");
#else
        extern decode_instr(v_code , v_code *, char *);

        v_code *c;

        for(c = code; c[0] != NOP || c[1] != NOP || c[2] != NOP; c++) {
                char buf[1024];
                decode_instr(*c, c, buf);
                printf("\t0%x:\t0%x\t%s\n",(unsigned)c, *c, buf);
        }
        fflush(stdout);
#endif
}

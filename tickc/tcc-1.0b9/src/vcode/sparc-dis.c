#include "vcode-internal.h"

#ifdef _V_SOLARIS_
#include "dis-asm.h"

#define MAXLENGTH (1<<23) /* Max length of function that can be disassembled */

/* Dump code pointed to by code. */
void v_dump (v_code *code) {
     struct disassemble_info info;
     unsigned int *p;
     int l;

     assert(sizeof(unsigned int) == 4);

     INIT_DISASSEMBLE_INFO(info, stdout);
     info.buffer = (bfd_byte *)code;
     info.buffer_vma = (bfd_vma)code;
     info.buffer_length = MAXLENGTH;

     printf("disassembled:\n");
     p = (unsigned *)info.buffer;
     while (! (p[0] == NOP && p[1] == NOP && p[2] == NOP)) {
	  printf("%p:\t0x%08x\t", p, *p);
	  l = print_insn_sparc((bfd_vma)p, &info);
	  if (l <= 0) break;
	  assert(l == 4);
	  printf("\n");
	  p++;
     }
}
#else
/* Dump code pointed to by code. */
void v_dump (v_code *code) {
        extern decode_instr(v_code , v_code *, char *);

        v_code *c;

        for(c = code; c[0] != NOP || c[1] != NOP || c[2] != NOP; c++) {
                char buf[1024];
                decode_instr(*c, c, buf);
                printf("\t0%x:\t0%x\t%s\n",(unsigned)c, *c, buf);
        }
        fflush(stdout);
}
#endif

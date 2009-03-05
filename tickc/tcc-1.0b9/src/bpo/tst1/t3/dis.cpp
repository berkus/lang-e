#include <assert.h>
#include <stdio.h>
#include <msdis.h>
#include <disx86.h>

#define ALEN 40
#define ILEN 120
#define MAXINST 64

static char taddr[ALEN+1], tbyte[ALEN+1], tinst[ILEN+1];
static DISX86 *d;

void disasminternal (void *code, unsigned int len) {
  unsigned long addr = (unsigned long)code;
  size_t inc, linst, laddr, lbyte;

  if (!d) d = new DISX86(DIS::distX86);

  while ((inc = d->CbDisassemble(addr, (void *)code, len))) {
    linst = d->CchFormatInstr(tinst, ILEN);
    laddr = d->CchFormatAddr(addr, taddr, ALEN);
    lbyte = d->CchFormatBytes(tbyte, ALEN);


    if (linst && lbyte && laddr)
      printf("%s\t%20s\t%s\n", taddr, tbyte, tinst);
    else
      printf("We have a problem [%p : %p : %p : %s : %s : %s].\n", 
	     laddr, lbyte, linst, taddr, tbyte, tinst);
    addr += inc; len -= inc; code = (void*)addr;
  }
}

unsigned int disinsninternal (void *insn) {
  unsigned long addr = (unsigned long)insn;
  size_t inc, linst, laddr;

  if (!d) d = new DISX86(DIS::distX86);

  if ((inc = d->CbDisassemble(addr, insn, MAXINST))) {
    linst = d->CchFormatInstr(tinst, ILEN);
    laddr = d->CchFormatAddr(addr, taddr, ALEN);
    if (linst && laddr && tinst)
      printf("%s\t%s\n", taddr, tinst);
    else
      printf("We have a problem [%p : %p : %s : %s].\n", 
	     laddr, linst, taddr, tinst);
  }
  return (unsigned int)inc;
}

unsigned int insnleninternal (void *insn) {
  if (!d) d = new DISX86(DIS::distX86);
  return (unsigned int)d->CbDisassemble((unsigned long)insn, 
					(void *)insn, MAXINST);
}

extern "C" {
  void disasm (void *code, unsigned int len) {
    disasminternal(code, len);
  }

  unsigned int disinsn (void *insn) {
    return disinsninternal(insn);
  }

  unsigned int insnlen (void *insn) {
    return insnleninternal(insn);
  }
}

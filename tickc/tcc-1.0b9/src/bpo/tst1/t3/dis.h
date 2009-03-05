#ifndef __DIS_H__
#define __DIS_H__

/* disasm: disassemble (to stdout) the 'len' bytes of code beginning at
   'code'.  Prints a diagnostic message if portions of this region do
   not contain executable code. */
extern void disasm (void *code, unsigned int len);

/* disinsn: if insn points to a valid instruction, disassembles the instruction
   to stdout and returns the length (in bytes) of the instruction.
   If insn does not point to a valid instruction, prints a diagnostic
   message and returns 0. */
extern unsigned int disinsn (void *insn);

/* disinsn: if insn points to a valid instruction, returns the length
   (in bytes) of the instruction.
   If insn does not point to a valid instruction, returns 0. */
extern unsigned int insnlen (void *insn);
#endif

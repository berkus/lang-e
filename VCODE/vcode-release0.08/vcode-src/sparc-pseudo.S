
/*  
 * .rem/.mul/.div seem to trash g2,g3,g4 and any out registers.  To guard against
 * this semi-efficiently we call these trampoline functions before calling
 * synthetic instructions in order to save g2,g3,g4 and to protect the original
 * out registers.
 */  

gcc2_compiled.:
___gnu_compiled_c:

#define SAVE_GLOBALS	\
	mov %g2, %l0;	\
	mov %g3, %l1;	\
	mov %g4, %l2;

#define RESTORE_GLOBALS	\
	mov %l0, %g2;	\
	mov %l1, %g3;	\
	mov %l2, %g4;

/* Solaris changed asm naming conventions. */
#ifdef _V_SOLARIS_
#	define PROC(x) .global __ ## x ; __ ## x : 
#else
#	define PROC(x) .global ___ ## x ; ___ ## x : 
#endif

.text
	.align 4
	.proc	04
PROC(mul)
	!#PROLOGUE# 0
	save %sp,-112,%sp
	!#PROLOGUE# 1
	SAVE_GLOBALS
	mov %i0,%o0
	call .umul,0
	mov %i1,%o1
	RESTORE_GLOBALS
	ret
	restore %g0,%o0,%o0
	.align 4
	.proc	04
PROC(div)
	!#PROLOGUE# 0
	save %sp,-112,%sp
	!#PROLOGUE# 1
	SAVE_GLOBALS
	mov %i0,%o0
	call .div,0
	mov %i1,%o1
	RESTORE_GLOBALS
	ret
	restore %g0,%o0,%o0
	.align 4
	.proc	04
PROC(rem)
	!#PROLOGUE# 0
	save %sp,-112,%sp
	!#PROLOGUE# 1
	SAVE_GLOBALS
	mov %i0,%o0
	call .rem,0
	mov %i1,%o1
	RESTORE_GLOBALS
	ret
	restore %g0,%o0,%o0
	.align 4
	.proc	016
PROC(umul)
	!#PROLOGUE# 0
	save %sp,-112,%sp
	!#PROLOGUE# 1
	SAVE_GLOBALS
	mov %i0,%o0
	call .umul,0
	mov %i1,%o1
	RESTORE_GLOBALS
	ret
	restore %g0,%o0,%o0
	.align 4
	.proc	016
PROC(udiv)
	!#PROLOGUE# 0
	save %sp,-112,%sp
	!#PROLOGUE# 1
	SAVE_GLOBALS
	mov %i0,%o0
	call .udiv,0
	mov %i1,%o1
	RESTORE_GLOBALS
	ret
	restore %g0,%o0,%o0
	.align 4
	.proc	016
PROC(urem)
	!#PROLOGUE# 0
	save %sp,-112,%sp
	!#PROLOGUE# 1
	SAVE_GLOBALS
	mov %i0,%o0
	call .urem,0
	mov %i1,%o1
	RESTORE_GLOBALS
	ret
	restore %g0,%o0,%o0

gcc2_compiled.:
___gnu_compiled_c:
.text
	.align 4
	.global _v_flushcache
	.proc	020
_v_flushcache:
	!#PROLOGUE# 0
	!#PROLOGUE# 1
	sll %o1,2,%g2
	iflush %o0
	add %g2,7,%g2
	and %g2,-8,%g2
	mov 0,%g3
	cmp %g3,%g2
	bge L3
	nop
L5:
	iflush %o0
	add %g3,4,%g3
	cmp %g3,%g2
	bl L5
	add %o0,4,%o0
L3:
	nop
	nop
	nop
	nop
	nop
	retl
	nop

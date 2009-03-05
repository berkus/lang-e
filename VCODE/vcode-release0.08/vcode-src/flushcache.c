#include <assert.h>
#include <stdio.h>

#include "vcode-internal.h"
#include "demand.h"

void v_flushcache(void *ptr, int nbytes) {
	int i;

	asm volatile ("iflush %0" : /* no outputs */ : "r" (ptr));

	/* make sure 8 byte aligned (I'm not sure this is necessary) */
	nbytes = v_roundup(nbytes, 8);

	/* Nuke every 8 bytes.   */
	for(i = 0; i < nbytes; i+=8, ptr = (char *)ptr + 8)
		asm volatile ("iflush %0" : /* no outputs */ : "r" (ptr));

	/* page 138: must be 5 instructions */
	asm volatile("nop");	/* 1 */
	asm volatile("nop");	/* 2 */
	asm volatile("nop");	/* 3 */
	asm volatile("nop");	/* 4 */
	asm volatile("nop");	/* 5 */
}

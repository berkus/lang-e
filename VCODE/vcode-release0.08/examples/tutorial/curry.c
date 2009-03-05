#include <stdio.h>
#include "vcode.h"		/* This header file defines all vcode insns. */

/* Fake connection */
struct tcb { 
	unsigned short dst_port;
	unsigned short src_port;
	/* ... */
};

typedef int (*write_ptr)(char *, int);

static int write(struct tcb *tcb, char *msg, int nbytes) {
	printf("dst port = %d, src port = %d\n", tcb->dst_port, tcb->src_port);
	return 1;
}

/* Create a new function to call a write function with
   the given control block hardwired as its first argument. */
write_ptr mkwrite(struct tcb *tcb) {
	v_reg_type arg[2];	/* two arguments */
	v_reg_type res;		/* function result. */


	/* After currying its first argument, write takes a pointer (msg) and an int (nbytes) */
        v_lambda("curry-write", "%p%i", arg, V_NLEAF, malloc(512), 512);
        	/* generate call to write: %P indicates the first argument (tcb) is a constant */
                res = v_scalli((v_iptr)write, "%P%p%u", tcb, arg[0], arg[1]);
        	v_reti(res);	/* return result of calling write */
	return v_end(0).i;
}

int main(void) {
	struct tcb t1, t2;
	write_ptr write1, write2;

	t1.dst_port = 1; 
	t1.src_port = 2; 
 	write1 = mkwrite(&t1);

	t2.dst_port = 3; 
	t2.src_port = 4; 
 	write2 = mkwrite(&t2);

	/* Test write */
	write1(0,0);
	write2(0,0);

	return 0;
}

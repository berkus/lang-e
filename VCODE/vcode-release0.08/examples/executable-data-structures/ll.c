/*
 * Compile a linked list to executable code.  Currently isn't particularly useful.
 * We actually need SMC to make this work well (I'm adding it...).
 */
#include <stdio.h>
#include <malloc.h>
#include "vcode.h"              /* This header file defines all vcode insns. */

struct node {
	struct node *next;
	int val;		/* value to look for */

};

typedef struct node *(*lookup_p)(int n);

/* Stage 1: compile linked list to machine code. */
lookup_p gen_lookup(struct node *l, v_code *code, int nbytes) {
	v_reg_type val;
	v_label_type next;

	/* Actually, I don't think we want to generate a full-fledged function. */
	v_lambda("gen", "%i", &val, V_LEAF, code, nbytes);

	next = v_genlabel();

	for(; l; l = l->next) {
		/* Jump to next comparision if this is not the right value. */
		v_label(next);
		next = v_genlabel();
		v_bneii(val, l->val, next);
			v_retpi(l);
	}
	v_label(next);
	v_retpi(0);

	return (lookup_p) v_end(0).i;
}

int main(int argc, char *argv[]) {
	struct node *head, *p;
	int n = argc == 2 ? atoi(argv[1]) : 100, i, nbytes;
	lookup_p lookup;
        static v_code *insn;

	nbytes = (20 * n * sizeof *insn);
	insn = (void *)malloc(nbytes);

	/* Create linked list of elements. */
	head = p = calloc(1, sizeof *p);
	for(i = 1; i < n; i++) {
		p->next = (void *)calloc(1, sizeof *p);
		p = p->next;
		p->val = i;
	}
		
	/* Now, we generate code from the linked list to search it. */
	lookup = gen_lookup(head, insn, nbytes);

	for(p = head; p; p = p->next) {
		struct node *res;

		res = lookup(p->val);
		if(res != p) {
			printf("res = %p, p = %p\n", res, p);
			printf("p->val = %d\n",p->val);
			printf("res->val = %d, p->val = %d\n", res->val, p->val);
		}
	}

	demand(!lookup(n), bogus structure!);

        return 0;
}

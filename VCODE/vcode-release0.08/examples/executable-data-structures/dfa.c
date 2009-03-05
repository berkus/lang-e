/* 
   Compile a dfa to executable code.  Basically, we could generalize DPF
   to include all of this.  Need OR's, etc.  Might want to just pass
   in functions that perform the required actions: getting the next
   node from this dfa, enumerating all transitions, etc.  
*/
#include <stdio.h>
#include <malloc.h>
#include "vcode.h"              /* This header file defines all vcode insns. */

/* the transition f(xition) = dst. */
struct xition {
	struct dfa *dst;	/* node we jump too. */
	int xition;		/* the value that we make this transition on. */
}; 

/* node of the dfa */
struct dfa {
	unsigned char n; 	/* number of transitions out of this state. */
	/* attributes */
	unsigned accept_p:1,		/* is this an accept? */
	struct xition x[1];	/* struct hack */ 
};

#if 0
	unsigned 	longest_match:1, 	/* return on match or continue? */
			failnode:1;		/* is there a failure node? */
#endif

typedef int (*dfa_p)(char *);

/* very very simple dfa creation. */
struct dfa create_dfa(char *pat) {
	struct dfa *p, *dfa, *end;

	for(p = last = 0, n = strlen(pat); n-- > 0; ) {
		p = malloc(sizeof *d);	
		p->n = 1;
		p->x[0].xition = pat[n];
		p->x[0].dst = (pat[n] != '*') ? last : p;
		last = p;
		if(!end)
			end = last;
	}
	if(end)
		end->accept_p = 1;
	return p;
}

dfa_p compile_dfa(struct dfa *dfa) {

}


int main(int argc, char *argv[]) {
	dfa_p dfa;
	struct dfa *p;

	/* 
	 * Create a DFA to match (a*).  Semantics are a bit funny: we return
	 * a boolean if the given string matched. 
	 */
	p = create_dfa("a*");
	dfa = compile_dfa(p);

	p = create_dfa("abcd");
	dfa = compile_dfa(p);

        return 0;
}

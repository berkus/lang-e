/* Compile a sorted integer vector to machine code that performs a binary search
   on it.  */
#include "vcode.h"

/* array from 0..n-1, return position or -1 */
int search(int x[], int n, int key) {
	int l, u, r;

	for(l = 0, u = n-1, r = n / 2; l <= u ; r/=2 ) {
		int p = u - r;
		if(x[p] == key) {
			return p;
		} else if(x[p] < key) {
			l = p+1;
		} else {
			u = p-1;
		}	
	}
	return -1;
}

/* recursively gen code to perform binary search. */
void gen(int *x, v_reg_type key, int l,  int u, int r) {
	int p; 
	v_label_type next;

	/* printf("making with l=%d,u=%d,r=%d\n",l,u,r); */
	if(l > u) {
		v_retii(-1);
		return;
	}

	p = u - r;

	next = v_genlabel();

	/*
               	if($(x[p]) == @key)
                        return $p;
                if($(x[p]) < @key)
                        @gen(x, key, p+1, u, r/2);
                else
                        @gen(x, key, l, p-1, r/2);
	*/

	v_bneii(key, x[p], next);
		v_retii(p);
	v_label(next);
	next = v_genlabel();

	v_bltii(key, x[p], next);
		gen(x, key, p+1, u, r/2);
	v_label(next);
		gen(x, key, l, p-1, r/2);
}

typedef int (*hash_ptr)(int key);

/* array from 0..n-1, return position or -1 */
hash_ptr mksearch(int n, int *x) {
	static v_code insn[1024];
	v_reg_type val;

	v_lambda("bin-search", "%i", &val, V_LEAF, insn, sizeof insn);
		gen(x, val, 0, n-1, n/2);
	return (hash_ptr)v_end(0).i;
}

#define n 16

int main(int argc, char *argv[]) { 
     	int x[n], i;
     	hash_ptr hash;

     	for(i=0; i<n; i++) 
		x[i] = i;

  	hash = mksearch(n, x);

	for(i=0;i<2*n;i++) {
		int key = rand() % (n+100), /* get range + 1 */

		pos = search(x,n,key),
		dpos = hash(key);

		if(pos != dpos)
			printf("error, iter %d <key = %d>: %d != %d\n", 
				i,key,pos,dpos);

		if(!((key >= n && pos == -1) || (key < n && x[pos] == key)))
			printf("search %d == %d\n",key, pos);
	}
	return 0;
}

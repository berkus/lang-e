#include <stdio.h>
#include <icode.h>

struct elem { int x; int y; int z; };
struct elem *array;

int compare(void *x, void *y) { return *(int *)x < *(int *)y; }

#define size 12
#define ndata 5

void swap (long * src, long * dst) {
     long tmp;
     int i;

     for(i = 0; i <  size/4; i++) {
	  tmp = src[i]; 
	  src[i] = dst[i]; 
	  dst[i] = tmp; 
     };
}

void adjust (int m/*, int n*/) {
     void *v = (void *)array;
     int n = ndata;
     long * bk;
     long * bj;
     char *b, *temp;
     int j, k;
     static long buf[128];

     printf("adjust\n");
     assert(size % 4 == 0);
     
     b = ((char *)v - size);
     temp = (char *)buf;
     
     j = m;
     k = m * 2;
     while (k <= n) { 
	  if (k < n
	      && (*compare)(b + size * k, b + size * ( k + 1)))
	       ++k;
	  bk = (long *)(b + k * size);
	  bj = (long *)(b + j * size);
	  if (compare(bj, bk))
	       swap(bj, bk);
	  j = k;
	  k *= 2;
     }
}

v_iptr mkcode() {
     unsigned int offset;
     v_iptr ip;
     i_local_t r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11,
	  r12, r13, r14, r15;
     i_label_t L0, L1, L2, L3, L4, L5, L6, L7, L8, L9;

     i_init(100);
     r0 = i_param(I_P);
     r1 = i_param(I_I);
     r7 = i_local(0, I_I);
     L0 = i_mklabel();
     L3 = i_mklabel();
     L4 = i_mklabel();

     i_seti(r7,2);
     i_jpi(L3);
     i_label(L4);
     i_argi(r7);
     i_callvi(adjust);
     i_subii(r7, r7, 1);
     i_label(L3);
     i_bgtii(r7, 0, L4);

     i_end();
     ip = i_emit(&offset, 0).i;
     if (ip) v_dump((void*)ip);
     return ip;
}

dumpelem(struct elem *array, int n) {
	int i;

	for(i = 0; i < n; i++)
	     printf("array[%d] = {%d,%d,%d}\n", i, 
		    array[i].x, array[i].y, array[i].z);
}

int main(int argc, char **argv) { 
     int i;
     v_iptr ip;

     array = (struct elem *)malloc(ndata*sizeof (struct elem));

     srandom(1);
     for (i = 0; i < ndata; i++)
	  array[i].x = random();

     i_debug_on();
     if (argv[1]) {
	  if (!strncmp(argv[1], "ls")) i_ralloc_ls();
	  else if (!strncmp(argv[1], "lr")) i_ralloc_lr();
	  else if (!strncmp(argv[1], "ez")) i_ralloc_ez();
	  else if (!strncmp(argv[1], "gc")) i_ralloc_gc();
     }
     ip = mkcode();

     dumpelem(array, ndata);
     (*ip)(array, ndata);
     printf("====================\n");
     dumpelem(array, ndata);
}

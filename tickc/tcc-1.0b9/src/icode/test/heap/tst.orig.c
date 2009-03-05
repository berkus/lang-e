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

void adjust (int m, int n) {
     void *v = (void *)array;
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
     r2 = i_local(0, I_I);
     r3 = i_local(0, I_I);
     r4 = i_local(0, I_I);
     r5 = i_local(0, I_I);
     r6 = i_local(0, I_I);
     r7 = i_local(0, I_I);
     r8 = i_local(0, I_I);
     r9 = i_local(0, I_I);
     r10 = i_local(0, I_I);
     r11 = i_local(0, I_I);
     r12 = i_local(0, I_I);
     r13 = i_local(0, I_I);
     r14 = i_local(0, I_I);
     r15 = i_local(0, I_I);
     L0 = i_mklabel();
     L3 = i_mklabel();
     L4 = i_mklabel();
     L5 = i_mklabel();
     L6 = i_mklabel();
     L7 = i_mklabel();
     L8 = i_mklabel();
     L9 = i_mklabel();

/*     i_setp(r5, 570804);*/
/*     i_subpi(r6, r0, 12);*/
#if 0
     i_divii(r9, r1, 2);
     i_movi(r7, r9);
#endif
     i_seti(r7,2);
/*     i_refmul(10);*/
     i_jpi(L3);
     i_label(L4);
/*     i_argp(r0);*/
     i_argi(r7);
     i_argi(r1);
     i_callvi(adjust);
     i_label(L5);
     i_subii(r7, r7, 1);
     i_label(L3);
     i_bgtii(r7, 0, L4);
/*     i_refdiv(10);*/

#if 0
     i_subii(r7, r1, 1);
     i_refmul(10);
     i_jpi(L7);
     i_label(L8);
     i_addpi(r2, r6, 12);
     i_seti(r9, 1);
     i_addii(r8, r7, 1);
     i_mulii(r8, r8, 12);
     i_addp(r3, r8, r6);
     i_movp(r15, r2);
     i_ldii(r14, r15, 0);
     i_movi(r4, r14);
     i_ldii(r14, r3, 0);
     i_stii(r14, r15, 0);
     i_stii(r4, r3, 0);
     i_movp(r13, r2);
     i_ldii(r12, r13, 4);
     i_movi(r4, r12);
     i_ldii(r12, r3, 4);
     i_stii(r12, r13, 4);
     i_stii(r4, r3, 4);
     i_movp(r11, r2);
     i_ldii(r10, r11, 8);
     i_movi(r4, r10);
     i_ldii(r10, r3, 8);
     i_stii(r10, r11, 8);
     i_stii(r4, r3, 8);
     i_argp(r0);
     i_argi(r9);
     i_argi(r7);
     i_callvi(adjust);
     i_label(L9);
     i_subii(r7, r7, 1);
     i_label(L7);
     i_bgtii(r7, 0, L8);
     i_refdiv(10);
#endif

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
     i_ralloc_gc();
     ip = mkcode();

     dumpelem(array, ndata);
     (*ip)(array, ndata);
     printf("====================\n");
     dumpelem(array, ndata);
}

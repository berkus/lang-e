/*
 * For matrix B and  constant c, compute c*B
 */
#include <stdio.h>
#include <malloc.h>
#include "vcode.h"
#include "stopwatch.h"

typedef unsigned short data_t;

#ifdef ALPHA
	void *malloc(unsigned);
	void *calloc(unsigned, unsigned);
#endif

static data_t **new_matrix(int n,int m);
static void m_print(char *s, data_t **a, int n);
void cmuli(v_reg_type rd, v_reg_type rs, int x);
static int disass, pixie;

/* driver */
static void scale(int n, data_t **a, data_t **b, data_t c) {
	v_reg_type  dst,src, src_end, v0, v1, al[10];
	v_label_type  loop1; 
	static unsigned insn[1024];
	v_vptr vp;

	/* simple unroll */
	assert(n >= 4 && (n % 4) == 0);

	v_lambda("foo", "%p%p", al, V_LEAF, insn, sizeof insn);

	/* row & c come in as parameters */
	dst 	= al[0];
	src 	= al[1];
	if(!v_getreg(&src_end, V_P, V_TEMP))
		v_fatal("scale: out of registers\n");
	if(!v_getreg(&v0, V_US, V_TEMP))
		v_fatal("scale: out of registers\n");
	if(!v_getreg(&v1, V_US, V_TEMP))
		v_fatal("scale: out of registers\n");

	loop1	= v_genlabel();

	/* relies on contigous memory */
	v_raw_load(v_ldpi(src, src, (0)), 1);	/* perform loads without interlocks */
	v_raw_load(v_ldpi(dst, dst, (0)), 1);
	v_addpi(src_end, src, (n * n) * sizeof **a);

	v_label(loop1);
		/* load 2 to get rid of delay slots */
		v_raw_load(v_ldusi(v0, src, (0 * sizeof **a)), 1);
		v_raw_load(v_ldusi(v1, src, (1 * sizeof **a)), 1);

		/* multiplies will be strength reduced */
		cmuli(v0, v0, c);
		v_addpi(dst, dst, (2 * sizeof **a));
		cmuli(v1, v1, c);
		v_stusi(v0, dst, -(2 * sizeof **a));
		v_addpi(src, src, (2 * sizeof **a));
	/* schedule delay slot instructions */
	v_schedule_delay(
		v_bltp(src, src_end, loop1),
		v_stusi(v1, dst, -(1 * sizeof **a))
	);

	v_retv();
	vp = v_end(0).v;
	if(disass)
		v_dump((void *)vp);
	if(!pixie)
		vp(a,b);	/* perform multiplication */
}

int main(int argc, char *argv[]) { 
	int i,j,trials=1, verbose=0,n = 3;
	data_t **a,**b,mask=1023,c = 7;

    	/* get options */
    	for (i=1; i<argc; i++)
      		if (strcmp(argv[i],"-n") == 0) n = atoi(argv[++i]);
      		else if (strcmp(argv[i],"-t") == 0) trials = atoi(argv[++i]);
      		else if (strcmp(argv[i],"-m") == 0) mask = atoi(argv[++i]);
      		else if (strcmp(argv[i],"-c") == 0) c = atoi(argv[++i]);
      		else if (strcmp(argv[i],"-v") == 0) verbose = 1;
      		else if (strcmp(argv[i],"-d") == 0) disass = 1;
      		else if (strcmp(argv[i],"-p") == 0) pixie = 1;

	/* allocate and initialize */
	a = new_matrix(n,n);
	b = new_matrix(n,n);
    	for(i=0;i<n;i++) {
      		for(j=0;j<n;j++) {
                	b[i][j] = (rand() & mask) + 1;
		}
	}
	if(verbose) {
		printf("constant = %u\n",c);
		m_print("B", b, n);
	}
        printf("doing constants 1 to %d\n",c);
        for(i=0;i<trials;i++) {
                int c1;
                if(!verbose) startwatch(0);
		for(c1 = 0; c1 < c; c1++)
                        scale(n, a, b, c1);
                if(!verbose) stopwatch(0);
        }
	if(verbose) m_print("A", a, n);
	return 0;
}
static void m_print(char *s, data_t **a, int n) {
  	int i, j;

  	printf("\n%s:", s);
  	for(i = 0; i < n; i++) {
    		printf("\n");
    		for(j = 0; j < n; j++)
      			printf(" %u ", a[i][j]);
    	}
  	printf("\n\n");
}
static data_t **new_matrix(int n,int m) {
  	int i;
  	data_t **matrix;
  	data_t *tmp;

  	if(!(matrix = (data_t **)calloc(n,sizeof(data_t *))))
		assert(0);
	if(!(tmp = (data_t *)calloc(n*n,sizeof(data_t)))) 
		assert(0);
  	for(i = 0; i < n; i++) 
		matrix[i] = &tmp[i*n];
  	return matrix;
}

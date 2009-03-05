/*
 * For matrix B and  constant c, compute c*B
 */
#include <stdio.h>
#include "vcode.h"
#include "stopwatch.h"

typedef int (*fptr)();

typedef unsigned short data_t;

static data_t **new_matrix(int n,int m);
static void m_print(char *s, data_t **a, int n);

/* driver */
static void scale(int n, data_t **A, data_t **B, data_t c) {
	/* contigous memory */
	data_t *a = A[0], *b = B[0], *end;

	/* assert(n >= 4 && !(n&3)); */

	end = a + n * n;

	for(; a != end; a+=2, b+=2) {
		a[0] = b[0] * c;
		a[1] = b[1] * c;
	}
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
#if 0
        scale(n, a, b, c);
#endif
        printf("doing constants 1 to %d\n",c);
        for(i=0;i<trials;i++) {
                int c1;
                if(!verbose) startwatch(0);
                for(c1 = 0; c1 < c; c1++)
                        scale(n, a, b,c1);
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

#include <stdio.h>
#include <stdlib.h>
#include "bpo.h"

typedef unsigned char uchar;

uchar id[][8] = {
     { (uchar)0xff, (uchar)0x12, (uchar)0x34, (uchar)0xba, 
       (uchar)0xf0, (uchar)0xbe, (uchar)0x00, (uchar)0x00 },
     { (uchar)0xcc, (uchar)0xcc, (uchar)0xcc, (uchar)0xcc, 
       (uchar)0xcc, (uchar)0x81, (uchar)0xaa, (uchar)0xbb },
     { (uchar)0xa1, (uchar)0xcc, (uchar)0xcc, (uchar)0xcc, 
       (uchar)0xcc, (uchar)0xc1, (uchar)0xde, (uchar)0xad }
};
#define len(x) (sizeof x / sizeof x[0])

int fd[][8] = {
     { 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01 },
     { 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00 },
     { 0x01, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00 }
};

uchar od[] = {
     (uchar)0x00, (uchar)0x00, (uchar)0x00, (uchar)0x00,
     (uchar)0x00, (uchar)0x00, (uchar)0x00, (uchar)0x00,
     (uchar)0x00, (uchar)0x00, (uchar)0x00, (uchar)0x00,
     (uchar)0x00, (uchar)0x00, (uchar)0x00, (uchar)0x00,
     (uchar)0x00, (uchar)0x00, (uchar)0x00, (uchar)0x00
};
     
W add1 (W p) { return p+1; }

int main(int argc, char **argv) {
     uchar *p;
     int i, j;
     int * rdo = (int *)malloc(16*sizeof(int));

     for (i = 0; i < len(id); i++) {
	  uchar *il = id[i]+len(id[i]);
	  uchar *ol = od+len(od);
	  for (j = 0; j < 16; j++) rdo[j] = 0;
	  printf("INPUT:\n");
	  for (p = id[i]; p < il; p++)
	       printf("0x%x:: 0x%x\n", p, (uchar)(*p)&0xFF);
	  ol = bpo(id[i], il, id[i], od, od, ol, add1, fd[i], rdo, 
	      argc > 1 ? atoi(argv[1]) : 0);
	  printf("OUTPUT:\n");
	  for (p = od; p < ol; p++)
	       printf("0x%x:: 0x%x\n", p, (uchar)(*p)&0xFF);
	  printf("RDO: ");
	  for (j = 0; j < (ol-od); j++)
	       printf("%d ", rdo[j]);
	  printf("\n");
     }
     return 0;
}

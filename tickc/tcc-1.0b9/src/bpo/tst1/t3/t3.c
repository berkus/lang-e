#include <stdio.h>
#include <stdlib.h>
#include "bpo.h"
#include "dis.h"

typedef unsigned char uchar;

uchar id[][20] = {
     { (uchar)0x8B, (uchar)0x45, (uchar)0x00, (uchar)0x33, 
       (uchar)0xC9, (uchar)0x89, (uchar)0x78, (uchar)0x5A,
       (uchar)0x3B, (uchar)0xF9, (uchar)0x89, (uchar)0x58,
       (uchar)0x5E, (uchar)0x89, (uchar)0x0E, (uchar)0x74,
       (uchar)0x37, (uchar)0x00, (uchar)0x00, (uchar)0x00
     },
};
#define len(x) (sizeof x / sizeof x[0])

int fd[][20] = {
     { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
       0x00, 0x00, 0x00, 0x00
     },
};

uchar od[] = {
     (uchar)0x00, (uchar)0x00, (uchar)0x00, (uchar)0x00,
     (uchar)0x00, (uchar)0x00, (uchar)0x00, (uchar)0x00,
     (uchar)0x00, (uchar)0x00, (uchar)0x00, (uchar)0x00,
     (uchar)0x00, (uchar)0x00, (uchar)0x00, (uchar)0x00,
     (uchar)0x00, (uchar)0x00, (uchar)0x00, (uchar)0x00,
     (uchar)0x00, (uchar)0x00, (uchar)0x00, (uchar)0x00,
     (uchar)0x00, (uchar)0x00, (uchar)0x00, (uchar)0x00,
     (uchar)0x00, (uchar)0x00, (uchar)0x00, (uchar)0x00
};
     
uchar *nextinsn (uchar *insn) { return insn + disinsn((void*)insn); }

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
	  ol = bpo(id[i], il, id[i], od, od, ol, nextinsn, fd[i], rdo, 
	      argc > 1 ? atoi(argv[1]) : 0);
	  printf("OUTPUT:\n");
	  for (p = od; p < ol; p++)
	       printf("0x%x:: 0x%x\n", p, (uchar)(*p)&0xFF);
	  printf("RDO: ");
	  for (j = 0; j < (ol-od); j++)
	       printf("%d ", rdo[j]);
	  printf("\n");
     }
     disasm(od, 20);
     return 0;
}

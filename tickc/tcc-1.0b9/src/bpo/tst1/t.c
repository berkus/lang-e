#include <stdio.h>
#include "bpo.h"

char id[][8] = 
{
     { 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00 },
     { 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef },
     
};
#define len(x) (sizeof x / sizeof x[0])

char od[] = {
     0x00,
     0x00,
     0x00,
     0x00,
     0x00,
     0x00,
     0x00,
     0x00
};
char *ol = od+len(id[0]);
     
W add1 (W p) { return p+1; }

int main(int argc, char **argv) {
     char *p, *lim;
     int i;
     for (i = 0; i < len(id); i++) {
	  char *il = id[i]+len(id[i]);
	  printf("INPUT:\n");
	  for (p = id[i]; p < il; p++)
	       printf("0x%x:: 0x%x\n", p, (char)(*p)&0xFF);
	  lim = bpo(id[i], il, id[i], od, od, ol, add1, 0L, 0L, 
	      argc > 1 ? atoi(argv[1]) : 0);
	  printf("OUTPUT:\n");
	  for (p = od; p < lim; p++)
	       printf("0x%x:: 0x%x\n", p, (char)(*p)&0xFF);
	  printf("\n");
     }
     {
	  char in[1] = { 0x81 };
	  char out[1];
	  printf("**in=0x%x, out=0x%x, <in>=0x%x\n", in, out, in[0]&0xFF);
	  bpo(in, in+1, in, out, out, out+1, add1, 0L, 0L, 1);
	  printf("**<out>=0x%x\n", (char)out[0]&0xFF);
     }
}

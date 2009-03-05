#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if !(defined(__BYTE__) || defined(__LONG__) || defined(__VCODE__))
#define __BYTE__
#endif
#include "bpo.h"

void main (int argc, char **argv) {
     int i;
     int buflen = 128;
     int debug = 0;
     int seed = 1;
     long int *lp, *ibufh, *ibuft;
     long int *obufh, *obuft;
     char *cp;

     for (i = 1; i < argc; i++)
	  if (!strcmp("-d", argv[i]) && i+1 < argc)
	       debug = atoi(argv[++i]);
	  else if (!strcmp("-l", argv[i]) && i+1 < argc)
	       buflen = atoi(argv[++i]);
	  else if (!strcmp("-s", argv[i]) && i+1 < argc)
	       seed = atoi(argv[++i]);
	  else
	       printf("Unrecognized option `%s'\n", argv[i]);

     if (buflen <= 0) return;

     ibufh = (long int *)malloc(buflen * sizeof(long int));
     ibuft = ibufh + buflen;
     obufh = (long int *)malloc(buflen * sizeof(long int));
     obuft = obufh + buflen;

     srandom((unsigned int)seed);
     for (lp = ibufh; lp < ibuft; *lp++ = random()) ;

     if (debug) {
	  printf("INPUT:\n");
	  for (cp = (char *)ibufh; cp < (char *)ibuft; cp++)
	       printf("0x%2x ", *cp&0xff);
	  printf("\n");
     }

     obuft = (long int *)bpo((char*)ibufh, (char*)ibuft, (char*)ibufh,
			     (char*)obufh, (char*)obufh, (char*)obuft, 
			     NULL, 0, 0, debug);
     
     if (debug) {
	  printf("OUTPUT:\n");
	  for (cp = (char *)ibufh; cp < (char *)ibuft; cp++)
	       printf("0x%2x ", *cp&0xff);
	  printf("\n");
     }

     free(ibufh);
     free(obufh);
}

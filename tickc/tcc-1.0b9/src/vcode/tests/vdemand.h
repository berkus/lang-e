#include <stdio.h>
#define vdemand(bool, msg) \
	(void)(__vdemand(bool, #bool, #msg, __FILE__, __LINE__, ip))
#define vdemand2(bool, msg) \
	(void)(__vdemand(bool, #bool, #msg, __FILE__, __LINE__, ip2))

static int __vdemand (int bbool, char *bool, char *msg, char *file, 
		      int line, int (*ip)()) {
     if (bbool) {
/*	  printf("%s passed\n", msg);*/
	  return 1;
     }
     printf("file %s, line %d: %s\n",file,line,bool);
     printf("%s failed\n",msg);
     v_errors++;
     v_dump((void *)ip);
     return 0;
}

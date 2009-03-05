#include <stdio.h>
#define vdemand(bool, msg) \
	(void)((bool) || __vdemand(#bool, #msg, __FILE__, __LINE__, ip))
#define vdemand2(bool, msg) \
	(void)((bool) || __vdemand(#bool, #msg, __FILE__, __LINE__, ip2))

static int __vdemand(char *bool, char *msg, char *file, int line, int (*ip)()) {
        printf("file %s, line %d: %s\n",file,line,bool);
        printf("%s\n",msg);
	v_errors++;
	v_dump((void *)ip);
	return 0;
}

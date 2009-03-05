/* $Id: string.h,v 1.1.1.1 1997/12/05 01:25:43 maxp Exp $ */

#ifndef __SSTRING_H__
#define __SSTRING_H__

#include <stdarg.h>
#include <string.h>

extern char *string(char *str);
extern char *stringd(int n);
extern char *stringn(char *str, int len);
extern char *vstring(char *str, ...);
extern char *stringf(char *fmt, ...);
#endif

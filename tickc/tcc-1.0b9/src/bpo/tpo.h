/* $Id: tpo.h,v 1.1.1.1 1997/12/05 01:25:43 maxp Exp $ */

#ifndef __TPO_H__
#define __TPO_H__

#include <stdio.h>
#include "bpo.h"

typedef struct {
     int ni;
     char **ins;
} trule_t;

extern trule_t tpoR[];		/* Rules */
extern unsigned int tpoNP;	/* Number of patterns */
extern char *tpoP[];		/* Patterns */

extern void tpo (char *inbuf, char *outbuf, char *outlim, int rulep, int dp);

#endif

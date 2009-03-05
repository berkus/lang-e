/* $Id: cwmisc.h,v 1.1.1.1 1997/12/05 01:25:44 maxp Exp $ */

#ifndef __CWMISC_H__
#define __CWMISC_H__

extern char *indent (char *ind);

extern int leftbit (T w);
extern int rightbit (T w);

extern int minlen (rule_t  **rv);
extern int maxlen (rule_t **rv);
extern T maskatpos (rule_t **r, int pos);
extern int valatpos (rule_t **r, int pos, T msk, T *val);

extern int card (T w, rule_t **r, int pos);
extern int iscritical (T w, rule_t **r, int pos);
extern T criticalwindow (rule_t **r, int pos, T msk);
extern int cwcard (rule_t **r, int pos);
extern int cwpos (rule_t **r, int mil, marktype status);

extern void filterlen (rule_t **r, int mil, rule_t ***s, rule_t ***l);
extern rule_t **filterwin (rule_t **r, int pos, T w, int val);

#endif

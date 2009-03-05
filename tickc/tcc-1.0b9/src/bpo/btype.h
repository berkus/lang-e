/* $Id: btype.h,v 1.2 1997/12/11 01:26:12 maxp Exp $ */

#ifndef __BTYPE_H__
#define __BTYPE_H__

#if defined(__VCODE__)

#if defined(__SIMPLE_SCALAR__) || defined(__sslittle__) || defined(__ssbig__)
typedef struct { unsigned l; unsigned h; } T;
#else
typedef unsigned T;
#endif

typedef T *W;

#define TMAX       ((unsigned)-1)
#define ASGN(l,r)  (*(l) = *(r))
#define INCR(a)    (++(a))
#define DECR(a)    (--(a))
#define GT(l,r)    ((l) > (r))
#define LOAD(a)    (*(a))
#define STORE(a,b) (*(a) = (b))

#elif defined(__LONG__)

typedef unsigned long T;	/* Base type for opts */
typedef T *W;

#define TMAX       ((unsigned long)-1)
#define ASGN(l,r)  (*(l) = *(r))
#define INCR(a)    (++(a))
#define DECR(a)    (--(a))
#define GT(l,r)    ((l) > (r))
#define LOAD(a)    (*(a))
#define STORE(a,b) (*(a) = (b))

#elif defined(__BYTE__)

typedef unsigned char T;	/* Base type for opts */
typedef T *W;			/* Wrapper for T (all prev/next/etc references
				   occur through W objects) */
#define TMAX       ((unsigned char)-1)
#define ASGN(l,r)  (*(l) = *(r))
#define INCR(a)    (++(a))
#define DECR(a)    (--(a))
#define GT(l,r)    ((l) > (r))
#define LOAD(a)    (*(a))
#define STORE(a,b) (*(a) = (b))

#else

#error "No btype defined."

#endif

#endif

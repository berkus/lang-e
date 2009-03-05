/* The <math.h> header contains prototypes for mathematical functions
 * for Linux.
 */

#ifndef _MATH_H
#define _MATH_H

#include <features.h>

__BEGIN_DECLS

extern __CONSTVALUE double acos __P ((double __x)) __CONSTVALUE2;
extern __CONSTVALUE double acosh __P ((double __x)) __CONSTVALUE2;
extern __CONSTVALUE double asin __P ((double __x)) __CONSTVALUE2;
extern __CONSTVALUE double asinh __P ((double __x)) __CONSTVALUE2;
extern __CONSTVALUE double atan __P ((double __x)) __CONSTVALUE2;
extern __CONSTVALUE double atan2 __P ((double __y, double __x)) __CONSTVALUE2;
extern __CONSTVALUE double atanh __P ((double __x)) __CONSTVALUE2;
extern __CONSTVALUE double ceil __P ((double __x)) __CONSTVALUE2;
extern __CONSTVALUE double cos __P ((double __x)) __CONSTVALUE2;
extern __CONSTVALUE double cosh __P ((double __x)) __CONSTVALUE2;
extern __CONSTVALUE double drem __P ((double __x, double __y)) __CONSTVALUE2;
extern __CONSTVALUE double exp __P ((double __x)) __CONSTVALUE2;
extern __CONSTVALUE double fabs __P ((double __x)) __CONSTVALUE2;
extern __CONSTVALUE double floor __P ((double __x)) __CONSTVALUE2;
extern __CONSTVALUE double fmod __P ((double __x, double __y)) __CONSTVALUE2;
extern __CONSTVALUE double hypot __P ((double __x, double __y)) __CONSTVALUE2;
extern              double frexp __P ((double __x, int *__exp));
extern __CONSTVALUE double ldexp __P ((double __x, int __exp)) __CONSTVALUE2;
extern __CONSTVALUE double log __P ((double __x)) __CONSTVALUE2;
extern __CONSTVALUE double log10 __P ((double __x)) __CONSTVALUE2;
extern              double modf __P ((double __x, double *__iptr));
extern __CONSTVALUE double pow __P ((double __x, double __y)) __CONSTVALUE2;
extern __CONSTVALUE double pow2 __P ((double __x)) __CONSTVALUE2;
extern __CONSTVALUE double pow10 __P ((double __x)) __CONSTVALUE2;
extern __CONSTVALUE double sin __P ((double __x)) __CONSTVALUE2;
extern __CONSTVALUE double sinh __P ((double __x)) __CONSTVALUE2;
extern __CONSTVALUE double sqrt __P ((double __x)) __CONSTVALUE2;
extern __CONSTVALUE double tan __P ((double __x)) __CONSTVALUE2;
extern __CONSTVALUE double tanh __P ((double __x)) __CONSTVALUE2;

extern __CONSTVALUE double erf  __P ((double __x)) __CONSTVALUE2;
extern __CONSTVALUE double erfc  __P ((double __x)) __CONSTVALUE2;
extern __CONSTVALUE double j0  __P ((double __x)) __CONSTVALUE2;
extern __CONSTVALUE double j1  __P ((double __x)) __CONSTVALUE2;
extern __CONSTVALUE double jn  __P ((int __n, double __x)) __CONSTVALUE2;
extern __CONSTVALUE double lgamma  __P ((double __x)) __CONSTVALUE2;
extern __CONSTVALUE double y0  __P ((double __x)) __CONSTVALUE2;
extern __CONSTVALUE double y1  __P ((double __x)) __CONSTVALUE2;
extern __CONSTVALUE double yn  __P ((int __n, double __x)) __CONSTVALUE2;

/* Some new functions */
extern __CONSTVALUE double expm1 __P ((double __x)) __CONSTVALUE2;
extern __CONSTVALUE double log1p __P ((double __x)) __CONSTVALUE2;
extern __CONSTVALUE double cbrt  __P ((double __x)) __CONSTVALUE2;

/* Return if VALUE is finite or NaN, +1 if it
   is +Infinity, -1 if it is -Infinity.  */
extern __CONSTVALUE int __isinf __P ((double __value)) __CONSTVALUE2;

/* Return nonzero if VALUE is not a number.  */
extern __CONSTVALUE int __isnan __P ((double __value)) __CONSTVALUE2;

/* Return nonzero if VALUE is finite and not NaN.  */
extern __CONSTVALUE int __finite __P ((double __value)) __CONSTVALUE2;
#ifdef	__OPTIMIZE__
#define	__finite(value)	(!__isinf (value) && !__isnan (value))
#endif

/* Deal with an infinite or NaN result.
   If ERROR is ERANGE, result is +Inf;
   if ERROR is - ERANGE, result is -Inf;
   otherwise result is NaN.
   This will set `errno' to either ERANGE or EDOM,
   and may return an infinity or NaN, or may do something else.  */
extern double __infnan __P ((int __error));

/* Return X with its signed changed to Y's.  */
extern __CONSTVALUE double __copysign __P ((double __x,
					    double __y)) __CONSTVALUE2;

#ifdef	__USE_MISC

/* Return the integer nearest X in the direction of the
   prevailing rounding mode.  */
extern __CONSTVALUE double __rint __P ((double __x)) __CONSTVALUE2;
extern __CONSTVALUE double rint __P ((double __x)) __CONSTVALUE2;

extern __CONSTVALUE int isinf __P ((double __value)) __CONSTVALUE2;
extern __CONSTVALUE int isnan __P ((double __value)) __CONSTVALUE2;
extern __CONSTVALUE int finite __P ((double __value)) __CONSTVALUE2;
extern __CONSTVALUE double infnan __P ((int __error)) __CONSTVALUE2;
extern __CONSTVALUE double copysign __P ((double __x,
					  double __y)) __CONSTVALUE2;

#ifdef	__OPTIMIZE__
#define	isinf(value)	__isinf(value)
#define	isnan(value)	__isnan(value)
#define	infnan(error)	__infnan(error)
#define	finite(value)	__finite(value)
#define	copysign(x, y)	__copysign((x), (y))
#endif /* Optimizing.  */

#endif /* Use misc.  */

__END_DECLS

extern int signgam;

/* Get machine-dependent HUGE_VAL value (returned on overflow).  */
#include <huge_val.h>

/* Get machine-dependent NAN value (returned for some domain errors).  */
#ifdef   __USE_GNU
#include <nan.h>
#endif
 
#include <float.h>
#include <values.h>

#ifndef HUGE
#define HUGE		DBL_MAX
#endif

#ifndef HUGE_VAL
#define HUGE_VAL	DBL_MAX
#endif

#ifndef M_E
#define M_E         2.7182818284590452354
#endif
#ifndef M_LOG2E
#define M_LOG2E     1.4426950408889634074
#endif
#ifndef M_LOG10E
#define M_LOG10E    0.43429448190325182765
#endif
#ifndef M_LN2
#define M_LN2       0.69314718055994530942
#endif
#ifndef M_LN10
#define M_LN10      2.30258509299404568402
#endif
#ifndef M_PI
#define M_PI        3.14159265358979323846
#endif
#ifndef M_PI_2
#define M_PI_2      1.57079632679489661923
#endif
#ifndef M_1_PI
#define M_1_PI      0.31830988618379067154
#endif
#ifndef M_PI_4
#define M_PI_4      0.78539816339744830962
#endif
#ifndef M_2_PI
#define M_2_PI      0.63661977236758134308
#endif
#ifndef M_2_SQRTPI
#define M_2_SQRTPI  1.12837916709551257390
#endif
#ifndef M_SQRT2
#define M_SQRT2     1.41421356237309504880
#endif
#ifndef M_SQRT1_2
#define M_SQRT1_2   0.70710678118654752440
#endif

#ifndef PI                      /* as in stroustrup */
#define PI  M_PI
#endif
#ifndef PI2
#define PI2  M_PI_2
#endif

#if defined(__mc68000__)
# include <m68k/__math.h>
#endif

#endif /* _MATH_H */

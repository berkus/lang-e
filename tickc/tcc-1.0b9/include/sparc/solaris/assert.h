#ifndef __ASSERT
#define __ASSERT

#if !(defined(__TCC__) && defined(__C2C__))
void assert(int);
#endif

#endif /* __ASSERT */

#undef assert
#ifdef NDEBUG
#define assert(ignore) ((void)0)
#else
extern void __assert(char *, char *, unsigned);
#define assert(e) ((void)((e)||(__assert(#e, __FILE__, __LINE__),0)))
#endif /* NDEBUG */

/* much code shamelessy stolen from ...gcc/pmax/2.7.2/include/va*.h */

#ifndef __LCC_VA_LIST
#define __LCC_VA_LIST

#ifndef __GNUC_VA_LIST
#define __GNUC_VA_LIST
typedef char * __gnuc_va_list;
#endif

/* Amount of space required in an argument list for an arg of type TYPE.
   TYPE may alternatively be an expression whose type is used.  */

#ifndef _VA_LIST
#define _VA_LIST
typedef __gnuc_va_list va_list;
#endif

#define __va_rounded_size(TYPE)  \
  (((sizeof (TYPE) + sizeof (int) - 1) / sizeof (int)) * sizeof (int))

#if 0
#define va_start(AP, LASTARG) 						\
 (AP = ((__gnuc_va_list) __builtin_next_arg ()))
#endif
#define va_start(list, parmN) (list = ((char *)&parmN + sizeof(parmN)))

#if 0
#define va_arg(AP, TYPE)						\
 (AP = (__gnuc_va_list) ((char *) (AP) + __va_rounded_size (TYPE)),	\
  *((TYPE *) (void *) ((char *) (AP) - __va_rounded_size (TYPE))))
#endif
#if defined (__MIPSEL__)
/* This is for little-endian machines; small args are padded upward.  */
#define va_arg(AP, TYPE)						\
 (AP = (__gnuc_va_list) ((char *) (AP) + __va_rounded_size (TYPE)),	\
  *((TYPE *) (void *) ((char *) (AP) - __va_rounded_size (TYPE))))
#else /* big-endian */
/* This is for big-endian machines; small args are padded downward.  */
#define va_arg(AP, TYPE)						\
 (AP = (__gnuc_va_list) ((char *) (AP) + __va_rounded_size (TYPE)),	\
  *((TYPE *) (void *) ((char *) (AP) - ((sizeof (TYPE) < 4		\
					 ? sizeof (TYPE)		\
					 : __va_rounded_size (TYPE))))))
#endif /* big-endian */

#define va_end(AP)

#endif /* __LCC_VA_LIST */

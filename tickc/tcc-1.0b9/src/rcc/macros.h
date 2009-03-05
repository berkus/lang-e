#include "tickc-rts.h"

#if (defined(__TCC__) && defined(__CC2__))
				/* If we're compiling with cc2 (the second
				   phase of `C-to-C), remove obnoxious GNU
				   assert and replace it with our own so
				   that it doesn't conflict with what may have
				   been included in phase 1 (__C2C__). */
#undef assert
#undef _assert
#undef __ASSERT
#include "../assert.h"

#undef _STDARG_H
#undef _ANSI_STDARG_H_
#undef __GNUC_VA_LIST
#undef __need___va_list

#endif /* __TCC__ && __CC2__ */

#define comment(msg)
#define __DRFP__
#define __DRLP__
#define __DRGP__
#define __RFFP__
#define __RFLP__

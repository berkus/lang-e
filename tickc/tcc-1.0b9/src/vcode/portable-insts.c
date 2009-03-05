#include "vcode-internal.h"

/***************************************************************
 * Loads with a guarenteed alignment.  Fairly simplistic at the moment.
 *
 */

/* Choose the most restrictive alignment. */
static inline int alignm(int x, int y) {
        if(!x)
                return y;
        if(!y)
                return x;
        return x < y ? x : y;
}

#ifndef v_alduci
	/* Fairly simplistic at the moment. */
	void v_alduci(v_reg_t rd, v_reg_t rs, long offset, long align) {
        	v_lduci(rd, rs, offset); 
	} 
#endif

#ifndef v_alduci
	/* Fairly simplistic at the moment. */
	void v_aldci(v_reg_t rd, v_reg_t rs, long offset, long align) {
        	v_ldci(rd, rs, offset); 
	} 
#endif

#ifndef v_aldusi
	void v_aldusi(v_reg_t rd, v_reg_t rs, long offset, long align) {
       		switch(alignm(align % 2,offset % 2)) {
        	case 0: v_ldusi(rd, rs, offset); break;
        	case 1: v_uldusi(rd, rs, offset); break;
        	}
	} 
#endif

#ifndef v_aldsi
	/* Fairly simplistic at the moment. */
	void v_aldsi(v_reg_t rd, v_reg_t rs, long offset, long align) {
       		switch(alignm(align % 2, offset % 2)) {
        	case 0: v_ldsi(rd, rs, offset); break;
        	case 1: v_uldsi(rd, rs, offset); break;
        	}
	} 
#endif

#ifndef v_aldii
	/* Fairly simplistic at the moment. */
	void v_aldii(v_reg_t rd, v_reg_t rs, long offset, long align) {
       		switch(alignm(align % 4, offset % 4)) {
        	case 0: v_ldii(rd, rs, offset); break;
        	case 1:
        	case 2:
        	case 3: v_uldii(rd, rs, offset); break;
        	}
	} 
#endif

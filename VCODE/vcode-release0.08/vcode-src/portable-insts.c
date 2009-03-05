#include "vcode-internal.h"

/* Unaligned loads */

#ifndef v_uldusi
	void v_uldusi(v_reg_t rd, v_reg_t rs, long offset) {
		v_reg_t tmp;

		if(!v_getreg(&tmp, V_I, V_TEMP))
			v_fatal("uldusi: out of registers\n");

		v_lduci(tmp, rs, offset+1); /* endien dependent? */
		v_lduci(rd, rs, offset);
		v_lshui(rd, rd, 8); 
		v_oru(rd, tmp, rd); 

		v_putreg(tmp, V_I);
	}
#endif

#ifndef v_uldii
	/* have to sign-ext.  following just works on 32-bit machines. */
	void v_uldii(v_reg_t rd, v_reg_t rs, long offset) {
		v_reg_t tmp1, tmp2;
	
		if (!v_getreg(&tmp1, V_I, V_TEMP) 
		|| !v_getreg(&tmp2, V_I, V_TEMP))
			v_fatal("uldii: out of registers\n");

		v_uldusi(tmp1, rs, offset);
		v_uldusi(tmp2, rs, offset+2);

		v_lshui(tmp1, tmp1, 16);

		v_ori(rd, tmp2, tmp1);

		v_putreg(tmp1, V_I);
		v_putreg(tmp2, V_I);
	}
#endif

#ifndef v_uldsi
	void v_uldsi(v_reg_t rd, v_reg_t rs, long offset) {
		v_reg_t tmp;
	
		if(!v_getreg(&tmp, V_I, V_TEMP))
			v_fatal("uldsi: out of registers\n");

		v_lduci(tmp, rs, offset); /* endien dependent? */
		v_lduci(rd, rs, offset+1);
		/* sign extend */
		v_lshui(tmp, tmp, 24);
		v_rshii(tmp, tmp, 16); 

		v_ori(rd, tmp, rd); 

		v_putreg(tmp, V_I);
	}
#endif


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

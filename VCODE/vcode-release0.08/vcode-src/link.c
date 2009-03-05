#include "vcode-internal.h"
#include "demand.h"

#define MAXLABELS       1024
#define FLAG    ((void *)0x0)
#define MAXLINKS        1024

enum { V_JUMP = 14, V_DATA, V_LOAD, V_BRANCH };

/* list of instructions and the labels they point to. */
static struct {
        v_code *addr;
        unsigned short label;
	unsigned short type;
	unsigned short rd;
}
        link_table[MAXLINKS],
        *ltp = &link_table[0];

/* list of label locations */
static struct {
        v_code *addr;
} labels[MAXLABELS];

static void mark(void *addr, v_label_type l, int type, int rd) {
	ltp->addr = addr;
	ltp->label = _vlr(l);
	ltp->type = type;
	ltp->rd = rd;
	ltp++;
        demand(ltp < &link_table[MAXLINKS], too many links);
}

void v_link_reset(void) {
        ltp = &link_table[0];
}

/* setup pointer to branch that depends on a given label */
void v_bmark(void *addr, v_label_type l) {
	mark(addr, l, V_BRANCH, 0);
}

/* setup pointer to jump that depends on a given label */
void v_jmark(void *addr, v_label_type l) {
	mark(addr, l, V_JUMP, 0);
}

/* a data label */
void v_dmark(void *addr, v_label_type l) {
	mark(addr, l, V_DATA, 0);
}

void v_smark(void *addr, v_label_t l, v_reg_t rd) {
	mark(addr, l, V_LOAD, _vrr(rd));
}

/* mark where label is */
void v_lmark(void *addr, v_label_type l) {
	/* printf("label %d @ %x\n",_vlr(l),addr); */
        labels[_vlr(l)].addr = addr;
}

#ifdef MIPS
/* go through and link the current guy */
void v_link(void) {
	extern v_label_type v_epilogue_l;
	extern int v_restore_p(void);
 	int restore_p = v_restore_p();
	v_code *last_ip = v_ip - 1;

	/* Traverse backward to handle case where we nuke last jump. */
        for(ltp--; ltp >= &link_table[0]; ltp--) {
		switch(ltp->type) {
		case V_JUMP:
		case V_BRANCH: {
                	unsigned    	*dst, *src;
			/* relative offset between label and delay slot */
                	int disp;

                        src = ltp->addr;
                	dst = labels[ltp->label].addr;

			if(ltp->label == _vlr(v_epilogue_l)) {
				/* if this is the last jump, delete it.  */
				if(!v_incremental && ltp->addr == last_ip) {
					/* modify epilogue beginning.  */
                			labels[ltp->label].addr--;
					v_ip--;
					break;
				/* reorder for better code. */
				} else {
					/* Flip this and last statement */
					unsigned tmp = *src;
					src[0] = src[-1];
					src[-1] = tmp;
					ltp->addr--;

					/* 
					 * function doesn't restore registers, 
					 * insert return instead of jump.
					 */
					if(!restore_p) {
						*ltp->addr = 0x3e00008;
						break;
					}
				}
			}
                	demand(dst != FLAG, not linked);

 			disp = (((int)dst  - (int)src) - 4) / 4;
			if(ltp->type == V_JUMP) {
				v_code *tmp;

				tmp = v_swapcp(ltp->addr);
				j(dst);
				v_swapcp(tmp);
			} else if(disp == (short)disp)
                		*ltp->addr |= (unsigned short)disp;
			/* need to jump to end of code and then do branch. */
			else {
				v_code inst = *ltp->addr, *tmp;
				tmp = v_ip;
				
				/* assume j() can reach... */
				tmp = v_swapcp(ltp->addr);
				j(tmp);
				demand(ltp->addr + 1 == v_ip,
				      must only use a single inst);
				v_swapcp(tmp);
				/* now put the branch here. */
				*v_ip = inst; 
				/* jump over fallthrough */
				*v_ip++ |= 3; 
				nop();
					/* false: jump to fallthrough */
					j(ltp->addr + 2);
					nop();
					/* true: jump to branch dst */
					j(dst);
					nop();
			}
			break;
		} 
		case V_DATA:
			*ltp->addr = (unsigned)labels[ltp->label].addr;
			break;
		case V_LOAD: {
			v_code *tmp;
			v_reg_t rd;

			rd = v_reg(ltp->rd);
			tmp = v_swapcp(ltp->addr);
			v_setp(rd, labels[ltp->label].addr);
			v_swapcp(tmp);
			break;
		}
		default: demand(0, bogus type);
		}
        }
	v_link_reset();
}
#else

/* 
 * ALPHA/SPARC specific linking.
 * For SPARC will eventually move instructions in delay slots, etc.
 * Right now, everything is unified.
 */

#include "binary.h"

/* go through and link the current guy */
void v_link(void) {
#ifdef ALPHA
	extern int v_restore_p(void);
	int restore_p = v_restore_p();
	v_code *last_ip = v_ip - 1;
#endif

	/* NOTE: it is important that we traverse backwards.  Need to 
	   subtract one off of the epilogue label if we delete the jump. */
        for(ltp--; ltp >= &link_table[0]; ltp--) {
		switch(ltp->type) {
		case V_BRANCH: {
                	unsigned    	*dst = labels[ltp->label].addr,
                        		*src = ltp->addr;
			/* relative offset between label and delay slot */
                	long disp;
                	if(dst == FLAG)
				v_fatal("label %d has not been positioned.\n", ltp->label);

 			disp = (((long)dst  - (long)src)) / 4;
#ifdef SPARC
			/* get lower 22 bits */
#			define LOWER22(x) ((~0 >> 11) & (x)) 
			disp = LOWER22((unsigned)SIGN_EXT22(disp));
#endif

#ifdef ALPHA
			/* 
			 * If this is the last jump delete it. Additionally, we 
			 * special case functions which do not have to restore 
			 * any registers, and inline the return.
			 */
			if(!v_incremental && ltp->label == _vlr(v_epilogue_l)) {
				/* is the last jump and has to restore registers. */
				if(ltp->addr == last_ip) {
					v_ip--;
					/* adjust the label! */
					labels[ltp->label].addr --;
					break;
				/* 
			 	 * Else if do not have to restore registers, inline
				 * return.
			 	 */
				} 
				else if(!restore_p) {
					/* Return instruction */
					*ltp->addr = 0x6bfa8001;
					continue;
				}
			}
			/* need to subtract one off. */
			disp--;
			/* get lower 21 bits */
#			define LOWER21(x) (((unsigned)~0 >> 12) & (x))	
			disp = disp & 0x1fffffUL;
			/* LOWER21((unsigned)SIGN_EXT21(disp)); */
#endif

                	*ltp->addr |= disp;
			break;
		} 
		case V_DATA:
			*(unsigned long *)ltp->addr = (unsigned long)labels[ltp->label].addr;
			/* printf("linked label %d to %x\n",ltp->label, ltp->addr); */
			break;
		case V_LOAD: {
			v_code *tmp;
			v_reg_t rd;

			rd = v_reg(ltp->rd);
			tmp = v_swapcp(ltp->addr);
			v_setp(rd, labels[ltp->label].addr);
			demand((v_ip - ltp->addr) <= 5, overflow!);
			v_swapcp(tmp);
			break;
		}
		default: demand(0, bogus type);
		}
        }
	v_link_reset();
}
#endif

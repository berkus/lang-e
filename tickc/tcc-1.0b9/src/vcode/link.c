#include "vcode.h"
#include "vcode-internal.h"
#include "demand.h"

#define MAXLABELS       16384
#define FLAG    ((void *)0x0)
#define MAXLINKS        16384

enum { V_JUMP = 14, V_DATA };

/* list of instructions and the labels they point to. */
static struct {
        v_code *addr;
        unsigned short label;
	unsigned short type;
}
        link_table[MAXLINKS],
        *ltp = &link_table[0];

/* list of label locations */
static struct {
        v_code *addr;
} labels[MAXLABELS];

#ifndef __no_bpo__
#define BBSIZE 14336		/* Maximum size of a basic block, in bytes */
#define OPTBUF 2048		/* Space available to bpo for optimizations
				   that grow the code size*/

static v_code bpobuf[BBSIZE+OPTBUF];
static v_code *bpolo = bpobuf, *bpohi = bpobuf+OPTBUF+BBSIZE;
static v_code *bpohead = bpobuf+OPTBUF;

static v_code *v_ipx;		/* Real code pointer (v_ip is in bpobuf) */
v_code *v_codeend;		/* End of space for code provided by user */

int v_bpop;			/* Flag to turn on peephole optimizations */
void v_bpo_on  (void) { v_bpop = 1; }
void v_bpo_off (void) { v_bpop = 0; }

static int v_bpo_initp;		/* BPO startup/end code */
void v_bpo_init (void) {
     v_ipx = v_ip;
     v_ip = bpohead;
     v_bpo_initp = 1;
}
void v_bpo_end (void) {
     v_bpo_initp = 0;
}

inline v_code *v_bpo (v_code *addr) {
     assert(v_bpo_initp && addr == v_ip);
     if (v_ip > bpohead) {
	  demand(v_ip < bpohi, insufficient bpo buffer size);
				/* Optimize code in [bpohead,v_ip]; put the
				   output at v_ipx; bump v_ipx to code end */
	  v_ipx = bpo(bpohead, v_ip, bpolo, 
		      v_ipx, v_ipx, v_codeend, 
		      0, 0);
	  v_ip = bpohead;	/* Reset v_ip to head of optimization buffer */
     }
     return v_ipx;
}
#endif

#define linkdbg(x) /*printf x*/

static void mark(void *addr, v_label_type l, int type) {
	ltp->addr = addr;
	ltp->label = _vlr(l);
	ltp->type = type;
	ltp++;
        demand(ltp < &link_table[MAXLINKS], too many links);
}

void v_link_reset(void) {
        ltp = &link_table[0];
}

/* setup pointer to jump that depends on a given label */
/* addr should follow the immediately preceding insn: i.e. emit the
   jump prior to calling jmark */
void v_jmark(void *addr, v_label_type l) {
#ifndef __no_bpo__
        if (v_bpop)
	     addr = (void *)v_bpo((v_code *)addr);
#endif
	mark((void *)((v_code *)addr-1), l, V_JUMP);
}

/* a data label */
void v_dmark(void *addr, v_label_type l) {
#ifndef __no_bpo__
        if (v_bpop)
	     addr = (void *)v_bpo((v_code *)addr);
#endif     
	mark(addr, l, V_DATA);
}

/* mark where label is */
void v_lmark(void *addr, v_label_type l) {
#ifndef __no_bpo__
        if (v_bpop)
	     addr = (void *)v_bpo((v_code *)addr);
#endif     
        linkdbg(("label %d @ %p\n",_vlr(l),addr));
        labels[_vlr(l)].addr = addr;
}

#if defined(__SIMPLE_SCALAR__)
void v_link(void) {
     extern v_label_type v_epilogue_l;
     v_code *last_ip = v_ip-1;

     linkdbg(("last ip is %p\n",last_ip));
     for(ltp--; ltp >= &link_table[0]; ltp--) {
	  switch(ltp->type) {
	  case V_JUMP: {
	       v_code *dst = labels[ltp->label].addr,
		      *src = ltp->addr;
	       int disp;	/* relative offset of label and delay slot */
	       demand(dst != FLAG, not linked);

	       linkdbg(("jump: dst=%p src=%p\n", dst, src));
	       if(ltp->label == _vlr(v_epilogue_l)) {
		    linkdbg(("jump to epilogue: dst=%p src=%p last=%p\n",\
			     dst, src, last_ip));

		    /* if this is the last jump, delete it.  */
		    if(ltp->addr == last_ip) {
			 v_ip--;
			 v_nop(); /* Can't change position of epilogue */
			 break;
		    }
	       }
	       disp = (((int)dst  - (int)src) - 8) / 4;
	       (*ltp->addr).h |= IMM((unsigned short)disp);
	       break;
	  } 
	  case V_DATA:
	       linkdbg(("v_data: dst=%p src=%p\n", labels[ltp->label].addr,\
		      ltp->addr));
	       *(unsigned *)ltp->addr = (unsigned)labels[ltp->label].addr;
	       break;
	  default: demand(0, bogus type);
	  }
     }
     v_link_reset();
}
#elif defined(MIPS)
/* go through and link the current guy */
void v_link(void) {
	extern v_label_type v_epilogue_l;
	extern int v_restore_p(void);
 	int restore_p = v_restore_p();
	v_code *last_ip = v_ip - 1;

        for(ltp--; ltp >= &link_table[0]; ltp--) {
		switch(ltp->type) {
		case V_JUMP: {
                	unsigned    	*dst = labels[ltp->label].addr,
                        		*src = ltp->addr;
			/* relative offset between label and delay slot */
                	int disp;
                	demand(dst != FLAG, not linked);

			if(ltp->label == _vlr(v_epilogue_l)) {
				/* if this is the last jump, delete it.  */
				if(ltp->addr == last_ip) {
#if 0
                			labels[ltp->label].addr--; /* xxx */
#endif
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
 			disp = (((int)dst  - (int)src) - 4) / 4;
                	*ltp->addr |= (unsigned short)disp;
			break;
		} 
		case V_DATA:
			*ltp->addr = (unsigned)labels[ltp->label].addr;
			/* printf("linked label %d to %x\n",ltp->label, ltp->addr); */
			break;
		default: demand(0, bogus type);
		}
        }
	v_link_reset();
}
#elif defined(ALPHA)

/* 
 * ALPHA-specific linking.
 */

#include "binary.h"

/* go through and link the current guy */
void v_link(void) {
	extern int v_restore_p(void);
	int restore_p = v_restore_p();
	v_code *last_ip = v_ip - 1;

	/* NOTE: it is important that we traverse backwards.  Need to 
	   subtract one off of the epilogue label if we delete the jump. */
        for(ltp--; ltp >= &link_table[0]; ltp--) {
		switch(ltp->type) {
		case V_JUMP: {
                	unsigned    	*dst = labels[ltp->label].addr,
                        		*src = ltp->addr;
			/* relative offset between label and delay slot */
                	long disp;
                	if(dst == FLAG)
				v_fatal("label %d has not been positioned.\n", ltp->label);

 			disp = (((long)dst  - (long)src)) / 4;
			/* 
			 * If this is the last jump delete it. Additionally, we 
			 * special case functions which do not have to restore 
			 * any registers, and inline the return.
			 */
			if(ltp->label == _vlr(v_epilogue_l)) {
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
                	*ltp->addr |= disp;
			break;
		} 
		case V_DATA:
			*(unsigned long *)ltp->addr = (unsigned long)labels[ltp->label].addr;
			/* printf("linked label %d to %x\n",ltp->label, ltp->addr); */
			break;
		default: demand(0, bogus type);
		}
        }
	v_link_reset();
}

#elif defined(SPARC)

#define _op(i)  (((i)>>30)&0x3)
#define _op2(i) (((i)>>22)&0x7)
#define _op3(i) (((i)>>19)&0x3f)
#define _is_nop(i) ((i)==0x01000000)
#define _is_call(i) (_op(i)==1)
#define _is_jmpl(i) (_op(i)==2 && _op3(i)==0x38)
#define _is_rett(i) (_op(i)==2 && _op3(i)==0x39)
#define _is_trap(i) (_op(i)==2 && _op3(i)==0x3a)
#define _is_jump(i) (_op(i)==0 && (_op2(i)==2 || _op2(i)==6 || _op2(i)==7))
#define _can_delay(i) (!(_is_call(i) || _is_jmpl(i) || _is_rett(i) \
			 || _is_trap(i) || _is_jump(i)))

#include "binary.h"
void v_link(void) {
	/* NOTE: it is important that we traverse backwards.  Need to 
	   subtract one off of the epilogue label if we delete the jump. */
        for(ltp--; ltp >= &link_table[0]; ltp--) {
		switch(ltp->type) {
		case V_JUMP: {
                	unsigned    	*dst = labels[ltp->label].addr,
                        		*src = ltp->addr;
			unsigned _dst = *dst;
			unsigned _src1 = *(src+1);
			/* relative offset between label and delay slot */
                	long disp;
                	if(dst == FLAG)
				v_fatal("label %d has not been positioned.\n",
					ltp->label);
			if (_dst && _is_nop(_src1) && _can_delay(_dst)) {
			     *(src+1) = _dst;
			     ++dst;
			}

 			disp = (((long)dst  - (long)src)) / 4;
			/* get lower 22 bits */
#			define LOWER22(x) ((~0 >> 11) & (x)) 
			disp = LOWER22((unsigned)SIGN_EXT22(disp));
                	*ltp->addr |= disp;
			break;
		} 
		case V_DATA:
			*(unsigned long *)ltp->addr = (unsigned long)labels[ltp->label].addr;
			break;
		default: demand(0, bogus type);
		}
        }
	v_link_reset();
}

#endif

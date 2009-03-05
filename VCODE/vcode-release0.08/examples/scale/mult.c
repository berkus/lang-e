#include "vcode.h"
#include "mult.h"
#include "demand.h"

/* should parameterize sub, shift, etc */
static void bmul(v_reg_type rd, v_reg_type rs, v_reg_type t1, unsigned x) ;

#define isequal(r, s) (r.reg == s.reg)

/* 
 * There are two problems here, both resulting from rd and rs conflicts:
 * 	1. rr is used to store the initial value of rs
 *	2. rd == rs, then conflict.
 */
static void mgen(v_reg_type rd, v_reg_type rs, v_reg_type t, unsigned x) {
	char *p = mlookup[x];
	v_reg_type rr, t2, lastr;
	int equal;

	demand(x, bogus value);
	demand(x < LSIZE, constant size is too large: call booth-gen with larger value);

	p++; /* skip over cost */

	lastr = rd;
	if((equal = isequal(rd,rs))) {
		if(v_getreg(&rd, V_I, V_TEMP) < 0)
			v_fatal("mgen: out of registers\n");
		t2 = rd;
	}
	rr = rs;
	while(1) {
		switch(*p++) {
		case NEGATE: 	
			if(*p == DONE) rd = lastr;
			v_negi(rd, rr); 
			break;
		case SHIFT_ADD: 
			v_lshii(t, rr, *p++);	
			if(*p == DONE) rd = lastr;
			v_addi(rd, t, rs);
			break;
		case SHIFT_SUB: 
			v_lshii(t, rr, *p++);	
			if(*p == DONE) rd = lastr;
			v_subi(rd, t, rs);
			break;
		case SHIFT_REV: 
			v_lshii(t, rr, *p++);	
			if(*p == DONE) rd = lastr;
			v_subi(rd, rs, t);
			break;
		case FACTOR_ADD: 
			v_lshii(t, rr, *p++);	
			if(*p == DONE) 
				v_addi(lastr, t, rd);
			else
				v_addi(rd, t, rd);
			break;
		case FACTOR_SUB: 
			v_lshii(t, rr, *p++);	
			if(*p == DONE) 
				v_subi(lastr, t, rd);
			else
				v_subi(rd, t, rd);
			break;
		case FACTOR_REV: 
			v_lshii(t, rr, *p++);	
			if(*p == DONE) 
				v_subi(lastr, rd, t);
			else
				v_subi(rd, rd, t);
			break;
		case SHIFT:
			if(p[1] == DONE) rd = lastr;
			v_lshii(rd, rr, *p++);
			break;
		case DONE:
			return;
		default:
			demand(0, bogus op);
		}
		rr = rd;	
	}
	if(equal) 
		v_putreg(t2, V_I);
}

#if 0
static void emit_multiply(v_reg_type rd, v_reg_type rs, unsigned x) {
	int cost, seq[10];

	/* compute sequence of multiplications */

	rem = x;
	while(1) {
		if(rem <= LSIZE) {
			*s++ = rem;
			break;
		} else {
			/* 
			 * If the number is prime, we have to 
			 * fallback on bmul or something 
			 */

			/* Should check for multiple of two or 2^n+/-1 */

			/* power of two? */
			if((rem & -rem) == rem) {
				cost += 1;
				*s++ = rem;
				break;
			}
			/* is the number prime? */
			if((p = rem % LSIZE) == LSIZE - 1)
				cost += estimate_cost(p);
				*s++ = p;
				break;
			/* exact division */
			else if(!p)
				*s++ = LSIZE;
			else
				*s++ = mlookup[LSIZE - p];
				rem = rem / LSIZE + 1;
		}
	}
#	define MULT_COST 15

	if(cost > MULT_COST) {
		li(t, x);
		v_multi(rd, rs, t);
		return;
	}
		
	/* need to accumulate these multiplications */
	for(s = seq[0]; *s; s++) {
		unsigned p = *s;
		if(p < LSIZE)
			mgen(rd, rs, t1, p);	
		else if(p == (p & -p))
			v_sllu(rd, 
	
			
	}
}

#endif

void cmuli(v_reg_type rd, v_reg_type rs, int x) {
	v_reg_type t1;

	if(!x) {
		v_seti(rd, 0);
		return;
	} else if(x == 1) {
		if(isequal(rd, rs))
			return;
		v_movi(rd, rs);
		return;
	}
	v_getreg(&t1, V_I, V_TEMP);
	if(x >= 0) {
		mgen(rd, rs, t1, x);
	} else {
		mgen(rd, rs, t1, -x);
		v_negi(rd, rd);
	}
	v_putreg(t1, V_I);
}

static void bmul(v_reg_type rd, v_reg_type rs, v_reg_type t1, unsigned x) {
        int i, virgin;

        for(i=0,virgin=1; x ; x >>= 1, i++)  {

                while((x&1) == 0) { i++; x>>=1; }
			
                if(x & 1) {
                        unsigned count = 0, t = i;
                        /* count run */
                        do { count++; x >>= 1; } while(x & 1);
                        i+=count;
                        if(count < 3) {
                                if(virgin) {
                                        v_lshii(rd,rs,(i-count));
                                        virgin = 0;
                                        count--;
                                }
                                while(count) {
                                        v_lshii(t1,rs,(i-count));
                                        v_addi(rd,rd,t1);
                                        count--;
                                }
                        } else {
                                if(!virgin) {
                                        v_lshii(t1,rs, (i));
                                        v_addi(rd,rd,t1);
                                        v_lshii(t1,rs, (t));
                                        v_subi(rd,rd,t1);
                                } else {
                                       virgin = 0;
                                        v_lshii(rd,rs,(i));
                                        if(!t)
                                                v_subi(rd,rd,rs);
                                        else {
                                                v_lshii(t1,rs,(t));
                                                v_subi(rd,rd,t1);
                                        }
                                }
                        }
                }
        }
}

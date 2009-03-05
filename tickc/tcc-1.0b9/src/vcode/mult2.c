#include "vcode.h"
#include "mult.h"
#include "demand.h"


#ifndef __fast__
#define isequal(r, s) ((r).reg == (s).reg)
#else
#define isequal(r, s) ((r) == (s))
#endif

/* 
 * There are two problems here, both resulting from rd and rs conflicts:
 * 	1. rr is used to store the initial value of rs
 *	2. rd == rs, then conflict.
 */
#define mkmgen(ty, TY, type)						   \
static void mgen ## ty (v_reg_type rd, v_reg_type rs, 			   \
			v_reg_type t, type x) { 			   \
	char *p = mlookup[x];						   \
	v_reg_type rr, t2 = 0, lastr;					   \
	int equal;							   \
									   \
	demand(x, bogus value);						   \
	if (x >= LSIZE) {						   \
	     v_mul ## ty ## i(rd, rs, x);				   \
	     return;							   \
	}								   \
									   \
	p++; /* skip over cost */					   \
									   \
	lastr = rd;							   \
	if((equal = isequal(rd,rs))) {					   \
		if(v_getreg(&rd, V_ ## TY , V_TEMP) < 0)		   \
			v_fatal("mgen: out of registers\n");		   \
		t2 = rd;						   \
	}								   \
	rr = rs;							   \
	while(1) {							   \
		switch(*p++) {						   \
		case NEGATE: 						   \
			if(*p == DONE) rd = lastr;			   \
			v_neg ## ty (rd, rr); 				   \
			break;						   \
		case SHIFT_ADD: 					   \
			v_lsh ## ty ## i(t, rr, *p++);			   \
			if(*p == DONE) rd = lastr;			   \
			v_add ## ty (rd, t, rs);			   \
			break;						   \
		case SHIFT_SUB: 					   \
			v_lsh ## ty ## i(t, rr, *p++);			   \
			if(*p == DONE) rd = lastr;			   \
			v_sub ## ty (rd, t, rs);			   \
			break;						   \
		case SHIFT_REV: 					   \
			v_lsh ## ty ## i(t, rr, *p++);			   \
			if(*p == DONE) rd = lastr;			   \
			v_sub ## ty (rd, rs, t);			   \
			break;						   \
		case FACTOR_ADD: 					   \
			v_lsh ## ty ## i(t, rr, *p++);			   \
			if(*p == DONE) 					   \
				v_add ## ty (lastr, t, rd);		   \
			else						   \
				v_add ## ty (rd, t, rd);		   \
			break;						   \
		case FACTOR_SUB: 					   \
			v_lsh ## ty ## i(t, rr, *p++);			   \
			if(*p == DONE) 					   \
				v_sub ## ty (lastr, t, rd);		   \
			else						   \
				v_sub ## ty (rd, t, rd);		   \
			break;						   \
		case FACTOR_REV: 					   \
			v_lsh ## ty ## i(t, rr, *p++);			   \
			if(*p == DONE) 					   \
				v_sub ## ty (lastr, rd, t);		   \
			else						   \
				v_sub ## ty (rd, rd, t);		   \
			break;						   \
		case SHIFT:						   \
			if(p[1] == DONE) rd = lastr;			   \
			v_lsh ## ty ## i(rd, rr, *p++);			   \
			break;						   \
		case DONE:						   \
			goto quit;					   \
		default:						   \
			demand(0, bogus op);				   \
		}							   \
		rr = rd;						   \
	}								   \
quit:									   \
	if(equal) 							   \
		v_putreg(t2, V_ ## TY);					   \
}

#define mkcmul(ty, TY, type)					\
void v_cmul ## ty (v_reg_type rd, v_reg_type rs, type x) {	\
	v_reg_type t1;						\
								\
	if(!x) {						\
		v_set ## ty (rd, 0);				\
		return;						\
	} else if(x == 1) {					\
		if(isequal(rd, rs))				\
			return;					\
		v_mov ## ty (rd, rs);				\
		return;						\
	}							\
	v_getreg(&t1, V_ ## TY , V_TEMP);			\
	if(x >= 0) { /* redundant test if x unsigned */		\
		mgen ## ty (rd, rs, t1, x);			\
	} else {						\
		mgen ## ty (rd, rs, t1, -x);			\
		v_neg ## ty (rd, rd);				\
	}							\
	v_putreg(t1, V_ ## TY );				\
}

mkmgen(i,I, int)
mkmgen(u,U, unsigned)
mkcmul(i,I, int)
mkcmul(u,U, unsigned)


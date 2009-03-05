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
 *	1. rr is used to store the initial value of rs
 *	2. rd == rs, then conflict.
 */
static void mgeni (v_reg_type rd, v_reg_type rs, v_reg_type t, int x) {
     char *p = mlookup[x];
     v_reg_type rr, t2 = 0, lastr;
     int equal;
     demand(x, bogus value);
     if (x >= LSIZE) {
	  v_mulii(rd, rs, x);
	  return;
     }
     p++;
     lastr = rd;
     if((equal = isequal(rd,rs))) {
	  if(v_getreg(&rd, V_I, V_TEMP) < 0) 
	       v_fatal("mgen: out of registers\n");
	  t2 = rd;
     }
     rr = rs;
     while(1) {
	  switch(*p++) {
	  case NEGATE: if(*p == DONE) rd = lastr;
	       v_negi(rd, rr);
	       break;
	  case SHIFT_ADD: v_lshii(t, rr, *p++);
	       if(*p == DONE) rd = lastr;
	       v_addi(rd, t, rs);
	       break;
	  case SHIFT_SUB: v_lshii(t, rr, *p++);
	       if(*p == DONE) rd = lastr;
	       v_subi(rd, t, rs);
	       break;
	  case SHIFT_REV: v_lshii(t, rr, *p++);
	       if(*p == DONE) rd = lastr;
	       v_subi(rd, rs, t);
	       break;
	  case FACTOR_ADD: v_lshii(t, rr, *p++);
	       if(*p == DONE) v_addi(lastr, t, rd);
	       else    v_addi(rd, t, rd);
	       break;
	  case FACTOR_SUB: 
	       v_lshii(t, rr, *p++);
	       if(*p == DONE) v_subi(lastr, t, rd);
	       else v_subi(rd, t, rd);
	       break;
	  case FACTOR_REV: 
	       v_lshii(t, rr, *p++);
	       if(*p == DONE) v_subi(lastr, rd, t);
	       else    v_subi(rd, rd, t);
	       break;
	  case SHIFT:	  if(p[1] == DONE) rd = lastr;
	       v_lshii(rd, rr, *p++);
	       break;
	  case DONE:	  goto quit;
	  default:	  demand(0, bogus op);
	  }	  
	  rr = rd;
     }	  
quit:	 
	  if(equal) 
	       v_putreg(t2, V_I );
} 

static void mgenu (v_reg_type rd, v_reg_type rs, v_reg_type t, unsigned x) {
     char *p = mlookup[x];
     v_reg_type rr, t2 = 0, lastr;
     int equal;
     demand(x, bogus value);
     if (x >= LSIZE) {
	  v_mului(rd, rs, x);
	  return;
     }	
     p++;
     lastr = rd;
     if((equal = isequal(rd,rs))) {
	  if(v_getreg(&rd, V_U, V_TEMP) < 0)
	       v_fatal("mgen: out of registers\n");
	  t2 = rd;
     }
     rr = rs;
     while(1) {
	  switch(*p++) {
	  case NEGATE: 
	       if(*p == DONE) rd = lastr;
	       v_negu(rd, rr);
	       break;
	  case SHIFT_ADD: 
	       v_lshui(t, rr, *p++);
	       if(*p == DONE) rd = lastr;
	       v_addu(rd, t, rs);
	       break;
	  case SHIFT_SUB:
	       v_lshui(t, rr, *p++);
	       if(*p == DONE) rd = lastr;
	       v_subu(rd, t, rs);
	       break;
	  case SHIFT_REV:
	       v_lshui(t, rr, *p++);
	       if(*p == DONE) rd = lastr;
	       v_subu(rd, rs, t);
	       break;
	  case FACTOR_ADD: 
	       v_lshui(t, rr, *p++);
	       if(*p == DONE) v_addu(lastr, t, rd);
	       else v_addu(rd, t, rd);
	       break;
	  case FACTOR_SUB: 
	       v_lshui(t, rr, *p++);
	       if(*p == DONE) v_subu(lastr, t, rd);
	       else v_subu(rd, t, rd);
	       break;
	  case FACTOR_REV: 
	       v_lshui(t, rr, *p++);
	       if(*p == DONE) v_subu(lastr, rd, t);
	       else v_subu(rd, rd, t);
	       break;
	  case SHIFT:
	       if(p[1] == DONE) rd = lastr;
	       v_lshui(rd, rr, *p++);
	       break;
	  case DONE:
	       goto quit;
	  default:
	       demand(0, bogus op);
	  }	    
	  rr = rd;
     }      
quit:
     if(equal) 
	  v_putreg(t2, V_U);
} 

void v_cmuli (v_reg_type rd, v_reg_type rs, int x) {
     v_reg_type t1;
     if(!x) {
	  v_seti(rd, 0);
	  return;
     } else if(x == 1) {
	  if(isequal(rd, rs))	  
	       return;
	  v_movi(rd, rs);
	  return;
     }	     v_getreg(&t1, V_I, V_TEMP);
     if(x >= 0) {
	  mgeni(rd, rs, t1, x);
     } else {
	  mgeni(rd, rs, t1, -x);
	  v_negi(rd, rd);
     }
     v_putreg(t1, V_I);
} 

void v_cmulu (v_reg_type rd, v_reg_type rs, unsigned x) {
     v_reg_type t1;
     if(!x) {
	  v_setu(rd, 0);
	  return;
     } else if(x == 1) {
	  if(isequal(rd, rs))
	       return;
	  v_movu(rd, rs);
	  return;
     }
     v_getreg(&t1, V_U, V_TEMP);
     if(x >= 0) {
	  mgenu(rd, rs, t1, x);
     } else {
	  mgenu(rd, rs, t1, -x);
	  v_negu(rd, rd);
     }	     
     v_putreg(t1, V_U);
} 

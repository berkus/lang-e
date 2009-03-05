/* Some useful polymorphic instructions. */
#include "vcode.h"

/* polymorphic set; given a type, destination and a type union, load the
* right value. */
void v_pset(int type, v_reg_type rd, union v_types tu) {
        switch(type) {
        case V_UC:      v_setu(rd, tu.uc);      break;
        case V_US:      v_setu(rd, tu.us);      break;
        case V_U:       v_setu(rd, tu.u);       break;
        case V_UL:      v_setul(rd, tu.ul);     break;
        case V_P:       v_setp(rd, tu.p);       break;
        case V_C:       v_seti(rd, tu.c);       break;
        case V_S:       v_seti(rd, tu.s);       break;
        case V_I:       v_seti(rd, tu.i);       break;
        case V_L:       v_setl(rd, tu.l);       break;
        case V_F:       v_setf(rd, tu.f);       break;
        case V_D:       v_setd(rd, tu.d);       break;
        default:        demand(0, bogus type);
        }
}

/* polymorphic move: given a type, move src to dst */
void v_pmov(int type, v_reg_type rd, v_reg_type rs) {
        switch(type) {
        case V_UC:
        case V_US:
        case V_U:       v_movu(rd, rs);         break;
        case V_UL:      v_movul(rd, rs);        break;
        case V_P:       v_movp(rd, rs);         break;
        case V_C:
        case V_S:
        case V_I:       v_movi(rd, rs);         break;
        case V_L:       v_movl(rd, rs);         break;
        case V_F:       v_movf(rd, rs);         break;
        case V_D:       v_movd(rd, rs);         break;
        default:        demand(0, bogus type);
        }
}

/* polymorphic store. */
void v_pst(int type, v_reg_type rd, v_reg_type base, int offset) {
        switch(type) {
        case V_UC:
        case V_US:
        case V_U:       v_stui(rd, base, offset);        break;
        case V_UL:      v_stuli(rd, base, offset);       break;
        case V_P:       v_stpi(rd, base, offset);        break;
        case V_C:
        case V_S:
        case V_I:       v_stii(rd, base, offset);        break;
        case V_L:       v_stli(rd, base, offset);        break;
        case V_F:       v_stfi(rd, base, offset);        break;
        case V_D:       v_stdi(rd, base, offset);        break;
        default:        demand(0, bogus type);
        }
}

/* polymorphic load. */
void v_pld(int type, v_reg_type rd, v_reg_type base, int offset) {
        switch(type) {
        case V_UC:
        case V_US:
        case V_U:       v_ldui(rd, base, offset);        break;
        case V_UL:      v_lduli(rd, base, offset);       break;
        case V_P:       v_ldpi(rd, base, offset);        break;
        case V_C:
        case V_S:
        case V_I:       v_ldii(rd, base, offset);        break;
        case V_L:       v_ldli(rd, base, offset);        break;
        case V_F:       v_ldfi(rd, base, offset);        break;
        case V_D:       v_lddi(rd, base, offset);        break;
        default:        demand(0, bogus type);
        }
}

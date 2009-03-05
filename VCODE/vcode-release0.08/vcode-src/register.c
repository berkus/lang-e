#include "vcode-internal.h"
#include <string.h>

#ifdef ALPHA
	typedef unsigned reg_t;	/* register type (lbu is expensive). */
#else
	typedef unsigned char reg_t;	/* register type. */
#endif

/* Will have to change this on machines with more than 32 regs. */
#define MAX_REGS	32
typedef unsigned uint32;

/* Information needed to describe a register class (e.g., int temporaries). */
struct v_reg_class {
	uint32	perm_members;	/* permenent members in this class. */
	/* Used to classify a register; can be added to by v_rawput. */
	uint32	members;	/* temporary members in the given class. */
	uint32	standin;	/* registers standing in for this class */
	/* cumulative mask of registers that have been allocated. */
	uint32	allocated;	
	uint32	free;		/* current set of available registers. */
	int n;			/* number of registers in this class. */
	reg_t	initial[MAX_REGS]; /* members in this class. */
	reg_t   available[MAX_REGS]; /* available registers */
	reg_t	*rp;		/* pointer to the current avail register. */
	int 	iter_state;	/* iteration variable. */
};


reg_t v_ptmp;

struct v_reg_class v_tempi; 	/* integer temporaries. */
struct v_reg_class v_vari; 	/* integer variables. */
struct v_reg_class v_unavaili; 	/* integer not allocated. */
struct v_reg_class v_tempf; 	/* float temporaries. */
struct v_reg_class v_varf; 	/* float variables. */
struct v_reg_class v_unavailf; 	/* float not allocated. */

static unsigned int_reg_type[MAX_REGS];       /* char, short, etc */
static unsigned fp_reg_type[MAX_REGS];        /* float or double */

/**********************************************************************
 * Syntactic sugar for manipulating the register data structures.
 * 
 */

#define R(x) (1 << (x))
#define BIT_OFF(mask, r) ((mask) &= ~R(r))
#define BIT_ON(mask, r) ((mask) |= R(r))
#define SET_P(mask, r) (R(r) & mask)

/* Track whether a register is free or not. */
#ifndef NDEBUG
#	define IS_FREE(class, r)	SET_P(class->free, r)
#	define SET_FREE(class, r)	BIT_ON(class->free, r)
# 	define SET_ALLOCED(class, r)	BIT_OFF(class->free, r)
#else
#	define IS_FREE(class, r)	1
#	define SET_FREE(class, r)	(void)1
# 	define SET_ALLOCED(class, r)	(void)1
#endif

#define v_pop(r) 							\
  (((v_ptmp = *(r)->rp++) == V_REG_SENTINAL) ?				\
      ((r)->rp--, -1)  :                             			\
      ( BIT_ON((r)->allocated, v_ptmp), SET_ALLOCED(r, v_ptmp), v_ptmp))

#define v_push(class, r) do {						\
	SET_FREE(class, r); 						\
	*--(class)->rp = r;						\
} while(0)

#ifndef NREGS
#       define NREGS    32
#endif

/* Used to mark the end of available registers. */
#define V_REG_SENTINAL 255
/* Indicate that an iterator is yet to be initiated. */
#define ITER_INITIAL -1


/********************************************************************
 * Internal functions.
 *
 */

#define types(t) (type_s[((t) < V_C || (t) > V_ERR) ? (V_ERR - V_C) : ((t) - V_C)])

/* 
 * This is a total hack to track whether we have initialized the register
 * structure.  It will bite us.  Eventually.
 */
static int register_init;

static void initialize(struct v_reg_class *r, uint32 mask);
static int count_bits(unsigned x);

static char *type_s[] = {
        "V_C", "V_UC", "V_S", "V_US", "V_I", "V_U",
        "V_L", "V_UL", "V_P",
        "V_F", "V_D",
        "V_B",
        "V_ERR"
};

/* Reset the register stack. */
static void reset(struct v_reg_class *r) {
	/* copy sentinal as well. */
	memcpy(&r->available[0], &r->initial[0], (r->n+1) * sizeof(reg_t));
	r->rp = &r->available[0];
	r->standin = 0;
	r->allocated = 0;
	r->free = r->members = r->perm_members;
}

/* Find-first-bit machinery; used for fast put/get reg */
static char ffb_table[257];
static void ffb_table_init() {
        int i;
        for(i=0;i<8;i++) ffb_table[(1 << i)] = i;
}

static inline unsigned long firstbitset(unsigned long b) {
#       define to8  (1 << 8)
#       define to16 (1 << 16)
#       define to24 (1 << 24)
        /* get byte 'b' from x */
#       define byte(x, b) (((x) >> ((b)*8)) & 0xff)

        b &= -b;        /* only leave a single bit on */
        if(b >= to16) {
                if(b >= to24) {
                        return ffb_table[byte(b,3)] + 24;
                } else {
                        return ffb_table[byte(b,2)] + 16;
                }
        } else {
                if(b >= to8) {
                        return ffb_table[byte(b,1)] + 8;
                } else {
                        return ffb_table[byte(b,0)];
                }
        }
}


/* reset all registers. */
void register_reset(void) {

	if(!register_init) {
		register_init = 1;
		initialize(&v_tempi, TEMPI);
		initialize(&v_vari, VARI);
		initialize(&v_unavaili, 0);
		initialize(&v_tempf, TEMPF);
		initialize(&v_varf, VARF);
		initialize(&v_unavailf, 0);
		ffb_table_init();
	}
	reset(&v_tempi);
	reset(&v_vari);
	reset(&v_tempf);
	reset(&v_varf);
}

/* Perform first time initialization of the given register class. */
static void initialize(struct v_reg_class *r, uint32 mask) {
	int i, reg;

	r->members = r->perm_members = mask;
	r->n = count_bits(r->members);

	/* setup the register vector. */
	for(reg = i = 0; i < MAX_REGS; i++)
		if(SET_P(r->members, i))
			r->initial[reg++] = i;

	demand(reg == r->n, disagreement);

	r->initial[r->n] = V_REG_SENTINAL;

	r->rp = &r->available[0];
	r->iter_state = ITER_INITIAL;
}

static int count_bits(unsigned x) {
        int i;

        for(i = 0; x; i++, x &= (x-1));
        return i;
}

static int 
ralloc(int class, struct v_reg_class *temp, struct v_reg_class *var) {
	int reg;

	reg = -1;
	if(class == V_VAR) {
#ifndef SPARC
		/* simplicity is good. */
		reg = v_pop(var);
#else

                /*
                 * Check for variable register.  If there are none left *and* 
		 * we have not emitted a call, then steal a temporary register
		 *  and mark that it must be manually saved.  We will have to 
		 * become even more restrictive when allowing composition.
 		 * 
		 * This is actually living too close to the edge for comfort.
		 * Don't like it.
                 */
		if((reg = v_pop(var)) < 0)
			if(!v_calls && (reg = v_pop(temp)) >= 0)
				BIT_ON(var->standin, reg);
#endif
	} else {
		demand(class == V_TEMP, bogus class);
                /*
                 * Check for temporary register.  If there are none left,
                 * try to steal a variable reg; if successful, mark that it
                 * must be saved on entrance and return.
		 *
		 * I would get rid of this if the SPARC didn't force it.
                 */
		if((reg = v_pop(temp)) < 0)
			if((reg = v_pop(var)) >= 0)
				/* Mark that reg is standing in. */
				BIT_ON(temp->standin, reg);
	}
	return reg;
}

static inline void
putreg(struct v_reg_class *temps, struct v_reg_class *vars, reg_t r) {
        if(SET_P(temps->members, r)) {
#		ifndef NDEBUG
                	if(SET_P(temps->free, r))
                        	v_fatal("v_putreg: r %d is already free!\n", r);
#		endif
		v_push(temps, r);
        } else {
                demand(SET_P(vars->members, r), bogus register type);
#		ifndef NDEBUG
                	if(SET_P(vars->free, r))
                        	v_fatal("v_putreg: r %d is already free!", r);
#		endif
		v_push(vars, r);
        }
}

/* Get mask of accumulated registers. */
static unsigned get_accum(int class) {
	struct v_reg_class *r;

	r = 0;
        switch(class) {
        case V_TEMPI: r = &v_tempi; break;
        case V_VARI:  r = &v_vari; break;
        case V_TEMPF: r = &v_tempf; break;
        case V_VARF:  r = &v_varf; break;
        default: 	v_fatal("v_naccum: bogus class %d\n", class);
        }
	return (r->allocated);
}

/* Remove register from pool. */
static int r_remove(struct v_reg_class *c, v_reg_type reg) {
	int r;

	r = _vrr(reg);

	if(!SET_P(c->perm_members, r))
		return 0;
	else {
		int i,n;

		/* Eliminate it from perm-members. */
		BIT_OFF(c->perm_members, r);
		/* Pull it off of the inital stack */
		n = c->n--;

		for(i = 0; i < n; i++) {
			if(c->initial[i] == r) {
				/* overwrite with last guy. */
				c->initial[i] = c->initial[c->n];
				/* Need to insert sentinal */
				c->initial[c->n] = V_REG_SENTINAL;
				return 1;
			}
		}
		demand(0, should have been in the stack!);
	}
}

/* Add a register to pool. */
static void r_add(struct v_reg_class *c, v_reg_type reg) {
	int r;

	r = _vrr(reg);
	BIT_ON(c->perm_members, r);
	c->initial[c->n++] = r;
	c->initial[c->n] = V_REG_SENTINAL;
}

/*******************************************************************
 * Exported functions.
 *
 */


/* allocate a register.  */
int v_getreg(v_reg_type* r, int type, int class) {
	int reg;

	assert_active(v_getreg);
	reg = 0;

        switch(type) {
        case V_C: case V_UC: case V_US: case V_S:
        case V_I: case V_U: case V_L: case V_UL: case V_P:
		if((reg = ralloc(class, &v_tempi, &v_vari)) >= 0)
                        int_reg_type[reg] = type;
		break;
        case V_D: case V_F:
		if((reg = ralloc(class, &v_tempf, &v_varf)) >= 0)
                        fp_reg_type[reg] = type;
		break;
        default:
                v_fatal("v_getreg: bogus type %d:%s\n", type, types(type));
        }
#	ifdef	REG_NOFAIL
        	assert(reg >= 0);
#	endif
	_vrrp(r) = reg;
        return reg >= 0;
}


/* deallocate a register. */
void v_putreg(v_reg_type r, int type) {
	assert_active(v_putreg);

        switch(type) {
        case V_C: case V_UC: case V_US: case V_S:
        case V_I: case V_U: case V_L: case V_UL: case V_P:
                putreg(&v_tempi, &v_vari, _vrr(r));
                break;
        case V_F: case V_D:
                putreg(&v_tempf, &v_varf, _vrr(r));
                break;
        default:
                v_fatal("v_putreg: bogus type %d:%s\n", type, types(type));
        }
}


/* 
 * Allocate n temporaries.  This is a dangerous function: any call
 * to getreg will allocate one of the temps.   Should provide a 
 * way to mark these.
 */
int v_get_temps(int *rv, int n, int class) {
	reg_t	*rp;
	
	assert_active(v_get_temps);
        rp = (class = V_TEMPI) ?
                v_tempi.rp : v_tempf.rp;

        while(n-- > 0)
                if((*rv++ = *rp++) == V_REG_SENTINAL)
                        return -1;
        return 0;
}

/*  
 * Used to make unused argument registers available for allocation.
 * Eliminate checks as to whether the register belongs to this
 * class. 
 */
void v_rawput(int r, int class) {
	int type;

	assert_active(v_rawput);

	type = V_ERR;
        switch(class) {
        case V_VARI:    BIT_ON(v_vari.members, r); type = V_L; break;
        case V_TEMPI:   BIT_ON(v_tempi.members, r); type = V_L; break;
        case V_VARF:    BIT_ON(v_varf.members, r);  type = V_D; break;
        case V_TEMPF:   BIT_ON(v_tempf.members, r); type = V_D; break;
        default: 	v_fatal("v_rawput: bogus class %d\n", class);
        }
	v_putreg(v_reg(r), type);
}

/* Acumulated number of registers that have been allocated in this class. */
int v_naccum(int class) {
	assert_active(v_naccum);
	return count_bits(get_accum(class));
}

/*
 * Use as an iterator to return integers for all registers
 * in a given mask.  Arguments are r: give the last register
 * or -1 to start; type: give the type.
 */
int v_reg_iter(int r, int type) {
        static int start_type;
        static unsigned mask;

	assert_active(v_reg_iter);
        if(r < 0)
                mask = get_accum(start_type = type);
        demand(type == start_type, switched types);

        if(!mask)
                return -1;
        else {
                r = firstbitset(mask);
                mask &= ~R(r);
                return r;
        }
}


/*
 * We remove the registers from their opposite set (if they are present)
 * and put them in the destination set.  Currently we do not check
 * whether they were present.  We should probably complain if they were
 * not.
 */
void v_mk_unavail(int type, v_reg_type r) {
	if(v_locked_p()) 
		v_fatal("v_chg_class: can only be called before lambda!\n");

	if(!register_init)
		register_reset();

	/* Check for register in perm_members */
        switch(type) {
        case V_C: case V_UC: case V_US:
        case V_S: case V_I: case V_U:
        case V_L: case V_UL: case V_P:
		if(!r_remove(&v_tempi, r) && !r_remove(&v_vari, r))
			v_fatal("v_mk_unavail: register is not avail.\n");
		else
			r_add(&v_unavaili, r);
		return;

        case V_F: case V_D:
		if(!r_remove(&v_tempf, r) && !r_remove(&v_varf, r))
			v_fatal("v_mk_unavail: register is not avail.\n");
		else
			r_add(&v_unavailf, r);
		return;

        default:        v_fatal("v_enum_class: bogus class %d\n", type);
        }
}

void v_mk_temp(int set, int type, unsigned mask) {
	demand(0,Need to reimplement this function);
}

void v_mk_var(int set, int type, unsigned mask) {
	demand(0,Need to reimplement this function);
}


/* Enumerate all elements in a class */
unsigned v_enum_rclass(int set, int type) {
	assert_active(v_enum_rclass);
        switch(type) {
        case V_C: case V_UC: case V_US:
        case V_S: case V_I: case V_U:
        case V_L: case V_UL: case V_P:
                switch(set) {
                case V_VAR:     return v_vari.members;
                case V_TEMP:    return v_tempi.members;
                case V_UNAVAIL: return v_unavaili.members; 
                default: 
		       v_fatal("v_enum_class: bogus register type %d\n", set);
                }
        case V_F: case V_D:
                switch(set) {
                case V_VAR:     return v_varf.members;
                case V_TEMP:    return v_tempf.members;
                case V_UNAVAIL: return v_unavailf.members; 
                default:        
			v_fatal("v_enum_class: bogus register type %d\n", set);
                }
        default:        v_fatal("v_enum_class: bogus class %d\n", type);
        }
        /*NOTREACHED*/
        return 0;
}

int v_istemp(v_reg_type r, int type) {
	int reg;

	reg = _vrr(r);
	return (v_enum_rclass(V_TEMP, type) & (1 << reg)) != 0;
}

void
v_move_regs(int class, int double_ld_p, void (*small_save)(v_reg_type),
            void (*big_save)(v_reg_type))
{
        unsigned mask, r;

        switch(class) {
                case V_STANDINF:
                        for(mask = v_varf.standin; mask; mask &= ~R(r)) {
                                r = firstbitset(mask);
                                switch(fp_reg_type[r]) {
                                case V_F: small_save(v_reg(r)); break;
                                case V_D: big_save(v_reg(r));  break;
                                default: assert(0);
                                }
                        }
                        break;
                case V_STANDINI:
                        for(mask = v_vari.standin; mask; mask &= ~R(r)) {
                                r = firstbitset(mask);
                                switch(fp_reg_type[r]) {
                                case V_C: case V_UC: case V_US: case V_S:
                                case V_I: case V_U:
                                        small_save(v_reg(r));
                                        break;
                                case V_L: case V_UL: case V_P:
                                        big_save(v_reg(r));
                                        break;
                                default: assert(0);
                                }
                        }
                        break;
                default: assert(0);
        }
}

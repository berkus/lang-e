#ifndef __VCODE_INTERNAL_H__
#define __VCODE_INTERNAL_H__
#define __VCODE_INTERNAL__ 	/* Invoke internal stuff.  */
#include "vcode.h"

#ifdef NDEBUG
#	define assert_active(proc_name)
#	define fatal(bool, msg)
#else
#	define assert_active(proc_name) do {			\
		if(!v_locked_p()) 				\
			v_fatal("(%s,%d) %s: called outside of a v_lambda/v_end pair.\n", __FILE__, __LINE__, #proc_name);	\
	} while(0)
#	define fatal(msg) do {					\
	     v_fatal("(%s,%d) %s.\n", __FILE__, __LINE__, #msg);\
	} while(0)
#endif

/* 
 * Does the machine support double word moves between memory and the register
 * bank?
 */
enum { V_NO_DOUBLE_LD , V_DOUBLE_LD };

/* The different register classes */
enum { V_TEMPI = 1, V_VARI, V_TEMPF, V_VARF, V_STANDINI, V_STANDINF };

/*
 * Gives the type of the argument and whether it is in a register
 * or is an immediate.
 */
struct v_arg {
        unsigned short type;
        unsigned short isreg_p;
        union v_types u;        /* union of all possible types */
};


extern int v_pseudo_call;
extern int v_nbytes;
#ifndef __no_bpo__
extern v_code *v_codeend;	/* End of space available for code */
extern int v_bpop;		/* Flag to turn on peephole optimizations */
extern void v_bpo_init (void);	/* Initialize bpo state */
extern void v_bpo_end (void);
extern v_code *v_bpo (v_code *addr); /* Binary peephole opt driver */
typedef v_code* bpoW;
typedef bpoW (*nft)(bpoW);	/* Type of buffer pointer increment function */
extern bpoW bpo (bpoW ih, bpoW it, bpoW il, 
		 bpoW oh, bpoW ot, bpoW ol, 
		 nft nf, int dp);
#endif
extern v_code   *v_ip;  /* pointer to current instruction stream */

/* Flush the cache */
void v_flushcache(void *ptr, int size);

/* Function management. */
int v_unlock(void);
void v_begin(v_code *i);
void v_epilogue(void);

int v_naccum(int type);
void v_rawput(int r, int type);
int v_reg_iter(int r, int type);
void v_move_regs(int class, int double_ld_p, void (*small_save)(v_reg_type),
            void (*big_save)(v_reg_type));

/* label and jump routines */
void v_link_reset(void);
void v_jmark(void *addr, v_label_type l);
void v_dmark(void *addr, v_label_type l);
void v_lmark(void *addr, v_label_type l);

void v_pset(int type, v_reg_type rd, union v_types tu);
void v_pmov(int type, v_reg_type rd, v_reg_type rs);
void v_pst(int type, v_reg_type rd, v_reg_type base, int offset);
void v_pld(int type, v_reg_type rd, v_reg_type base, int offset);


/* machine specific-lambda. */
void v_mach_lambda(int nargs, int *arglist, v_reg_type *args, int leaf, v_code *ip);
/* machine-specific end */
union v_fp v_mach_end(int *nbytes);

void v_link(void);

/* Translate format string to internal type array */
int *v_xlatel(char *fmt, int *nargs);

/* defined by machine dependent backend */
void v_mach_call(struct v_cstate *c, void (*ptr)());
void v_rmach_call(struct v_cstate *c, v_reg_type r);

extern const v_reg_type
                v_zero,      /* register always returns zero */
                v_sp,      /* stack pointer */
                v_at,      /* used by vcode to load constants, etc.  */
                v_dat,      /* used by vcode for synthetic fp ops.  */
                v_ra,      /* return register */
                v_caller_int_rr, /* return register (what caller sees) */
                v_callee_int_rr, /* return register (what callee sees) */
                v_fp_rr,     /* fp return register */
                v_fp,      /* frame-pointer */
                v_lp;      /* register to give for local references */


/* Max number of instructions required to load a single-precision immediate. */
extern const unsigned v_float_imm_insns; 
/* Max number of instructions required to load a double-precision immediate. */
extern const unsigned v_double_imm_insns;

/* round x upto alignment n */
#define v_roundup(x,n) ((((unsigned long)(x))+((n)-1))&(~((n)-1)))
#define v_type(x) ((x) - V_C)

void v_mach_push(struct v_cstate *c, int ty, struct v_arg *arg);

void v_set_fp_imms(void *ip);
inline int v_locked_p(void);

extern int v_istemp(v_reg_type r, int type);
int v_get_temps(int *rv, int n, int class) ;


extern unsigned v_calls;
extern int v_isleaf;
extern int v_ar_size;
#endif

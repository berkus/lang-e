/* $Id: icode.c,v 1.12 1998/05/17 20:38:33 maxp Exp $ */

/* Icode implementation */

#include <assert.h>
#include <stdlib.h>
#include "icode.h"
#include "icode-internal.h"
#include "mem.h"

/*
 * Data
 */
				/* Code */
i_puint32 i_buf, i_lim, i_ip;	/* Pointers to code (start, limit, current) */

				/* Labels */
i_label_t i_lab_cur = min_lab;	/* Current label */

				/* Locals */
i_lrep_t i_locals[max_loc];	/* Array of locals */
i_local_t i_loc_icur = 0;	/* Current int and fp local id */
i_local_t i_loc_fcur = i_loc_ilim;

i_cnt_t num_i, num_f;

				/* Parameters */
i_pmap_t i_params[max_param];	/* Map from param number to local */
i_local_t i_params_cur;		/* Current (maximum) parameter id */

				/* Floating point immediates */
static i_cnt_t i_fpi_lim;		/* Max number of fp immediates */
static i_cnt_t i_fpi_cur;		/* Current fp immediate id */
i_fpi_t * i_fpi;		/* Array of fp immediates */

unsigned int i_leafp = 1;	/* True when current function is a leaf */
i_ref_t i_refc = 1.;

/*
 * Start/end code
 */

/* i_init: begin icode generation */
void i_init (unsigned long ninst) {
     unsigned int i;
     unsigned long bufsz = ninst*i_isize;

     assert(ninst > 0);
				/* Initialize buffer pointers */
     NEW(i_ip, bufsz);
     i_buf = i_ip;
     i_lim = i_buf+bufsz-i_margin;

				/* Initialize parameters */
     for (i=0; i < max_param; i++)
	  i_params[i].t = -1;
     i_reg_init();
}

/* i_end: end icode generation */
void i_end (void) {
     i_retv();			/* Add a final return */
     i_lim = i_ip;		/* Reset code limit */
     num_i = i_loc_icur;
     num_f = i_loc_fcur - i_loc_ilim;
}

/* i_reset: reset icode */
void i_reset (void) {
				/* Reset some things: */
				/* Register lists */
     if (num_i) i_reg_reset(INT);
     if (num_f) i_reg_reset(FP);

     i_lab_cur = min_lab;
				/* Locals */
     i_loc_icur = 0; 
     i_loc_fcur = i_loc_ilim;

     i_fpi = NULL;		/* FP immediates */
     i_fpi_lim = i_fpi_cur = 0;

     i_refc = 1.;		/* Ref counts */

     i_params_cur = 0;		/* Params */
     i_leafp = 1;		/* Leaf flag */
     i_nbb = 0;			/* Number of basic blocks */

     i_calls = NULL;		/* Call addresses */

     tc_adealloc();		/* Free all memory */
}

/*
 * Linking
 */

i_label_t i_mklabel (void) {
     if (i_lab_cur >= max_lab)
	  i_fatal("Too many labels");
     return i_lab_cur++;
}


/*
 * Locals and parameters
 */

/* i_local: allocate a new local */
i_local_t i_local (unsigned long flags, unsigned int type) {
     i_local_t id;
     if (isfloat(type)) {
	  if (i_loc_fcur >= i_loc_flim)
	       i_fatal("Too many floating point locals");
	  id = i_loc_fcur++;
     } else {
	  if (i_loc_icur >= i_loc_ilim)
	       i_fatal("Too many integer locals");
	  id = i_loc_icur++;
     }
     
     TYPE(id) = type;
     if (flags & I_MEMORY) {	/* Check whether local must live on stack */
	  SCLASS(id) = STACK;
	  REFCNT(id) = 0;
     } else {
	  SCLASS(id) = UNDECIDED;
	  REFCNT(id) = (flags & I_REGISTER) ? 50 : 1;
     }
     ADDR(id) = 0; PARAMP(id) = 0;
     LRANGE(id) = NOLRINFO;	/* Reset live range */
     return id;
}

/* i_localb: return a new local aggregate of size bytes */
i_local_t i_localb (unsigned int size) {
     i_local_t id;
     if (i_loc_icur >= i_loc_ilim)
	  i_fatal("Too many integer locals");

     id = i_loc_icur++;
     TYPE(id) = I_B;
     SCLASS(id) = STACK;
     LRANGE(id) = NOLRINFO;
     ADDR(id) = size;		/* Hide size in addr field, since don't need
				   to know both simultaneously */
     REFCNT(id) = 0;
     return id;
}

/* i_param: allocate new parameter of type type for this function */
i_local_t i_param (unsigned int type) {
     i_local_t id;		/* Temporary local id */
     if (i_params_cur >= max_param)
	  i_fatal("Too many parameters");
     id = i_local(0, type);

     SCLASS(id) = REGISTER;	/* Params are placed in registers */
     PARAMP(id) = 1;
				/* Register this parameter */
     i_params[i_params_cur].i = id;
     i_params[i_params_cur].t = type;
     i_params_cur++;		/* Increase parameter count */
     return id;
}

/* i_paramn: allocate the kth parameter for this function; give it type type */
i_local_t i_paramn (unsigned int type, unsigned int k) {
     i_local_t id;		/* Temporary local id */
     if (k >= max_param)
	  i_fatal("Too many parameters");
     if (k >= i_params_cur)
	  i_params_cur = k+1;	/* If necessary, bump max number of params */

     id = i_local(0, type);
     SCLASS(id) = REGISTER;	/* Params are placed in registers */
     PARAMP(id) = 1;
     i_params[k].i = id;	/* Register this param */
     i_params[k].t = type;
     return id;
}

/*
 * Floating point immediates
 */

i_cnt_t i_fpi_addf (float f) {
     growlist(i_fpi_t, i_fpi, i_fpi_cur, i_fpi_lim, dn_fpi);
     i_fpi[i_fpi_cur].f = f;
     return i_fpi_cur++;
}

i_cnt_t i_fpi_addd (double d) {
     growlist(i_fpi_t, i_fpi, i_fpi_cur, i_fpi_lim, dn_fpi);
     i_fpi[i_fpi_cur].d = d;
     return i_fpi_cur++;
}

/*
 * Unparsing
 */

/* i_unparseinsn: parse the icode insn pointed to by cp */
void i_unparseinsn (FILE *fd, i_puint32 cp) {
     if (fd == 0) fd = stdout;
     fprintf(fd, "\t\t%p: ", cp);
     switch(get_op(cp)) {
     case i_op_nop:
	  fprintf(fd, "nop\n"); break;
#include "pp.h"
     case i_op_refmul:
	  fprintf(fd, "REFMUL(%d)\n", get_imm(cp)); break;
     case i_op_refdiv:
	  fprintf(fd, "REFDIV(%d)\n", get_imm(cp)); break;
     case i_op_self: 
	  fprintf(fd, "self(r%d)\n", get_rd(cp)); break;
     case i_op_lbl:
	  fprintf(fd, "L%d:\n", get_rd(cp)); break;
     default:
	  assert(0);
     }
}

/* i_unparse: unparse each insn in the current i_buf */
void i_unparse (void) {
     i_puint32 cp = i_buf;
     while (cp < i_lim) {
	  i_unparseinsn(stdout, cp);
	  switch (i_op2class[get_op(cp)]) {
	  case I_CALL: case I_CALLF: case I_CALLI: case I_CALLIF:
	       cp += 2*i_isize;
	       break;
	  default:
	       cp += i_isize;
	  }
     }
}

/*
 * Emitting to vcode
 */

extern void i_xlate (v_code *self);

/* Cache support */
unsigned i_cachesize = cachesize;
void i_setcachesize (unsigned int s) { i_cachesize = s; }

/* reserve_registers:
   Make some registers unavailable to vcode: required because icode needs
   some temps for spilling/reloading, and these may conflict with temps used
   by vcode for strength reduction. */
static inline void reserve_registers (void) {
#if defined(__sparc__)
     v_mk_unavail(V_I, _g4);
#endif
}

/* i_emit: emits native code corresponding to current icode.  
   Builds flow graph, does register allocation, and invokes vcode to emit code.

   Offset is space (in sizeof v_code*) between beginning of allocated heap 
   space and beginning of code segment (code segment starts at a random offset
   within allocated block to even out cache effects).  Clients can read value
   of offset after invocation to deallocate heap space when it is no longer 
   needed.

   Ninsn is assigned the number of generated instructions.

   A typical code generation sequence would look like:
   		i_init(size);
		i_op1(..); i_op2(..); ...
		i_end();
		int_func = i_emit(&offset,&ninsn).i;
*/
union v_fp i_emit (unsigned int *offset, unsigned int *ninsn) {
     unsigned int i;
     int t;			/* Dummy vcode type */
     unsigned int isz, sz;	/* Size of code buffer: 1 icode -> 5 vcode */
     v_code *cp;		/* Pointer to vcode buffer */
     union v_fp vp;		/* Pointer to finished code */
     static int done_rnd = 0;	/* Flag whether random() needs a seed */
     static int done_reg = 0;	/* Flag whether regs have been reserved */
     unsigned int roff;		/* Random offset for code pointer */
     
     assert(offset);

     isz = i_lim-i_buf;		/* Decide on the vcode buffer size */
     sz = isz * (isz < 10 ? 20 : 10);
     DEBUG(i_unparse());

     i_fg_build();		/* Build the flow graph */
     DEBUG(i_fg_unparse());

     cp = (v_code *)malloc(i_cachesize+sz*sizeof(v_code));
     if (cp == NULL)
	  i_fatal("Out of memory");
#ifndef NDEBUG
     memset((void *)cp, 0xba, i_cachesize+sz*sizeof(v_code));
#endif

     if (!done_rnd) { srand(4); done_rnd = 1; }
     roff = (unsigned)rand();	/* Bump the code pointer randomly */
     roff = i_cachealign(roff);
     cp = (v_code*)((char *)cp + roff);

     if (!done_reg) { reserve_registers(); done_reg = 1; }

				/* Ensure that all params are allocated */
     for (i=0; i < i_params_cur; i++) {
	  if (((t = (int)i_params[i].t)) == -1)
	       i_fatal("Incomplete parameter allocation");
	  v_param_alloc(i, t, &ADDR(i_params[i].i));
     }
				/* Initialize vcode */
     v_clambda("", i_leafp ? V_LEAF : V_NLEAF, cp, sz);

     if (i_quit & END_FG) goto earlyquit;

				/* Allocate code buffer, not from the arena */
     if (num_i || num_f)
	  i_regalloc(i_params);	/* Need to ralloc only if there are locals */

     if (i_quit & (END_LV|END_RA1|END_RA2)) goto earlyquit;

     i_xlate(cp);		/* Translate icode to vcode */
     vp = v_end(&sz);		/* Finish vcode */

     *offset = (char *)vp.v-(((char *)cp)-roff);

     if (ninsn)
	  *ninsn = sz / sizeof *v_ip;
     return vp;

earlyquit:
     v_end(&sz);
     vp.v = (v_vptr)0;
     *offset = 0;
     free((char*)cp-roff);
     return vp;
}

/*
 * Random icode state
 */

				/* Debugging */
unsigned int i_debug = 0;
void i_debug_on (void) { i_debug = 1; }
void i_debug_off (void) { i_debug = 0; }

				/* Reference counts */
unsigned int i_dorefcnt = 1;
void i_refcnt_on  (void) { i_dorefcnt = 1; }
void i_refcnt_off (void) { i_dorefcnt = 0; }

				/* Register allocation mode */
unsigned int i_ralloctype = RA_LS;
void i_ralloc_ez (void) { i_ralloctype = RA_EZ; }
void i_ralloc_gc (void) { i_ralloctype = RA_GC; }
void i_ralloc_lr (void) { i_ralloctype = RA_LR; }
void i_ralloc_ls (void) { i_ralloctype = RA_LS; }

				/* Benchmarking stuff */
unsigned int i_quit;
				/* End after building flow graph */
void i_end_fg (void) { i_quit = END_FG; }
				/* End after computing live variables */
void i_end_lv (void) { i_quit = END_LV; }
				/* End after ralloc part 1 (building IG/LR) */
void i_end_ra1 (void) { i_quit = END_RA1; }
				/* End after complete register allocation */
void i_end_ra2 (void) { i_quit = END_RA2; }

/*
 * Errors
 */

void i_fatal (char *msg) {
     fprintf(stderr, "Fatal error in icode: %s\n", msg);
     exit(-1);
}

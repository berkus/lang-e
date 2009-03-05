 /*
 * macros defining format of instructions
 */

/* used to encode/decode register representations

   XXX -- Now that we don't support virtual registers, these aren't necesary anymore. */

#define __VIRT(x) ((x) < 0)	/* true if x is a virtual register */
#define __NVIRT(x) ((x) >= 0)	/* true if x is not a virtual register */
#define __REG(x) (x)		/* value encoded in insn for reg x */
#define __ADDR(x) (x)		/* offset from __EBP for virtual reg */
#define __MKVIRT(x) (-(x))	/* return rep for virt register x */
#define __MKNVIRT(x) (x)		/* return rep for phys reg x */

/* encodings of the registers used internally and in the instructions
   themselves */

#define __EAX 0x0
#define __ECX 0x1
#define __EDX 0x2
#define __EBX 0x3
#define __ESP 0x4
#define __EBP 0x5
#define __ESI 0x6
#define __EDI 0x7

/* get the next register after a given on, wrapping around at the high end */
#define __REG_NEXT(r) (r == __EBX ? __ESI : (r+1) & 0x7 )

/* register to use as the base for effective address computation */
#define __EAX_EF (__EAX)
#define __ECX_EF (__ECX)
#define __EDX_EF (__EDX)
#define __EBX_EF (__EBX)
#define __EBP_EF (__EBP)
#define __ESI_EF (__ESI)
#define __EDI_EF (__EDI)
#define __SIB    0x4

/* offset from base for effective address. Either single byte follows opcode
   or 32 bit value. Values here are ready to be ORed into mod/rm byte. */

#define __DISP0 0x0
#define __DISP8 0x40
#define __DISP32 0x80

/* generate a mod/rm byte */
#define __modRM(mod,rm,reg) (mod | reg << 3 | rm)

/* generate a SIB byte */
#define __sib(base,index) (index << 3 | base)

/* write some number of bytes out */

#define __cat1(x) *((v_code *)v_ip)++ = x;
#define __cat2(x,y) __cat1 (x); __cat1 (y);
#define __cat3(x,y,z) __cat2 (x,y); __cat1 (z);
#define __cat4(w,x,y,z) __cat3(w,x,y); __cat1(z);
#define __catWord(w) *(int *)v_ip = w; v_ip += 4;
#define __incBy4 v_ip += 4;

/* low level code generation instructions. Each one of these macros
 * generates exactly one x86 instruction. Higher level macros combine
 * these to actually implement a single VCODE instruction.
 *
 * each instruction is generally of the form OpSrc2DestType where Op
 * is the type of instruction (eg mov) Src and Dest are encodings of
 * the type of the source and destination as well as their numbers (eg
 * R2IPlusZero meaning Src is 'R' or a register and Dest is IPlusZero
 * meaing and immediate plus zero. Type is one of the vcode types such
 * as I, U, US, etc...
 *
 * Example: MOVRPlusR2RU is supposed to mean 'MOV' a 'R'egister '2' an
 * effective address computed by adding one 'R'egister 'Plus' another
 * 'R'egister and the data type being manipulated is an 'U'nsigned
 * integer.  */

/* MOV variants */
#define MOVV2RU(addr,reg) __cat3 (0x8b, __modRM (__DISP8, __EBP_EF, reg), addr)
#define MOVR2VU(reg,addr) __cat3 (0x89, __modRM (__DISP8, __EBP_EF, reg), addr)
#define MOVR2RU(reg1,reg2) __cat2 (0x89, 0xc0 | (reg1 << 3) | reg2)
#define MOVI2VUI(imm,addr) __cat3 (0xc7, __modRM (__DISP8, __EBP_EF, 0), addr); __catWord(imm)
#define MOVI2RUI(imm,reg) __cat2 (0xc7, 0xc0 | reg); __catWord (imm)
#define MOVL2R(l,reg) __cat2 (0xc7, 0xc0 | reg); v_dlabel (v_ip, l); __incBy4

/* 32 bit loads */
#define MOVRPlusR2RU(b,i,d) __cat3 (0x8b, __modRM (__DISP0, __SIB, d), __sib (b,i))
#define MOVRPlusZero2RU(b,d) __cat2 (0x8b, __modRM (__DISP0, b, d))
#define MOVRPlusI2RU(b,dsp,d) __cat2 (0x8b, __modRM (__DISP32, b, d)); __catWord (dsp)
#define MOVIPlusZero2RU(dsp, d) __cat2 (0x8b, __modRM (__DISP0, __EBP, d)); __catWord (dsp)

/* 32 bit stores */
#define MOVR2RPlusRU(b,i,d) __cat3 (0x89, __modRM (__DISP0, __SIB, d), __sib (b,i))
#define MOVR2RPlusZeroU(b,d) __cat2 (0x89, __modRM (__DISP0, b, d))
#define MOVR2RPlusIU(b,dsp,d) __cat2 (0x89, __modRM (__DISP32, b, d)); __catWord (dsp)
#define MOVR2IPlusZeroU(dsp,d) __cat2 (0x89, __modRM (__DISP0, __EBP, d)); __catWord (dsp);

/* 16 bit loads */
#define MOVRPlusR2RUS(b,i,d) __cat4 (0x0f, 0xbf, __modRM (__DISP0, __SIB, d), __sib (b,i))
#define MOVRPlusZero2RUS(b,d) __cat3 (0x0f, 0xbf, __modRM (__DISP0, b, d))
#define MOVRPlusI2RUS(b,dsp,d) __cat3 (0x0f, 0xbf, __modRM (__DISP32, b, d)); __catWord (dsp)
#define MOVIPlusZero2RUS(dsp, d) __cat2 (0xc7, __modRM (__DISP0, __EBP, d)); __catWord (dsp)

/* 16 bit stores */
#define MOVR2RPlusRUS(b,i,d) __cat3 (0x89, __modRM (__DISP0, __SIB, d), __sib (b,i))
#define MOVR2RPlusZeroUS(b,d) __cat2 (0x89, __modRM (__DISP0, b, d))
#define MOVR2RPlusIUS(b,dsp,d) __cat2 (0x89, __modRM (__DISP32, b, d)); __catWord (dsp)
#define MOVR2IPlusZeroUS(dsp,d) __cat2 (0x89, __modRM (__DISP0, __EBP, d)); __catWord (dsp)

/* 8 bit loads*/
#define MOVRPlusR2RUC(b,i,d) __cat4 (0x0f, 0xbe, __modRM (__DISP0, __SIB, d), __sib (b,i))
#define MOVRPlusZero2RUC(b,d) __cat3 (0x0f, 0xbe, __modRM (__DISP0, b, d))
#define MOVRPlusI2RUC(b,dsp,d) __cat3 (0x0f, 0xbe, __modRM (__DISP32, b, d)); __catWord (dsp)
#define MOVIPlusZero2RUC(dsp, d) __cat3 (0x0f, 0xbe, __modRM (__DISP0, __EBP, d)); __catWord (dsp)

/* 8 bit stores */
#define MOVR2RPlusRUC(b,i,d) __cat3 (0x88, __modRM (__DISP0, __SIB, d), __sib (b,i))
#define MOVR2RPlusZeroUC(b,d) __cat2 (0x88, __modRM (__DISP0, b, d))
#define MOVR2RPlusIUC(b,dsp,d) __cat2 (0x88, __modRM (__DISP32, b, d)); __catWord (dsp)
#define MOVR2IPlusZeroUC(dsp,d) __cat2 (0x88, __modRM (__DISP0, __EBP, d)); __catWord (dsp)

/* COMPLEMENT variants (1's complement) */
#define COMRU(reg) __cat2 (0xf7, 0xd0 | reg)
#define COMVU(addr) __cat3 (0xf7, __modRM (__DISP8, __EBP_EF, 0x2), addr)
#define COMI2RUI(imm,reg) MOVI2RUI(imm,reg); COMRU(reg)
#define COMI2VUI(imm,addr) MOVI2VUI(imm,addr); COMVU(addr)

/* NEG variants (2's complement) */
#define NEGRU(reg) __cat2 (0xf7, 0xd8 | reg)
#define NEGVU(addr) __cat3 (0xf7, __modRM (__DISP8, __EBP_EF, 0x3), addr)
#define NEGI2RUI(imm,reg) MOVI2RUI(imm,reg); NEGRU(reg)
#define NEGI2VUI(imm,addr) MOVI2VUI(imm,addr); NEGVU(addr)

/* ADD variants */
#define ADDV2RU(addr,reg) __cat3 (0x03, __modRM (__DISP8, __EBP_EF, reg), addr)
#define ADDR2VU(reg,addr) __cat3 (0x01, __modRM (__DISP8, __EBP_EF, reg), addr)
#define ADDR2RU(reg2,reg1) __cat2 (0x03, 0xc0 | (reg1 << 3) | reg2)
#define ADDI2VU(imm,addr) __cat3 (0x81, __modRM (__DISP8, __EBP_EF, 0), addr); __catWord (imm)
#define ADDI2RU(imm,reg) __cat2 (0x81, 0xc0 | reg); __catWord (imm)

/* SUB variants */
#define SUBR2VU(reg,addr) __cat3 (0x29, __modRM (__DISP8, __EBP_EF, reg), addr)
#define SUBV2RU(addr, reg) __cat3 (0x2b, __modRM (__DISP8, __EBP_EF, reg), addr)
#define SUBR2RU(reg1,reg2) __cat2 (0x29, 0xc0 | (reg1 << 3) | reg2)
#define SUBI2VU(imm,addr) __cat3 (0x81, __modRM (__DISP8, __EBP_EF, 0x5), addr); __catWord (imm)
#define SUBI2RU(imm,reg) __cat2 (0x81, 0xe8 | reg); __catWord (imm)

/* AND variants */
#define ANDV2RU(addr,reg) __cat3 (0x23, __modRM (__DISP8, __EBP_EF, reg), addr)
#define ANDR2VU(reg,addr) __cat3 (0x21, __modRM (__DISP8, __EBP_EF, reg), addr)
#define ANDR2RU(reg1,reg2) __cat2 (0x21, 0xc0 | (reg1 << 3) | reg2)
#define ANDI2VU(imm,addr) __cat3 (0x81, __modRM (__DISP8, __EBP_EF, 4), addr); __catWord (imm)
#define ANDI2RU(imm,reg) __cat2 (0x81, 0xe0 | reg); __catWord (imm)

/* OR variants */
#define ORV2RU(addr,reg) __cat3 (0x0b, __modRM (__DISP8, __EBP_EF, reg), addr)
#define ORR2VU(reg,addr) __cat3 (0x09, __modRM (__DISP8, __EBP_EF, reg), addr)
#define ORR2RU(reg1,reg2) __cat2 (0x09, 0xc0 | (reg1 << 3) | reg2)
#define ORI2VU(imm,addr) __cat3 (0x81, __modRM (__DISP8, __EBP_EF, 1), addr); __catWord (imm)
#define ORI2RU(imm,reg) __cat2 (0x81, 0xc8 | reg); __catWord (imm)

/* XOR variants */
#define XORV2RU(addr,reg) __cat3 (0x33, __modRM (__DISP8, __EBP_EF, reg), addr)
#define XORR2VU(reg,addr) __cat3 (0x31, __modRM (__DISP8, __EBP_EF, reg), addr)
#define XORR2RU(reg1,reg2) __cat2 (0x31, 0xc0 | (reg1 << 3) | reg2)
#define XORI2VU(imm,addr) __cat3 (0x81, __modRM (__DISP8, __EBP_EF, 0), addr); __catWord (imm)
#define XORI2RU(imm,reg) __cat2 (0x81, 0xf0 | reg); __catWord (imm)

/* PUSH variants */
#define PUSHR(reg) __cat2 (0xff, 0xf0 | reg)
#define PUSHV(addr) __cat3 (0xff, __modRM (__DISP8, __EBP_EF, 6), addr)
#define PUSHI(imm) __cat1 (0x68); __catWord (imm)

/* POP variants */
#define POPR(reg) __cat2 (0x8f, 0xc0 | reg)

/* LSH variants */
#define LSHRU(reg) __cat2 (0xd3, 0xe0 | reg)
#define LSHVU(addr) __cat3 (0xd3, __modRM (__DISP8, __EBP_EF, 4), addr)
#define LSHRByIU(reg,imm) __cat3 (0xc1, 0xe0 | reg, imm)
#define LSHVByIU(addr,imm) __cat4 (0xc1, __modRM (__DISP8, __EBP_EF, 4), addr, imm)

/* RSH (unsigned) variants */
#define RSHRU(reg) __cat2 (0xd3, 0xe8 | reg)
#define RSHVU(addr) __cat3 (0xd3, __modRM (__DISP8, __EBP_EF, 5), addr)
#define RSHRByIU(reg,imm) __cat3 (0xc1, 0xe8 | reg, imm)
#define RSHVByIU(addr,imm) __cat4 (0xc1, __modRM (__DISP8, __EBP_EF, 5), addr, imm)

/* RSH (signed) variants */
#define RSHRI(reg) __cat2 (0xd3, 0xf8 | reg)
#define RSHVI(addr) __cat3 (0xd3, __modRM (__DISP8, __EBP_EF, 7), addr)
#define RSHRByII(reg,imm) __cat3 (0xc1, 0xf8 | reg, imm)
#define RSHVByII(addr,imm) __cat4 (0xc1, __modRM (__DISP8, __EBP_EF, 7), addr, imm)

/* MUL (signed) variants */
#define MULR2RI(reg1,reg2) __cat3 (0x0f, 0xaf, 0xc0 | reg2 << 3 | reg1)
#define MULV2RI(addr,reg) __cat4 (0x0f, 0xaf, __modRM (__DISP8, __EBP_EF, reg), addr)
/* result has to go to register with IMUL, so we gen several insns here */
#define MULR2VI(reg,addr) PUSHR(__EAX);                	       	       	       	       	       	   \
			  MOVV2RU(addr,__EAX); 							   \
			  __cat2 (0xf7, 0xe0 | reg); 						   \
			  MOVR2VU(__EAX, addr); 						   \
			  POPR (__EAX)
#define MULI2RI(imm,reg) {  if (imm == 2) {                                                        \
                              ADDR2RU (reg, reg); 						   \
			    } else if ((imm & -imm) == imm ) {					   \
			      unsigned i = mul_str_reduce (imm); 				   \
			      LSHRByIU (reg, i);						   \
			    } else {								   \
			      __cat2 (0x69, 0xc0 | reg << 3 | reg); 				   \
			      __catWord(imm);							   \
			    }									   \
			  }
#define MULI2VI(imm,addr) {                                                                         \
			    if ((imm & -imm) == imm) {						    \
			      unsigned i = mul_str_reduce (imm);				    \
			      LSHVByIU (addr, i);						    \
			    } else {								    \
			      PUSHR (__EAX);							    \
			      __cat3 (0x69, __modRM (__DISP8, __EBP_EF, __EAX), addr);		    \
	                     __catWord (imm); 							    \
			      POPR (__EAX);							    \
			    }									    \
			  }

/* MUL and DIV instructions assume that __EDX has already been saved
   and that __EAX has been loaded. Result ends up in __EDX:__EAX */

/* MUL (unsigned) variants */
#define MULRU(reg) __cat2 (0xf7, 0xe0 | reg)
#define MULVU(addr) __cat3 (0xf7, __modRM (__DISP8, __EBP_EF, 4), addr)

/* DIV (signed) variants */
#define DIVRI(reg) __cat3 (0x99, 0xf7, 0xf8 | reg)
#define DIVVI(addr) __cat4 (0x99, 0xf7, __modRM (__DISP8, __EBP_EF, 7), addr)

/* DIV (unsigned) variants */
#define DIVRU(reg) XORR2RU (__EDX, __EDX); __cat2 (0xf7, 0xf0 | reg)
#define DIVVU(addr) XORR2RU (__EDX, __EDX); __cat3 (0xf7, __modRM (__DISP8, __EBP_EF, 6), addr)

/* SETcc variants */
#define SETER(reg) __cat3 (0x0f, 0x90 | __EQ, 0xc0 | reg)
#define SETEV(addr) __cat4 (0x0f, 0x90 | __EQ, __modRM (__DISP8, __EBP_EF, 0), addr)

/* CMP variants */
#define CMPRByRI(reg2,reg1) CMPRByRU(reg2,reg1)
#define CMPVByRI(addr,reg) CMPVByRU(addr,reg)
#define CMPRByVI(reg,addr) CMPRByVU(reg,addr)
#define CMPRByII(reg,imm) CMPRByIU(reg,imm)
#define CMPVByII(addr,imm) CMPVByIU(addr,imm)

#define CMPRByRU(reg1,reg2) __cat2 (0x39, 0xc0 | reg2 << 3  | reg1)
#define CMPRByVU(reg,addr) __cat3 (0x3b, __modRM (__DISP8, __EBP_EF, reg), addr)
#define CMPVByRU(addr,reg) __cat3 (0x39, __modRM (__DISP8, __EBP_EF, reg), addr)
#define CMPRByIU(reg,imm)  __cat2 (0x81, 0xf8 | reg); __catWord (imm)
#define CMPVByIU(addr,imm) __cat3 (0x81, __modRM (__DISP8, __EBP_EF, 7), addr); __catWord (imm)

/* CJUMP variants. First group if for signed comparisons while second
   group is for unsigned. */

#define __LT 0xc
#define __LE 0xe
#define __GT 0xf
#define __GE 0xd
#define __EQ 0x4
#define __NE 0x5

#define __NAE 0x2
#define __AE 0x3
#define __BE 0x6
#define __NBE 0x7

#define CJUMP(cond,label) __cat2 (0x0f, 0x80 | cond); v_dmark ((v_code *)v_ip, label); __incBy4

/* jump writes the jump opcode but we don't know the displacement to
   the label yet, so we remember this location for later backpatching,
   and just push up the instruction pointer */

/* unconditional jumps to labels or registers */
#define JUMPV_SZ 5
#define JUMPV(label) __cat1 (0xe9); v_dmark ((v_code *)v_ip, label); __incBy4
#define JUMPRP(reg) __cat2 (0xff, 0xe0 | reg)
#define JUMPVP(addr) __cat3 (0xff, __modRM (__DISP8, __EBP_EF, 4), addr)
#define JUMPI(imm) __cat1 (0xe9); __catWord ((unsigned )imm-(unsigned )v_ip-4)

/* CALL variants */
#define CALLV(label) __cat1 (0xe8); v_dmark ((v_code *)v_ip, label); __incBy4
#define CALLI(imm) __cat1 (0xe8); __catWord ((int )imm - ((int )v_ip+4))
#define CALLRP(reg) __cat2 (0xff, 0xd0 | reg)
#define CALLVP(addr) __cat3 (0xff, __modRM (__DISP8, __EBP_EF, 0x2), addr)

/* RET */
#define RET __cat1 (0xc3)

/* NOP */
#define __NOP __cat1 (0x90)

/*
 * higher-order macros that generate individual defs. for instructions.
 * These macros translate VCODE instructions which use virtual registers
 * and three address forms to a sequence of x86 instructions using only
 * explicity memory access and registers along with only two operands per
 * instruction.
 */

static inline v_reg_t __find_free_reg3 (v_reg_t x, v_reg_t y, v_reg_t z) {
  v_reg_t v;

  for (v = __EAX;; v = __REG_NEXT(v)) {
    if (v == x) continue;
    if (v == y) continue;
    if (v == z) continue;
    break;
  }
  return v;
}

static inline v_reg_t __find_free_reg2 (v_reg_t y, v_reg_t z) {
  v_reg_t v;

  for (v = __EAX;; v = __REG_NEXT(v)) {
    if (v == y) continue;
    if (v == z) continue;
    break;
  }
  return v;
}

/* have to explicity define the shifts since they implicityly use CL */

#define SHIFT(name,op,type)                            	       	       	       	       	       	\
static inline void _V_##name##type##_ (v_reg_t rd, v_reg_t rs1, v_reg_t rs2) {			\
  int dest_is_ecx = 0;					                        \
												\
  if (__REG(rd) == __ECX) {									\
    dest_is_ecx = 1;										\
    rd = __EBP;  										\
    PUSHR (__EBP);										\
  }												\
												\
  if (rs1 != rd) {										\
    MOVR2RU (__REG(rs1), __REG(rd));								\
  }												\
												\
  if (__REG(rs2) != __ECX) {									\
    if (!dest_is_ecx) {										\
      PUSHR(__ECX);										\
    }												\
    MOVR2RU (__REG(rs2), __ECX);								\
  }												\
  												\
  op##R##type## (__REG(rd));									\
												\
  if (dest_is_ecx) {										\
    MOVR2RU (__EBP, __ECX);									\
    POPR (__EBP);              	       	       	       	       	       	       	       	       	\
  } else {											\
    POPR (__ECX);										\
  }												\
}

#define SHIFTI(name,op,type)   	       	       	       	       	       	       	       	       	     \
static inline void _V_##name##type##I_ (v_reg_t rd, v_reg_t rs, int imm) { 			     \
  if (rs != rd) {										     \
    MOVR2RU (__REG(rs), __REG(rd));								     \
  }												     \
  op##RByI##type (__REG(rd), imm);               						     \
}

/* comutative binary operation */

#define COMMUTATIVE_BINARY(name,op,type)               	       	       	       	       	       	     \
static inline void _V_##name##type##_ (v_reg_t rd, v_reg_t rs1, v_reg_t rs2) {			     \
  if (rs2 == rd) {    										     \
    op##R2R##type (__REG(rs1), __REG(rs2));        						     \
    return;                                    							     \
  }                                              						     \
                                                 						     \
  if (rs1 == rd)  {   										     \
    op##R2R##type (__REG(rs2), __REG(rs1));          						     \
    return;                                      						     \
  }												     \
												     \
  MOVR2RU (__REG(rs1), __REG(rd));             							     \
  op##R2R##type (__REG(rs2), __REG(rd));							     \
}


/* non-comutative binary operation */

#define NONCOMMUTATIVE_BINARY(name,op,type)            	       	       	       	       	       	     \
static inline void _V_##name##type##_ (v_reg_t rd, v_reg_t rs1, v_reg_t rs2) {			     \
												     \
  if (rs1 == rd) {										     \
    op##R2R##type (__REG (rs2), __REG(rs1));       						     \
    return;                                    							     \
  }                                              						     \
                                                 						     \
  PUSHR (__EBP);                                 						     \
  MOVR2RU (__REG(rs1), __EBP);                     						     \
  op##R2R##type (__REG(rs2), __EBP);               						     \
  MOVR2RU (__EBP, __REG(rd));                     						     \
  POPR (__EBP);                                  						     \
}

/* binary instruction with second operand an imediate */

#define BINARY_IMM(name,op,type)                                                                     \
static inline void _V_##name##type##I_ (v_reg_t rd, v_reg_t rs, int imm) {			     \
  if (rs != rd){										     \
    MOVR2RU (__REG(rs), __REG(rd));								     \
  }												     \
  op##I2R##type (imm, __REG(rd));                						     \
}

/* generate a compare and branch sequence */

#define BRANCH(name,op,type)                           	       	       	       	       	       	     \
static inline void _V_##name##type##_ (v_reg_t rs1, v_reg_t rs2, 				     \
				       v_label_t label) { 					     \
    CMPRByR##type (__REG(rs1), __REG(rs2));          						     \
    CJUMP(##op##,label);			 						     \
}

#define BRANCHI(name,op,type)                                                                        \
static inline void _V_##name##type##I_ (v_reg_t rs1, int imm, 					     \
					v_label_t label) { 					     \
                                                 						     \
  CMPRByI##type (__REG(rs1), imm);               						     \
  CJUMP(##op##,label);				 						     \
}

/* Sepcial cased code for div or unsigned mul since they require rs1 to be
   __EAX and rd is __EDX:__EAX. */

#define DIV_OR_MULU(name,op,type,res)                  	       	       	       	       	       	     \
static inline void _V_##name##type##_ (v_reg_t rd,      					     \
			     v_reg_t rs1,     							     \
			     v_reg_t rs2) {   							     \
                                                                                                     \
  v_reg_t extra_reg;                                                                                 \
  int restore_rs2 = 0, restore_eax = 1;                                                              \
												     \
  /* Get rs2 into something besides EAX */							     \
  if (__REG(rs2) == __EAX) {									     \
    extra_reg = __EBP; /* __find_free_reg2 (__EAX, __REG(rs1)); */						     \
    if (__REG(rd) != extra_reg) {                                                                    \
       PUSHR (extra_reg);									     \
       restore_rs2 = 1;										     \
    }                                                                                                \
    MOVR2RU (__REG(rs2), extra_reg);								     \
    rs2 = extra_reg;										     \
  }												     \
												     \
  /* Get source into __EAX */                      						     \
  if (__REG(rs1) != __EAX) {                               					     \
    if (__REG(rd) != __EAX) {                                                                        \
       PUSHR (__EAX);                                 						     \
       restore_eax = 1;                                                                              \
    }                                                                                                \
    MOVR2RU (__REG(rs1), __EAX);	                 					     \
  }                                              						     \
  												     \
  /* result goes into DX so save it (plus signed div uses DX as part of rs1) */ 		     \
  /* XXX does this mean we should zero edx for signed div? */					     \
  if (__REG(rd) != res) {                               					     \
    PUSHR (res);  										     \
  }                                              						     \
                                                 						     \
  /* ok, do the op */                            						     \
  op##R##type (__REG(rs2));                      						     \
                                                 						     \
  /* result is now in res--move it to the right place */ 					     \
  if (__REG(rd) != res) {                               					     \
    MOVR2RU (res, __REG(rd));               						     	     \
    POPR (res);											     \
  }                                              						     \
                                                 						     \
  if (restore_eax) {										     \
    POPR (__EAX);										     \
  }												     \
												     \
  if (restore_rs2) {										     \
    POPR (__REG(rs2));										     \
  }												     \
}

#define DIV_OR_MULU_IMM(name,op,type,res)                                                            \
static inline void _V_##name##type##I_ (v_reg_t rd,      					     \
			     v_reg_t rs1,     							     \
			     int imm) {          						     \
  /* Get source into __EAX */                      						     \
  if (rs1 != __EAX) {                               						     \
    PUSHR (__EAX);										     \
    MOVR2RU (__REG(rs1), __EAX);								     \
  }                                              						     \
												     \
  /* result goes into DX so save it (plus signed div uses DX as part of rs1) */ 		     \
  /* XXX again, does this mean we should clear edx ? */						     \
  if (rd != __EDX) {                               						     \
    PUSHR (__EDX);                                 						     \
  }                                              						     \
                                                 						     \
  /* can't take immediate operands, so we have to load into another register */ 		     \
  PUSHR (__ECX);                                   						     \
  MOVI2RUI (imm, __ECX);                           						     \
  op##R##type (__ECX);                             						     \
  POPR (__ECX);                                    						     \
                                                 						     \
  /* result is now in res--move it to the right place */ 					     \
  if (res != rd) {                               						     \
    MOVR2RU (__REG(res), __REG(rd));               						     \
  }                                              						     \
                                                 						     \
  /* and restore the saved registers */          						     \
  if (rd != __EDX) {                               						     \
    POPR (__EDX);                                  						     \
  }                                              						     \
  if (rs1 != __EAX) {                               						     \
    POPR (__EAX);                                  						     \
  }                                              						     \
}

/* load/store */

#define LDST(name,op,op_zero,type)                      	       	                             \
static inline void _V_##name##type##_ (v_reg_t rd,			 			     \
				       v_reg_t rs1,			 			     \
				       v_reg_t rs2) {		 				     \
  if (rs1 == v_zero) {		 								     \
    op_zero##type (__REG(rs2), __REG(rd));                                                           \
  } else {											     \
    op##type (__REG(rs1), __REG(rs2), __REG(rd));				 		     \
  }												     \
}

#define LDSTI(name,op,op_zero,type)                                    	       	       	             \
static inline void _V_##name##type##I_ (v_reg_t rd,			 			     \
        			       v_reg_t rs,			 			     \
        			       int imm) {			 			     \
  if (rs == v_zero) {										     \
    op_zero##type (imm, __REG(rd));                                                                  \
  } else {											     \
    op##type (__REG(rs), imm, __REG(rd));					 		     \
  }									 			     \
}

/* have to special case NOT */
static inline void _V_NOTU_(v_reg_t rd,
			    v_reg_t rs) {
  CMPRByIU (__REG(rs), 0);
  MOVI2RUI (0, __REG(rd));
  SETER (__REG(rd));
}

/* unary operations */

#define UNARY(name,op,type)                                                                          \
static inline void _V_##name##type##_ (v_reg_t rd,         					     \
				     v_reg_t rs) {       					     \
  if (__REG(rs) != __REG(rd)) {                    						     \
    MOVR2RU (__REG(rs), __REG(rd));								     \
  }												     \
  op##R##type (__REG(rd));                       						     \
}

#define UNARY_IMM(name,op,type)                                                                      \
static inline void _V_##name##type##I_ (v_reg_t rd,          					     \
				     int imm) {              					     \
  op##I2R##type##I (imm, __REG(rd));             						     \
}

extern v_label_t v_epilouge_label;

#define _V_RETV_ JUMPV(v_epilouge_label);

static inline void _V_RETU_ (v_reg_t reg) {
  if (__REG(reg) != __EAX) {
    MOVR2RU (__REG(reg), __EAX);
  }
  JUMPV(v_epilouge_label);
}

static inline void _V_RETUI_ (unsigned int imm) {
  MOVI2RUI (imm, __EAX);
  JUMPV(v_epilouge_label);
}

#define _V_MOVU_(rd,rs) MOVR2RU(rs,rd)


/*
 * Finnally, define the actual VCODE instructions using the code
 * generating macros defined imm__Ediately above.
 */

#define v_addi(rd,rs1,rs2) _V_ADDU_(rd,rs1,rs2)
#define v_addu(rd,rs1,rs2) _V_ADDU_(rd,rs1,rs2)
#define v_addp(rd,rs1,rs2) _V_ADDU_(rd,rs1,rs2)
#define v_addl(rd,rs1,rs2) _V_ADDU_(rd,rs1,rs2)
#define v_addul(rd,rs1,rs2) _V_ADDU_(rd,rs1,rs2)
#define v_addii(rd,rs1,rs2) _V_ADDUI_(rd,rs1,rs2)
#define v_addui(rd,rs1,rs2) _V_ADDUI_(rd,rs1,rs2)
#define v_addpi(rd,rs1,rs2) _V_ADDUI_(rd,rs1,rs2)
#define v_addli(rd,rs1,rs2) _V_ADDUI_(rd,rs1,rs2)
#define v_adduli(rd,rs1,rs2) _V_ADDUI_(rd,rs1,rs2)
COMMUTATIVE_BINARY (ADD,ADD,U)
BINARY_IMM (ADD,ADD,U)

#define v_subi(rd,rs1,rs2) _V_SUBU_(rd,rs1,rs2)
#define v_subu(rd,rs1,rs2) _V_SUBU_(rd,rs1,rs2)
#define v_subl(rd,rs1,rs2) _V_SUBU_(rd,rs1,rs2)
#define v_subul(rd,rs1,rs2) _V_SUBU_(rd,rs1,rs2)
#define v_subii(rd,rs1,rs2) _V_SUBUI_(rd,rs1,rs2)
#define v_subui(rd,rs1,rs2) _V_SUBUI_(rd,rs1,rs2)
#define v_subli(rd,rs1,rs2) _V_SUBUI_(rd,rs1,rs2)
#define v_subuli(rd,rs1,rs2) _V_SUBUI_(rd,rs1,rs2)
NONCOMMUTATIVE_BINARY (SUB,SUB,U)
BINARY_IMM (SUB,SUB,U)

#define v_andi(rd,rs1,rs2) _V_ANDU_(rd,rs1,rs2)
#define v_andu(rd,rs1,rs2) _V_ANDU_(rd,rs1,rs2)
#define v_andl(rd,rs1,rs2) _V_ANDU_(rd,rs1,rs2)
#define v_andul(rd,rs1,rs2) _V_ANDU_(rd,rs1,rs2)
#define v_andii(rd,rs1,rs2) _V_ANDUI_(rd,rs1,rs2)
#define v_andui(rd,rs1,rs2) _V_ANDUI_(rd,rs1,rs2)
#define v_andli(rd,rs1,rs2) _V_ANDUI_(rd,rs1,rs2)
#define v_anduli(rd,rs1,rs2) _V_ANDUI_(rd,rs1,rs2)
COMMUTATIVE_BINARY (AND,AND,U)
BINARY_IMM (AND,AND,U)

#define v_ori(rd,rs1,rs2) _V_ORU_(rd,rs1,rs2)
#define v_oru(rd,rs1,rs2) _V_ORU_(rd,rs1,rs2)
#define v_orl(rd,rs1,rs2) _V_ORU_(rd,rs1,rs2)
#define v_orul(rd,rs1,rs2) _V_ORU_(rd,rs1,rs2)
#define v_orii(rd,rs1,rs2) _V_ORUI_(rd,rs1,rs2)
#define v_orui(rd,rs1,rs2) _V_ORUI_(rd,rs1,rs2)
#define v_orli(rd,rs1,rs2) _V_ORUI_(rd,rs1,rs2)
#define v_oruli(rd,rs1,rs2) _V_ORUI_(rd,rs1,rs2)
COMMUTATIVE_BINARY (OR,OR,U)
BINARY_IMM (OR,OR,U)

#define v_muli(rd,rs1,rs2) _V_MULI_(rd,rs1,rs2)
#define v_mull(rd,rs1,rs2) _V_MULI_(rd,rs1,rs2)
#define v_mulii(rd,rs1,rs2) _V_MULII_(rd,rs1,rs2)
#define v_mulli(rd,rs1,rs2) _V_MULII_(rd,rs1,rs2)
COMMUTATIVE_BINARY (MUL,MUL,I)
BINARY_IMM (MUL,MUL,I)

#define v_mulu(rd,rs1,rs2) _V_MULU_(rd,rs1,rs2)
#define v_mulul(rd,rs1,rs2) _V_MULU_(rd,rs1,rs2)
#define v_mului(rd,rs1,rs2) _V_MULUI_(rd,rs1,rs2)
#define v_mululi(rd,rs1,rs2) _V_MULUI_(rd,rs1,rs2)
DIV_OR_MULU (MUL, MUL, U, __EAX)
DIV_OR_MULU_IMM (MUL, MUL, U, __EAX)

#define v_divi(rd, rs1, rs2) _V_DIVI_(rd,rs1,rs2)
#define v_divu(rd, rs1, rs2) _V_DIVU_(rd,rs1,rs2)
#define v_divl(rd, rs1, rs2) _V_DIVI_(rd,rs1,rs2)
#define v_divul(rd, rs1, rs2) _V_DIVU_(rd,rs1,rs2)
#define v_divii(rd, rs1, rs2) _V_DIVII_(rd,rs1,rs2)
#define v_divui(rd, rs1, rs2) _V_DIVUI_(rd,rs1,rs2)
#define v_divli(rd, rs1, rs2) _V_DIVII_(rd,rs1,rs2)
#define v_divuli(rd, rs1, rs2) _V_DIVUI_(rd,rs1,rs2)
DIV_OR_MULU (DIV, DIV, U, __EAX)
DIV_OR_MULU (DIV, DIV, I, __EAX)
DIV_OR_MULU_IMM (DIV, DIV, U, __EAX)
DIV_OR_MULU_IMM (DIV, DIV, I, __EAX)

#define v_modi(rd, rs1, rs2) _V_MODI_(rd,rs1,rs2)
#define v_modu(rd, rs1, rs2) _V_MODU_(rd,rs1,rs2)
#define v_modl(rd, rs1, rs2) _V_MODI_(rd,rs1,rs2)
#define v_modul(rd, rs1, rs2) _V_MODU_(rd,rs1,rs2)
#define v_modii(rd, rs1, rs2) _V_MODII_(rd,rs1,rs2)
#define v_modui(rd, rs1, rs2) _V_MODUI_(rd,rs1,rs2)
#define v_modli(rd, rs1, rs2) _V_MODII_(rd,rs1,rs2)
#define v_moduli(rd, rs1, rs2) _V_MODUI_(rd,rs1,rs2)
DIV_OR_MULU (MOD, DIV, U, __EDX)
DIV_OR_MULU (MOD, DIV, I, __EDX)
DIV_OR_MULU_IMM (MOD, DIV, U, __EDX)
DIV_OR_MULU_IMM (MOD, DIV, I, __EDX)

#define v_xori(rd,rs1,rs2) _V_XORU_(rd,rs1,rs2)
#define v_xoru(rd,rs1,rs2) _V_XORU_(rd,rs1,rs2)
#define v_xorl(rd,rs1,rs2) _V_XORU_(rd,rs1,rs2)
#define v_xorul(rd,rs1,rs2) _V_XORU_(rd,rs1,rs2)
#define v_xorii(rd,rs1,rs2) _V_XORUI_(rd,rs1,rs2)
#define v_xorui(rd,rs1,rs2) _V_XORUI_(rd,rs1,rs2)
#define v_xorli(rd,rs1,rs2) _V_XORUI_(rd,rs1,rs2)
#define v_xoruli(rd,rs1,rs2) _V_XORUI_(rd,rs1,rs2)
COMMUTATIVE_BINARY (XOR,XOR,U)
BINARY_IMM (XOR,XOR,U)

#define v_lshi(rd,rs1,rs2) _V_LSHU_(rd,rs1,rs2)
#define v_lshu(rd,rs1,rs2) _V_LSHU_(rd,rs1,rs2)
#define v_lshl(rd,rs1,rs2) _V_LSHU_(rd,rs1,rs2)
#define v_lshul(rd,rs1,rs2) _V_LSHU_(rd,rs1,rs2)
#define v_lshii(rd,rs1,rs2) _V_LSHUI_(rd,rs1,rs2)
#define v_lshui(rd,rs1,rs2) _V_LSHUI_(rd,rs1,rs2)
#define v_lshli(rd,rs1,rs2) _V_LSHUI_(rd,rs1,rs2)
#define v_lshuli(rd,rs1,rs2) _V_LSHUI_(rd,rs1,rs2)
SHIFT(LSH,LSH,U)
SHIFTI(LSH,LSH,U)

#define v_rshi(rd,rs1,rs2) _V_RSHI_(rd,rs1,rs2)
#define v_rshu(rd,rs1,rs2) _V_RSHU_(rd,rs1,rs2)
#define v_rshl(rd,rs1,rs2) _V_RSHI_(rd,rs1,rs2)
#define v_rshul(rd,rs1,rs2) _V_RSHU_(rd,rs1,rs2)
#define v_rshii(rd,rs1,rs2) _V_RSHII_(rd,rs1,rs2)
#define v_rshui(rd,rs1,rs2) _V_RSHUI_(rd,rs1,rs2)
#define v_rshli(rd,rs1,rs2) _V_RSHII_(rd,rs1,rs2)
#define v_rshuli(rd,rs1,rs2) _V_RSHUI_(rd,rs1,rs2)
SHIFT(RSH,RSH,U)
SHIFT(RSH,RSH,I)
SHIFTI(RSH,RSH,U)
SHIFTI(RSH,RSH,I)

#define v_movi(rd,rs)  _V_MOVU_(rd,rs)
#define v_movu(rd,rs)  _V_MOVU_(rd,rs)
#define v_movl(rd,rs)  _V_MOVU_(rd,rs)
#define v_movul(rd,rs) _V_MOVU_(rd,rs)
#define v_movp(rd,rs)  _V_MOVU_(rd,rs)

#define v_seti(rd,imm) _V_MOVUI_(rd,imm)
#define v_setu(rd,imm) _V_MOVUI_(rd,imm)
#define v_setl(rd,imm) _V_MOVUI_(rd,imm)
#define v_setul(rd,imm) _V_MOVUI_(rd,imm)
#define v_setp(rd,imm) _V_MOVUI_(rd,imm)
#define v_setv(rd,imm) _V_MOVUI_(rd,imm)
#define v_sets(rd,imm) _V_MOVUI_(rd,imm)
#define v_setus(rd,imm) _V_MOVUI_(rd,imm)
#define v_setc(rd,imm) _V_MOVUI_(rd,imm)
#define v_setuc(rd,imm) _V_MOVUI_(rd,imm)
UNARY_IMM(MOV,MOV,U)

#define v_comu(rd,rs) _V_COMU_(rd,rs)
#define v_comi(rd,rs) _V_COMU_(rd,rs)
#define v_coml(rd,rs) _V_COMU_(rd,rs)
#define v_comul(rd,rs) _V_COMU_(rd,rs)
#define v_comui(rd,rs) _V_COMUI_(rd,rs)
#define v_comii(rd,rs) _V_COMUI_(rd,rs)
#define v_comli(rd,rs) _V_COMUI_(rd,rs)
#define v_comuli(rd,rs) _V_COMUI_(rd,rs)
UNARY(COM,COM,U)
UNARY_IMM(COM,COM,U)

#define v_notu(rd,rs) _V_NOTU_(rd,rs)
#define v_noti(rd,rs) _V_NOTU_(rd,rs)
#define v_notl(rd,rs) _V_NOTU_(rd,rs)
#define v_notul(rd,rs) _V_NOTU_(rd,rs)
#define v_notui(rd,rs) _V_NOTUI_(rd,rs)
#define v_notii(rd,rs) _V_NOTUI_(rd,rs)
#define v_notli(rd,rs) _V_NOTUI_(rd,rs)
#define v_notuli(rd,rs) _V_NOTUI_(rd,rs)

#define v_negu(rd,rs) _V_NEGU_(rd,rs)
#define v_negi(rd,rs) _V_NEGU_(rd,rs)
#define v_negl(rd,rs) _V_NEGU_(rd,rs)
#define v_negul(rd,rs) _V_NEGU_(rd,rs)
#define v_negui(rd,rs) _V_NEGUI_(rd,rs)
#define v_negii(rd,rs) _V_NEGUI_(rd,rs)
#define v_negli(rd,rs) _V_NEGUI_(rd,rs)
#define v_neguli(rd,rs) _V_NEGUI_(rd,rs)
UNARY(NEG,NEG,U)
UNARY_IMM(NEG,NEG,U)

#define v_blti(rs1,rs2,label) _V_BLTI_(rs1,rs2,label)
#define v_bltu(rs1,rs2,label) _V_BLTU_(rs1,rs2,label)
#define v_bltl(rs1,rs2,label) _V_BLTI_(rs1,rs2,label)
#define v_bltul(rs1,rs2,label) _V_BLTU_(rs1,rs2,label)
#define v_bltp(rs1,rs2,label) _V_BLTU_(rs1,rs2,label)
#define v_bltii(rs1,rs2,label) _V_BLTII_(rs1,rs2,label)
#define v_bltui(rs1,rs2,label) _V_BLTUI_(rs1,rs2,label)
#define v_bltli(rs1,rs2,label) _V_BLTII_(rs1,rs2,label)
#define v_bltuli(rs1,rs2,label) _V_BLTUI_(rs1,rs2,label)
#define v_bltpi(rs1,rs2,label) _V_BLTUI_(rs1,(int )rs2,label)
BRANCH(BLT,__NAE,U)
BRANCHI(BLT,__NAE,U)
BRANCH(BLT,__LT,I)
BRANCHI(BLT,__LT,I)

#define v_blei(rs1,rs2,label) _V_BLEI_(rs1,rs2,label)
#define v_bleu(rs1,rs2,label) _V_BLEU_(rs1,rs2,label)
#define v_blel(rs1,rs2,label) _V_BLEI_(rs1,rs2,label)
#define v_bleul(rs1,rs2,label) _V_BLEU_(rs1,rs2,label)
#define v_blep(rs1,rs2,label) _V_BLEU_(rs1,rs2,label)
#define v_bleii(rs1,rs2,label) _V_BLEII_(rs1,rs2,label)
#define v_bleui(rs1,rs2,label) _V_BLEUI_(rs1,rs2,label)
#define v_bleli(rs1,rs2,label) _V_BLEII_(rs1,rs2,label)
#define v_bleuli(rs1,rs2,label) _V_BLEUI_(rs1,rs2,label)
#define v_blepi(rs1,rs2,label) _V_BLEUI_(rs1,(int )rs2,label)
BRANCH(BLE,__BE,U)
BRANCHI(BLE,__BE,U)
BRANCH(BLE,__LE,I)
BRANCHI(BLE,__LE,I)

#define v_bgei(rs1,rs2,label) _V_BGEI_(rs1,rs2,label)
#define v_bgeu(rs1,rs2,label) _V_BGEU_(rs1,rs2,label)
#define v_bgel(rs1,rs2,label) _V_BGEI_(rs1,rs2,label)
#define v_bgeul(rs1,rs2,label) _V_BGEU_(rs1,rs2,label)
#define v_bgep(rs1,rs2,label) _V_BGEU_(rs1,rs2,label)
#define v_bgeii(rs1,rs2,label) _V_BGEII_(rs1,rs2,label)
#define v_bgeui(rs1,rs2,label) _V_BGEUI_(rs1,rs2,label)
#define v_bgeli(rs1,rs2,label) _V_BGEII_(rs1,rs2,label)
#define v_bgeuli(rs1,rs2,label) _V_BGEUI_(rs1,rs2,label)
#define v_bgepi(rs1,rs2,label) _V_BGEUI_(rs1,(int )rs2,label)
BRANCH(BGE,__AE,U)
BRANCHI(BGE,__AE,U)
BRANCH(BGE,__GE,I)
BRANCHI(BGE,__GE,I)

#define v_bgti(rs1,rs2,label) _V_BGTI_(rs1,rs2,label)
#define v_bgtu(rs1,rs2,label) _V_BGTU_(rs1,rs2,label)
#define v_bgtl(rs1,rs2,label) _V_BGTI_(rs1,rs2,label)
#define v_bgtul(rs1,rs2,label) _V_BGTU_(rs1,rs2,label)
#define v_bgtp(rs1,rs2,label) _V_BGTU_(rs1,rs2,label)
#define v_bgtii(rs1,rs2,label) _V_BGTII_(rs1,rs2,label)
#define v_bgtui(rs1,rs2,label) _V_BGTUI_(rs1,rs2,label)
#define v_bgtli(rs1,rs2,label) _V_BGTII_(rs1,rs2,label)
#define v_bgtuli(rs1,rs2,label) _V_BGTUI_(rs1,rs2,label)
#define v_bgtpi(rs1,rs2,label) _V_BGTUI_(rs1,(int )rs2,label)
BRANCH(BGT,__NBE,U)
BRANCHI(BGT,__NBE,U)
BRANCH(BGT,__GT,I)
BRANCHI(BGT,__GT,I)

#define v_beqi(rs1,rs2,label) _V_BEQU_(rs1,rs2,label)
#define v_bequ(rs1,rs2,label) _V_BEQU_(rs1,rs2,label)
#define v_beql(rs1,rs2,label) _V_BEQU_(rs1,rs2,label)
#define v_bequl(rs1,rs2,label) _V_BEQU_(rs1,rs2,label)
#define v_beqp(rs1,rs2,label) _V_BEQU_(rs1,rs2,label)
#define v_beqii(rs1,rs2,label) _V_BEQUI_(rs1,rs2,label)
#define v_bequi(rs1,rs2,label) _V_BEQUI_(rs1,rs2,label)
#define v_beqli(rs1,rs2,label) _V_BEQUI_(rs1,rs2,label)
#define v_bequli(rs1,rs2,label) _V_BEQUI_(rs1,rs2,label)
#define v_beqpi(rs1,rs2,label) _V_BEQUI_(rs1,(int )rs2,label)
BRANCH(BEQ,__EQ,U)
BRANCHI(BEQ,__EQ,U)

#define v_bnei(rs1,rs2,label) _V_BNEU_(rs1,rs2,label)
#define v_bneu(rs1,rs2,label) _V_BNEU_(rs1,rs2,label)
#define v_bnel(rs1,rs2,label) _V_BNEU_(rs1,rs2,label)
#define v_bneul(rs1,rs2,label) _V_BNEU_(rs1,rs2,label)
#define v_bnep(rs1,rs2,label) _V_BNEU_(rs1,rs2,label)
#define v_bneii(rs1,rs2,label) _V_BNEUI_(rs1,rs2,label)
#define v_bneui(rs1,rs2,label) _V_BNEUI_(rs1,rs2,label)
#define v_bneli(rs1,rs2,label) _V_BNEUI_(rs1,rs2,label)
#define v_bneuli(rs1,rs2,label) _V_BNEUI_(rs1,rs2,label)
#define v_bnepi(rs1,rs2,label) _V_BNEUI_(rs1,(int )rs2,label)
BRANCH(BNE,__NE,U)
BRANCHI(BNE,__NE,U)

#define v_alduci(rd,rs,offset,alignment) v_lduci(rd,rs,offset)
#define v_aldusi(rd,rs,offset,alignment) v_ldusi(rd,rs,offset)
#define v_aldui(rd,rs,offset,alignment) v_ldui(rd,rs,offset)

#define v_ldi(rd,rs,offset) _V_LDU_(rd,rs,offset)
#define v_ldu(rd,rs,offset) _V_LDU_(rd,rs,offset)
#define v_ldl(rd,rs,offset) _V_LDU_(rd,rs,offset)
#define v_ldul(rd,rs,offset) _V_LDU_(rd,rs,offset)
#define v_ldp(rd,rs,offset) _V_LDU_(rd,rs,offset)
#define v_lds(rd,rs,offset) _V_LDUS_(rd,rs,offset)
#define v_ldus(rd,rs,offset) _V_LDUS_(rd,rs,offset)
#define v_ldc(rd,rs,offset) _V_LDUC_(rd,rs,offset)
#define v_lduc(rd,rs,offset) _V_LDUC_(rd,rs,offset)
#define v_ldii(rd,rs,offset) _V_LDUI_(rd,rs,offset)
#define v_ldui(rd,rs,offset) _V_LDUI_(rd,rs,offset)
#define v_ldli(rd,rs,offset) _V_LDUI_(rd,rs,offset)
#define v_lduli(rd,rs,offset) _V_LDUI_(rd,rs,offset)
#define v_ldpi(rd,rs,offset) _V_LDUI_(rd,rs,(int )offset)
#define v_ldsi(rd,rs,offset) _V_LDUSI_(rd,rs,(int )offset)
#define v_ldusi(rd,rs,offset) _V_LDUSI_(rd,rs,(int )offset)
#define v_ldci(rd,rs,offset) _V_LDUCI_(rd,rs,(int )offset)
#define v_lduci(rd,rs,offset) _V_LDUCI_(rd,rs,(int )offset)
LDST(LD,MOVRPlusR2R,MOVRPlusZero2R,U)
LDSTI(LD,MOVRPlusI2R,MOVIPlusZero2R,U)
LDST(LD,MOVRPlusR2R,MOVRPlusZero2R,US)
LDSTI(LD,MOVRPlusI2R,MOVIPlusZero2R,US)
LDST(LD,MOVRPlusR2R,MOVRPlusZero2R, UC)
LDSTI(LD,MOVRPlusI2R,MOVIPlusZero2R, UC)

#define v_sti(rd,rs,offset) _V_STU_(rd,rs,offset)
#define v_stu(rd,rs,offset) _V_STU_(rd,rs,offset)
#define v_stl(rd,rs,offset) _V_STU_(rd,rs,offset)
#define v_stul(rd,rs,offset) _V_STU_(rd,rs,offset)
#define v_stp(rd,rs,offset) _V_STU_(rd,rs,offset)
#define v_sts(rd,rs,offset) _V_STUS_(rd,rs,offset)
#define v_stus(rd,rs,offset) _V_STUS_(rd,rs,offset)
#define v_stc(rd,rs,offset) _V_STUC_(rd,rs,offset)
#define v_stuc(rd,rs,offset) _V_STUC_(rd,rs,offset)
#define v_stii(rd,rs,offset) _V_STUI_(rd,rs,offset)
#define v_stui(rd,rs,offset) _V_STUI_(rd,rs,offset)
#define v_stli(rd,rs,offset) _V_STUI_(rd,rs,offset)
#define v_stuli(rd,rs,offset) _V_STUI_(rd,rs,offset)
#define v_stpi(rd,rs,offset) _V_STUI_(rd,rs,(int )offset)
#define v_stsi(rd,rs,offset) _V_STUSI_(rd,rs,(int )offset)
#define v_stusi(rd,rs,offset) _V_STUSI_(rd,rs,(int )offset)
#define v_stci(rd,rs,offset) _V_STUCI_(rd,rs,(int )offset)
#define v_stuci(rd,rs,offset) _V_STUCI_(rd,rs,(int )offset)
LDST(ST,MOVR2RPlusR,MOVR2RPlusZero,U)
LDSTI(ST,MOVR2RPlusI,MOVR2IPlusZero,U)
LDST(ST,MOVR2RPlusR,MOVR2RPlusZero,US)
LDSTI(ST,MOVR2RPlusI,MOVR2IPlusZero,US)
LDST(ST,MOVR2RPlusR,MOVR2RPlusZero,UC)
LDSTI(ST,MOVR2RPlusI,MOVR2IPlusZero,UC)

#define v_jv(label) JUMPV (label)
static inline void v_jp(reg) {
  if (__NVIRT(reg)) {
    JUMPRP (__REG(reg));
  } else {
    JUMPVP (__ADDR(reg));
  }
}
#define v_jpi(imm) JUMPI(imm)

#define v_jalv(label) CALLV(label)
#define v_jalp(r) if (__NVIRT(r)) {CALLRP(__REG(r));} else {CALLVP(__ADDR(r));}
#define v_jalpi(imm) CALLI(imm)

#define v_retv() _V_RETV_
#define v_reti(reg) _V_RETU_(reg)
#define v_retl(reg) _V_RETU_(reg)
#define v_retu(reg) _V_RETU_(reg)
#define v_retul(reg) _V_RETU_(reg)
#define v_retp(reg) _V_RETU_(reg)
#define v_retii(reg) _V_RETUI_(reg)
#define v_retli(reg) _V_RETUI_(reg)
#define v_retui(reg) _V_RETUI_(reg)
#define v_retuli(reg) _V_RETUI_(reg)
#define v_retpi(reg) _V_RETUI_((int )reg)


#define v_push_argi(c,r) push_arg_reg (c, r)
#define v_push_argu(c,r) push_arg_reg (c, r)
#define v_push_argl(c,r) push_arg_reg (c, r)
#define v_push_argul(c,r) push_arg_reg (c, r)
#define v_push_argp(c,r) push_arg_reg (c, r)

#define v_push_argii(c,i) push_arg_imm (c, i)
#define v_push_argui(c,i) push_arg_imm (c, i)
#define v_push_argli(c,i) push_arg_imm (c, i)
#define v_push_arguli(c,i) push_arg_imm (c, i)
#define v_push_argpi(c,i) push_arg_imm (c, i)

static inline void v_nop() {__NOP;}
static inline void v_nuke_nop () {}

#define v_schedule_delay(branch,delay) delay; branch
#define v_raw_load(mem,count) mem
#define v_flushcache(x,y)

#define cvi2u(x) x
#define cvi2ul(x) x
#define cvi2l(x) x

#define cvu2i(x) x
#define cvu2ul(x) x
#define cvu2l(x) x

#define cvl2i(x) x
#define cvl2u(x) x
#define cvl2ul(x) x

#define cvul2i(x) x
#define cvul2u(x) x
#define cvul2l(x) x
#define cvul2p(x) x

#define cvp2ul(x) x


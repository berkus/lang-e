#!/usr/bin/perl

# rather than turning off code segment translation in sim-* for static code
# (couldn't get this to work with sim-outorder), we emit already translated
# dynamic code.  In SS-speak, we emit SS_OP_ENUM(SS_OPCODE(opcode)) directly.
# $opcode provides this mapping. 
$opnum = 1;
while (<>) {
    if (/DEFINST\(\w+,\s*(0x\w+),/) {
	$opcode{$1} = $opnum++;
    }
}

print <<"EOF"
#include <assert.h>

#define OP(x)          (x)
#define RS(x)          ((x) << 24)
#define RT(x)          ((x) << 16)
#define RD(x)          ((x) << 8)
/*#define IMM(x)         ((int)((short)((x) & 0xffff)))*/
#define IMM(x)         (((short)(x)) & 0xffff)
#define UIMM(x)        ((x) & 0xffff)
#define TARG(x)        ((x) & 0xffffff)
#define BCODE(x)       ((x) & 0xfffff)
#define SHAMT(x)       ((x) & 0xff)

#define nop() do { \\
    (*v_ip).l = OP($opcode{"0x00"}); (*v_ip++).h = 0; \\
} while (0)
#define _nop()
    
#define Fd(op1, op2, rd)\\
    do { (*v_ip).l = OP(op1); (*v_ip++).h = RD(rd); } while (0)
#define Fds(op1, op2, rd, rs)\\
    do { (*v_ip).l = OP(op1); (*v_ip++).h = RD(rd) | RS(rs); } while (0)
#define Fdst(op1, op2, rd, rs, rt)\\
    do { (*v_ip).l = OP(op1); (*v_ip++).h = RS(rs) | RT(rt) | RD(rd); } while (0)
#define Fdti(op1, op2, rd, rt, imm)\\
    do { (*v_ip).l = OP(op1); (*v_ip++).h = RD(rd) | RT(rt) | IMM(imm); } while (0)
#define Fdts(op1, op2, rd, rt, rs)\\
    do { (*v_ip).l = OP(op1); (*v_ip++).h = RS(rs) | RT(rt) | RD(rd); } while (0)
#define Fj(op1, op2, targ) /* branches: insert nop */ \\
    do { (*v_ip).l = OP(op1); (*v_ip++).h = IMM(0); \\
	 v_jmark(v_ip, targ); _nop(); \\
     } while (0)
#define Fs(op1, op2, rs)\\
    do { (*v_ip).l = OP(op1); (*v_ip++).h = RS(rs); } while (0)
#define Fsi(op1, op2, rs, imm) /* branches: insert nop */ \\
    do { \\
	(*v_ip).l = OP(op1); (*v_ip++).h = RS(rs) | IMM(0); \\
	v_jmark(v_ip, imm); \\
	_nop(); \\
    } while (0)
#define Fst(op1, op2, rs, rt)\\
    do { (*v_ip).l = OP(op1); (*v_ip++).h = RS(rs) | RT(rt); } while (0)
#define Fsti(op1, op2, rs, rt, imm) /* branches: insert nop */\\
    do { \\
	(*v_ip).l = OP(op1); (*v_ip++).h = RS(rs) | RT(rt) | IMM(0); \\
	v_jmark(v_ip, imm); \\
	_nop(); \\
    } while (0)
#define Fti(op1, op2, rt, imm)\\
    do { (*v_ip).l = OP(op1); (*v_ip++).h = RT(rt) | IMM(imm); } while (0)
#define Fts(op1, op2, rt, rs)\\
    do { (*v_ip).l = OP(op1); (*v_ip++).h = RS(rs) | RT(rt); } while (0)
#define Ftsd(op1, op2, rt, rs, rd) /* loads/stores: insert nop */\\
    do { (*v_ip).l = OP(op1); (*v_ip++).h = RS(rs) | RT(rt) | RD(rd); _nop(); } while (0)
#define Ftsi(op1, op2, rt, rs, imm)\\
    do { (*v_ip).l = OP(op1); (*v_ip++).h = RS(rs) | RT(rt) | IMM(imm); } while (0)
#define FtsI(op1, op2, rt, rs, imm)\\
    do {							\\
	unsigned int __FtsI_rt = rt;				\\
	unsigned int __FtsI_rs = rs;				\\
	long __FtsI_imm = imm;					\\
	if (is16bit(__FtsI_imm)) {				\\
	    Ftsi(op1, op1, __FtsI_rt, __FtsI_rs, __FtsI_imm);	\\
	} else {						\\
	    noc_set(_at, __FtsI_imm);				\\
	    Fdst(op2, op2, __FtsI_rt, __FtsI_rs, _at);		\\
	}							\\
    } while(0)
#define FtsO(op1, op2, rt, rs, imm)\\
    /* like FtsI, but uses Ftsd (for loads/stores) instead of Fdst */ \\
    do {							\\
	unsigned int __FtsI_rt = rt;				\\
	unsigned int __FtsI_rs = rs;				\\
	long __FtsI_imm = imm;					\\
	if (is16bit(__FtsI_imm)) {				\\
	    Ftsi(op1, op1, __FtsI_rt, __FtsI_rs, __FtsI_imm);	\\
	    _nop();						\\
	} else {						\\
	    noc_set(_at, __FtsI_imm);				\\
	    Ftsd(op2, op2, __FtsI_rt, __FtsI_rs, _at);		\\
	}							\\
    } while(0)
#define Ftsu(op1, op2, rt, rs, uimm)\\
    do { (*v_ip).l = OP(op1); (*v_ip++).h = RS(rs) | RT(rt) | UIMM(uimm); } while (0)
#define FtsU(op1, op2, rt, rs, uimm)\\
    do {							\\
	unsigned int __FtsU_rt = rt;				\\
	unsigned int __FtsU_rs = rs;				\\
	unsigned long __FtsU_uimm = uimm;			\\
	if (isu16bit(__FtsU_uimm)) {				\\
	    Ftsu(op1, op1, __FtsU_rt, __FtsU_rs, __FtsU_uimm);	\\
	} else {						\\
	    noc_setu(_at, __FtsU_uimm);				\\
	    Fdst(op2, op2, __FtsU_rt, __FtsU_rs, _at);		\\
	}							\\
    } while(0)
EOF
    ;

@ops = (
	[ "jr1",	"0x03",	"0x03",		"s" ], # see jr below
	[ "jalr1",	"0x04",	"0x04",		"ds" ], # see jalr below

	[ "lb_rr",	"0xc0",	"0xc0",		"tsd" ],
	[ "lb",		"0x20",	"0xc0",		"tsO" ],
	[ "lbu_rr",	"0xc1",	"0xc1",		"tsd" ],
	[ "lbu",	"0x22",	"0xc1",		"tsO" ],
	[ "lh_rr",	"0xc2",	"0xc2",		"tsd" ],
	[ "lh",		"0x24",	"0xc2",		"tsO" ],
	[ "lhu_rr",	"0xc3",	"0xc3",		"tsd" ],
	[ "lhu",	"0x26",	"0xc3",		"tsO" ],
	[ "lw_rr",	"0xc4",	"0xc4",		"tsd" ],
	[ "lw",		"0x28",	"0xc4",		"tsO" ],
	[ "l_s_rr",	"0xc5",	"0xc5",		"tsd" ],
	[ "l_s",	"0x2a",	"0xc5",		"tsO" ],
	[ "l_d_rr",	"0xcf",	"0xcf",		"tsd" ],
	[ "l_d",	"0x2b",	"0xcf",		"tsO" ],

	[ "sb_rr",	"0xc6",	"0xc6",		"tsd" ],
	[ "sb",		"0x30",	"0xc6",		"tsO" ],
	[ "sh_rr",	"0xc7",	"0xc7",		"tsd" ],
	[ "sh",		"0x32",	"0xc7",		"tsO" ],
	[ "sw_rr",	"0xc8",	"0xc8",		"tsd" ],
	[ "sw",		"0x34",	"0xc8",		"tsO" ],
	[ "s_s_rr",	"0xc9",	"0xc9",		"tsd" ],
	[ "s_s",	"0x36",	"0xc9",		"tsO" ],
	[ "s_d_rr",	"0xd2",	"0xd2",		"tsd" ],
	[ "s_d",	"0x37",	"0xd2",		"tsO" ],

	[ "sllv",	"0x56",	"0x56",		"dts" ],
	[ "sll",	"0x55",	"0x55",		"dti" ],
	[ "srav",	"0x5a",	"0x5a",		"dts" ],
	[ "sra",	"0x59",	"0x59",		"dti" ],
	[ "srlv",	"0x58",	"0x58",		"dts" ],
	[ "srl",	"0x57",	"0x57",		"dti" ],

	[ "and",	"0x4e",	"0x4e",		"dst" ],
	[ "andi",	"0x4f",	"0x4e",		"tsU" ],
	[ "or",		"0x50",	"0x50",		"dst" ],
	[ "ori",	"0x51",	"0x50",		"tsU" ],
	[ "noc_ori",	"0x51",	"0x51",		"tsu" ],
	[ "xor",	"0x52",	"0x52",		"dst" ],
	[ "xori",	"0x53",	"0x52",		"tsU" ],
	[ "nor",	"0x54",	"0x54",		"dst" ],

	[ "add",	"0x40",	"0x40",		"dst" ],
	[ "addi",	"0x41",	"0x40",		"tsI" ],
	[ "addu",	"0x42",	"0x42",		"dst" ],
	[ "addiu",	"0x43",	"0x42",		"tsI" ],
	[ "noc_addiu",	"0x43",	"0x43",		"tsi" ],
	[ "fadd_s",	"0x70",	"0x70",		"dst" ],
	[ "fadd_d",	"0x71",	"0x71",		"dst" ],
	[ "sub",	"0x44",	"0x44",		"dst" ],
	[ "subu",	"0x45",	"0x45",		"dst" ],
	[ "fsub_s",	"0x72",	"0x72",		"dst" ],
	[ "fsub_d",	"0x73",	"0x73",		"dst" ],
	[ "mult",	"0x46",	"0x46",		"st" ],
	[ "multu",	"0x47",	"0x47",		"st" ],
	[ "fmul_s",	"0x74",	"0x74",		"dst" ],
	[ "fmul_d",	"0x75",	"0x75",		"dst" ],
	[ "vdiv",	"0x48",	"0x48",		"st" ],
	[ "vdivu",	"0x49",	"0x49",		"st" ],
	[ "fdiv_s",	"0x76",	"0x76",		"dst" ],
	[ "fdiv_d",	"0x77",	"0x77",		"dst" ],
	[ "mflo",	"0x4c",	"0x4c",		"d" ],# needed for "mul", "div"
	[ "mfhi",	"0x4a",	"0x4a",		"d" ], # needed for "mod"
	[ "fneg_s",	"0x7c",	"0x7c",		"ds" ],
	[ "fneg_d",	"0x7d",	"0x7d",		"ds" ],

	[ "fmov_s",	"0x7a",	"0x7a",		"ds" ],
	[ "fmov_d",	"0x7b",	"0x7b",		"ds" ],
	[ "lui",	"0xa2",	"0xa2",		"ti" ],	# needed for "set"

	[ "beq",	"0x05",	"0x05",		"sti" ],
	[ "bne",	"0x06",	"0x06",		"sti" ],
	[ "blez",	"0x07",	"0x07",		"si" ],
	[ "bgtz",	"0x08",	"0x08",		"si" ],
	[ "bltz",	"0x09",	"0x09",		"si" ],
	[ "bgez",	"0x0a",	"0x0a",		"si" ],

	[ "slt",	"0x5b",	"0x5b",		"dst" ],
	[ "slti",	"0x5c",	"0x5b",		"tsI" ],
	[ "sltu",	"0x5d",	"0x5d",		"dst" ],
	[ "sltiu",	"0x5e",	"0x5d",		"tsI" ], # needed for "neg"

	[ "fabs_s",	"0x78",	"0x78",		"ds" ],
	[ "fabs_d",	"0x79",	"0x79",		"ds" ],
	[ "fsqrt_s",	"0x96",	"0x96",		"ds" ],
	[ "fsqrt_d",	"0x97",	"0x97",		"ds" ],

	[ "cvt_s_d",	"0x80",	"0x80",		"ds" ],
	[ "cvt_s_w",	"0x81",	"0x81",		"ds" ],
	[ "cvt_d_s",	"0x82",	"0x82",		"ds" ],
	[ "cvt_d_w",	"0x83",	"0x83",		"ds" ],
	[ "cvt_w_s",	"0x84",	"0x84",		"ds" ],
	[ "cvt_w_d",	"0x85",	"0x85",		"ds" ],
	[ "mtc1",	"0xa5",	"0xa5",		"ts" ],
	[ "mfc1",	"0xa3",	"0xa3",		"ts" ],

	[ "bc1f",	"0x0b",	"0x0b",		"j" ],
	[ "bc1t",	"0x0c",	"0x0c",		"j" ],

	[ "c_eq_s",	"0x90",	"0x90",		"st" ],
	[ "c_eq_d",	"0x91",	"0x91",		"st" ],
	[ "c_lt_s",	"0x92",	"0x92",		"st" ],
	[ "c_lt_d",	"0x93",	"0x93",		"st" ],
	[ "c_le_s",	"0x94",	"0x94",		"st" ],
	[ "c_le_d",	"0x95",	"0x95",		"st" ],
	);

foreach $op (@ops) {
    $opname = ${$op}[0];
    $op1 = $opcode{${$op}[1]};
    $op2 = $opcode{${$op}[2]};
    $optype = ${$op}[3];
    $l = length $optype;
    if ($l == 1) {
	print "#define $opname(a) F$optype($op1,$op2,a)\n";
    } elsif ($l == 2) {
	print "#define $opname(a,b) F$optype($op1,$op2,a,b)\n";
    } elsif ($l == 3) {
	print "#define $opname(a,b,c) F$optype($op1,$op2,a,b,c)\n";
    }
}

print <<"EOF"
#define jump(targ)\\
    do { unsigned int __jump_targ = (targ);			\\
	 assert((__jump_targ & 0x3) == 0);			\\
         if (isu26bit(__jump_targ)) {				\\
	     (*v_ip).l = OP($opcode{"0x01"});			\\
             (*v_ip++).h = TARG(__jump_targ>>2);		\\
         } else {						\\
	    noc_set(_at,__jump_targ);				\\
	    jr1(_at);						\\
	 }							\\
     } while (0)
#define jal(targ)\\
    do { unsigned int __jump_targ = (targ);			\\
	 assert((__jump_targ & 0x3) == 0);			\\
         if (isu26bit(__jump_targ)) {				\\
	     (*v_ip).l = OP($opcode{"0x02"});			\\
	     (*v_ip++).h = TARG(__jump_targ>>2);		\\
         } else {						\\
	    noc_set(_at,__jump_targ);				\\
	    jalr1(_ra,_at);					\\
	 }							\\
     } while (0)

#define jr(s) do { jr1(s); _nop(); } while (0)
#define jalr(d,s) do { jalr1(d,s); _nop(); } while (0)

/* snarfed shamelessly from binary.h for mips */

#define beqi(rs, im, l) do {		\\
    if(!im) beq(rs, _r0, l);		\\
    else { set(_at, im); beq(rs, _at, l); }		\\
} while(0)

#define bnei(rs, im, l) do {		\\
    if(!im) bne(rs, _r0, l);		\\
    else { set(_at, im); bne(rs, _at, l); }		\\
} while(0)

#define blt(a, b, name) do {		\\
	if(b == _r0) 	 bltz(a,name);		\\
	else if(a == _r0) bgez(b,name);		\\
	else { slt(_at, a, b); bne(_at, _r0, name); }			\\
} while(0)

#define blti(a, b, name) do {	\\
	if(b == 0) bltz(a,name);	\\
	else { slti(_at, a, b); bne(_at, _r0, name); }		\\
} while(0)
 
#define ble(a, b, name) do {		\\
	if(b == _r0) 	 blez(a,name);		\\
	else if(a == _r0) bgtz(b,name);		\\
	else { slt(_at, b, a); beq(_at, _r0, name); }			\\
} while(0)

#define blei(a, b, name) do {	\\
	if(b == 0) blez(a,name);	\\
	else { blti(a,(b+1),name); }		\\
} while(0)
 
#define bgt(a, b, name) do {		\\
	if(b == _r0) 	 bgtz(a,name);		\\
	else if(a == _r0) blez(b,name);		\\
	else { slt(_at, b, a);  bne(_at, _r0, name); }			\\
} while(0)

#define bgti(a, b, name) do {	\\
	if(b == 0) bgtz(a,name);	\\
	else { bgei(a,(b+1),name); }		\\
} while(0)
 
#define bge(a, b, name) do {		\\
	if(b == _r0) 	 bgez(a,name);		\\
	else if(a == _r0) bltz(b,name);		\\
	else { slt(_at, a, b);  beq(_at, _r0, name); }			\\
} while(0)

#define bgei(a, b, name) do {	\\
	if(b == 0) bgez(a,name);	\\
	else { slti(_at, a, b); beq(_at, _r0, name); }		\\
} while(0)
 
#define bltu(a, b, name) do {		\\
	if(b == _r0) 	 demand(0, lt zero always false);		\\
	else if(a == _r0) bne(b, _r0, name);		\\
	else { sltu(_at, a, b); bne(_at, _r0, name); }			\\
} while(0)

#define bltui(a, b, name) do {	\\
	if(b == 0) demand(0, lt zero always false);	\\
	else { sltiu(_at, a, b); bne(_at, _r0, name); }		\\
} while(0)
 
#define bleu(a, b, name) do {		\\
	if(b == _r0) 	 beq(a, _r0, name);		\\
	else if(a == _r0) demand(0, gte zero always true);		\\
	else { sltu(_at, b, a); beq(_at, _r0, name); }			\\
} while(0)

#define bleui(a, b, name) do {	\\
	if(b == 0) beq(a, _r0, name);	\\
	else { bltui(a,(b+1),name); }		\\
} while(0)
 
#define bgtu(a, b, name) do {		\\
	if(b == _r0) 	 bne(a, _r0, name);		\\
	else if(a == _r0) demand(0, lt zero always true);		\\
	else { sltu(_at, b, a); bne(_at, _r0, name); }			\\
} while(0)

#define bgtui(a, b, name) do {	\\
	if(b == 0) bne(a, _r0, name);	\\
	else { bgeui(a,(b+1),name); }		\\
} while(0)
 
#define bgeu(a, b, name) do {		\\
	if(b == _r0) 	 demand(0, gte zero always true);		\\
	else if(a == _r0) beq(b, 0, name);		\\
	else { sltu(_at, a, b); beq(_at, _r0, name); }			\\
} while(0)

#define bgeui(a, b, name) do {	\\
	if(b == 0) demand(0, gte zero always true);	\\
	else { sltiu(_at,a,b); beq(_at, _r0, name); }		\\
} while(0)

#define sd s_d
#define ld l_d

#define movi2f(dst, src) mtc1(dst, src)
#define movf2i(dst, src) mfc1(dst, src)
#define movi2d(dst, src) do {                   \\
       	unsigned _src = src, _dst = dst;        \\
       	mtc1(_dst, _src);                       \\
       	mtc1(_dst+1, _src+1);                   \\
} while(0)

#define movd2i(dst, src) do {                   \\
       	unsigned _src = src, _dst = dst;        \\
       	mfc1(_dst, _src);                       \\
       	mfc1(_dst+1, _src+1);                   \\
} while(0)

#define mov(d,s) do { if((d)!=(s)) addu(d, s, _zero); } while (0)

#define hi(x)	(((unsigned)(x)) >> 16)
#define lo(x)	(((unsigned)(x)) & 0xffff)
#define is16bit(imm)    ((int)(imm) < 0x7fff && (int)(imm) >= -0x7fff)
#define isu16bit(imm)   ((unsigned)(imm) <= 0xffff)

#define is16bit(imm)    ((int)(imm) < 0x7fff && (int)(imm) >= -0x7fff)
#define isu16bit(imm)   ((unsigned)(imm) <= 0xffff)
#define isu26bit(imm)	((unsigned)(imm) <= 0x3ffffff)

#define noc_setu 	noc_set
#define noc_set(dst, simm) do {			\\
	int sim = simm;				\\
	lui(dst, hi(sim));			\\
        if(lo(sim)) 				\\
                noc_ori(dst, dst, lo(sim));	\\
} while(0)

#define set(dst, simm) 	do {			\\
	int sim = simm;				\\
        if(is16bit(sim)) {			\\
		noc_addiu(dst, _r0, sim);	\\
	} else {				\\
		lui(dst, hi(sim));		\\
        	if(lo(sim)) 			\\
                	noc_ori(dst, dst, lo(sim));\\
        }					\\
} while(0)

#define setu(dst, im) do {				\\
	unsigned uim = im;				\\
        if(isu16bit(uim)) {				\\
		 noc_ori(dst, _r0, uim);		\\
	} else {					\\
                lui(dst, hi(uim));			\\
        	if(lo(uim)) 				\\
                	noc_ori(dst, dst, lo(uim));	\\
        }						\\
} while(0)

EOF
;

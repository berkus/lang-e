%{
#define INTTMP 0xff800000
#define INTVAR 0x007fffff
#define FLTTMP 0xff800000
#define FLTVAR 0x007fffff

#define isafv(op) ((op) == ADDRGP || (op) == ADRFFP || (op) == ADRFLP)

enum { VR_VAR=0, VR_TMP };

#include "c.h"

#define NODEPTR_TYPE Node
#define OP_LABEL(p) ((p)->op)
#define LEFT_CHILD(p) ((p)->kids[0])
#define RIGHT_CHILD(p) ((p)->kids[1])
#define STATE_LABEL(p) ((p)->x.state)

static void address     ARGS((Symbol, Symbol, int));
static void blkfetch    ARGS((int, int, int, int));
static void blkloop     ARGS((int, int, int, int, int, int[]));
static void blkstore    ARGS((int, int, int, int));
static void defaddress  ARGS((Symbol));
static void defconst    ARGS((int, Value));
static void defstring   ARGS((int, char *));
static void defsymbol   ARGS((Symbol));
static void doarg       ARGS((Node));
static void emit2       ARGS((Node));
static void export      ARGS((Symbol));
static void clobber     ARGS((Node));
static void function    ARGS((Symbol, Symbol [], Symbol [], int));
static void global      ARGS((Symbol));
static void iLocal       ARGS((Symbol));
static void import      ARGS((Symbol));
static int  isimmediate ARGS((Node));
static void progbeg     ARGS((int, char **));
static void progend     ARGS((void));
static void segment     ARGS((int));
static void space       ARGS((int));
static void target      ARGS((Node));
extern int  atoi        ARGS((char *));

static void emitclosurebeg ARGS((Code));
static void emitclosureend ARGS((Code));
static void genclosurebeg ARGS((Code));
static void genclosureend ARGS((Code));

static void mkfreevar ARGS((Symbol, void *));
static void mkvs ARGS((Symbol, void *));
static void mklcl ARGS((Symbol, void *));
static void emitcomment ARGS((char *));
static void decllabel ARGS((Code));
static void decllocal ARGS((Code));
static void initlocal ARGS((Code));
static void freelocal ARGS((Code, int, int));
static void declstatic ARGS((Symbol, void *));
static void declfreevar ARGS((Symbol, void *));
static void declspec ARGS((Symbol, void *));
static void decllcl ARGS((Symbol, void *));
static void declrtc ARGS((Symbol, void *));
static void defesymaddr ARGS((Symbol, void *));
static void defesymval ARGS((Symbol, void *));
static void defesymcnst ARGS((Symbol, void *));
static void defesymstr ARGS((Symbol, void *));
static void undefesym ARGS((Symbol, void *));

static int callinit = 0;
static int unique = 0;

static Symbol sreg[3];
static Symbol blkreg; 
static Symbol ireg[32], freg[32];
static int tmpregs[] = {3, 9, 10};

static Symbol   oldrmap[16];
static unsigned oldtmask[2];
static unsigned oldvmask[2];

Symbol tr;

extern unsigned (*emitter) ARGS((Node, int)) ;

%}
%start stmt
%term ADDD=306 ADDF=305 ADDI=309 ADDP=311 ADDU=310
%term ADDRFP=279
%term ADDRGP=263
%term ADDRLP=295
%term ARGB=41 ARGD=34 ARGF=33 ARGI=37 ARGP=39 ARGV=40
%term ASGNB=57 ASGNC=51 ASGND=50 ASGNF=49 ASGNI=53 ASGNP=55 ASGNS=52
%term BANDU=390
%term BCOMU=406
%term BORU=422
%term BXORU=438
%term CALLB=217 CALLD=210 CALLF=209 CALLI=213 CALLV=216
%term CNSTC=19 CNSTD=18 CNSTF=17 CNSTI=21 CNSTP=23 CNSTS=20 CNSTU=22
%term CVCI=85 CVCU=86
%term CVDF=97 CVDI=101
%term CVFD=114
%term CVIC=131 CVID=130 CVIS=132 CVIU=134
%term CVPU=150
%term CVSI=165 CVSU=166
%term CVUC=179 CVUI=181 CVUP=183 CVUS=180
%term DIVD=450 DIVF=449 DIVI=453 DIVU=454
%term EQD=482 EQF=481 EQI=485
%term GED=498 GEF=497 GEI=501 GEU=502
%term GTD=514 GTF=513 GTI=517 GTU=518
%term INDIRB=73 INDIRC=67 INDIRD=66 INDIRF=65 INDIRI=69 INDIRP=71 INDIRS=68
%term JUMPV=584
%term LABELV=600
%term LED=530 LEF=529 LEI=533 LEU=534
%term LOADB=233 LOADC=227 LOADD=226 LOADF=225 LOADI=229 LOADP=231 LOADS=228 LOADU=230
%term LSHI=341 LSHU=342
%term LTD=546 LTF=545 LTI=549 LTU=550
%term MODI=357 MODU=358
%term MULD=466 MULF=465 MULI=469 MULU=470
%term NED=562 NEF=561 NEI=565
%term NEGD=194 NEGF=193 NEGI=197
%term RETD=242 RETF=241 RETI=245
%term RSHI=373 RSHU=374
%term SUBD=322 SUBF=321 SUBI=325 SUBP=327 SUBU=326
%term ICSF=721 ICSD=722 ICSC=723 ICSS=724
%term ICSI=725 ICSU=726 ICSP=727 ICSV=728 ICSB=729 
%term RTCF=737 RTCD=738 RTCC=739 RTCS=740
%term RTCI=741 RTCU=742 RTCP=743 RTCB=745
%term CRETF=753 CRETD=754 CRETC=755 CRETS=756
%term CRETI=757 CRETU=758 CRETP=759 CRETB=761
%term ADRFFP=775 ADRFLP=791
%term GETREG=800 PUTREG=816 NOTE=832
%term KLABELV=872 KTEST=880 KJUMPV=904
%term VREGP=967
%term FASGN=976 ADRVS=992
%term DJUMP=1008 SELFP=1031
%%

reg:  INDIRC(VREGP)     "# read register\n"
reg:  INDIRD(VREGP)     "# read register\n"
reg:  INDIRF(VREGP)     "# read register\n"
reg:  INDIRI(VREGP)     "# read register\n"
reg:  INDIRP(VREGP)     "# read register\n"
reg:  INDIRS(VREGP)     "# read register\n"
stmt: ASGNC(VREGP,reg)  "# write register\n"
stmt: ASGND(VREGP,reg)  "# write register\n"
stmt: ASGNF(VREGP,reg)  "# write register\n"
stmt: ASGNI(VREGP,reg)  "# write register\n"
stmt: ASGNP(VREGP,reg)  "# write register\n"
stmt: ASGNS(VREGP,reg)  "# write register\n"

stmt: GETREG "#getreg\n"
stmt: PUTREG "#putreg\n"
stmt: NOTE   "%a\n"

rtcb: RTCB "#rname"	    0
rtcc: RTCC "#rname"	    0
rtcd: RTCD "#rname"	    0
rtcf: RTCF "#rname"	    0
rtci: RTCI "#rname"	    0
rtcs: RTCS "#rname"	    0
rtcu: RTCU "#rname"	    0
rtcp: RTCP "#rname"	    0

rtc: rtcc "%0"
rtc: rtcd "%0"
rtc: rtcf "%0"
rtc: rtci "%0"
rtc: rtcs "%0"
rtc: rtcu "%0"
rtc: rtcp "%0"

ics: ICSF	"#call a cspec cgf"
ics: ICSD	"#call a cspec cgf"
ics: ICSC	"#call a cspec cgf"
ics: ICSS	"#call a cspec cgf"
ics: ICSI	"#call a cspec cgf"
ics: ICSU	"#call a cspec cgf"
ics: ICSP	"#call a cspec cgf"
ics: ICSB	"#call a cspec cgf"

afv: ADDRGP    "__DRGP__ %a"	0
afv: ADRFFP    "__RFFP__ %a"	0
afv: ADRFLP    "__RFLP__ %a"	0

alv: ADDRFP    "__DRFP__ %a"	0
alv: ADDRLP    "__DRLP__ %a"	0

reg: alv       "v_addpi(%c,v_lp,%0);\n"	1
reg: ADRVS     "v_addpi(%c,v_lp,_tc_offset(%a));\n" 1

reg: SELFP     "v_setp(%c,(void *)_tc_cp);\n"	0

immc: CNSTC    "%a"	0
immi: CNSTI    "%a"	0
imms: CNSTS    "%a"	0
immu: CNSTU    "%a"	0
immp: CNSTP    "%a"	0
immp: rtcp     "%0"
immp: afv      "%0"

imm: immc	"%0"
imm: immi	"%0"
imm: imms	"%0"
imm: immu	"%0"
imm: immp	"%0"
imm: rtc	"%0"

stmt: reg	"%0"
stmt: ICSV	"# call a void cspec cgf\n"

reg: ics   "%c = %0\n"
reg: immc  "v_setc(%c,%0);\n"		1
reg: immi  "v_seti(%c,%0);\n"		1
reg: imms  "v_sets(%c,%0);\n"		1
reg: immu  "v_setu(%c,%0);\n"		1
reg: immp  "v_setp(%c,(long)%0);\n"	1

reg: rtcb  "v_setp(%c,(long)&%0);\n"	1
reg: rtcc  "v_setc(%c,%0);\n"       	1
reg: rtcf  "v_setf(%c,%0);\n"       	1
reg: rtcd  "v_setd(%c,%0);\n"       	1
reg: rtci  "v_seti(%c,%0);\n"       	1
reg: rtcp  "v_setp(%c,%0);\n"       	1
reg: rtcs  "v_sets(%c,%0);\n"       	1
reg: rtcu  "v_setu(%c,%0);\n"       	1

stmt: ASGNC(ADRVS,reg)	 "# write vspec;\n" 1
stmt: ASGNS(ADRVS,reg)	 "# write vspec;\n" 1
stmt: ASGNI(ADRVS,reg)	 "# write vspec;\n" 1
stmt: ASGNP(ADRVS,reg)	 "# write vspec;\n" 1
stmt: ASGND(ADRVS,reg)	 "# write vspec;\n" 1
stmt: ASGNF(ADRVS,reg)	 "# write vspec;\n" 1
reg: INDIRC(ADRVS)	 "# read vspec;\n" 1
reg: INDIRS(ADRVS)	 "# read vspec;\n" 1
reg: INDIRI(ADRVS)	 "# read vspec;\n" 1
reg: INDIRP(ADRVS)	 "# read vspec;\n" 1
reg: INDIRD(ADRVS)	 "# read vspec;\n" 1
reg: INDIRF(ADRVS)	 "# read vspec;\n" 1

stmt: FASGN(alv,reg)     "#;\n" 0

addri: afv		 "v_zero,(long)%0"	1
addri: alv		 "v_lp,(long)%0"	1
addri: ADDI(reg, imm)	 "%0, (long)%1"		1
addri: ADDP(reg, imm)	 "%0, (long)%1"		1
addri: ADDU(reg, imm)	 "%0, (long)%1"		1
stmt: ASGNC(addri,reg)	 "v_stci(%1,%0);\n" 1
stmt: ASGNS(addri,reg)	 "v_stsi(%1,%0);\n" 1
stmt: ASGNI(addri,reg)	 "v_stii(%1,%0);\n" 1
stmt: ASGNP(addri,reg)	 "v_stpi(%1,%0);\n" 1
stmt: ASGND(addri,reg)	 "v_stdi(%1,%0);\n" 1
stmt: ASGNF(addri,reg)	 "v_stfi(%1,%0);\n" 1
reg: INDIRC(addri)	 "v_ldci(%c,%0);\n" 1
reg: INDIRS(addri)	 "v_ldsi(%c,%0);\n" 1
reg: INDIRI(addri)	 "v_ldii(%c,%0);\n" 1
reg: INDIRP(addri)	 "v_ldpi(%c,%0);\n" 1
reg: INDIRD(addri)	 "v_lddi(%c,%0);\n" 1
reg: INDIRF(addri)	 "v_ldfi(%c,%0);\n" 1
reg: CVCI(INDIRC(addri)) "v_ldci(%c,%0);\n"  1
reg: CVCU(INDIRC(addri)) "v_lduci(%c,%0);\n" 1
reg: CVSI(INDIRS(addri)) "v_ldsi(%c,%0);\n"  1
reg: CVSU(INDIRS(addri)) "v_ldusi(%c,%0);\n" 1

addr: ADDI(reg, reg)	 "%0,%1"		1
addr: ADDP(reg, reg)	 "%0,%1"		1
addr: ADDU(reg, reg)	 "%0,%1"		1
addr: reg		 "v_zero,%0"		1
stmt: ASGNC(addr,reg)	 "v_stc(%1,%0);\n"	1
stmt: ASGNS(addr,reg)	 "v_sts(%1,%0);\n"	1
stmt: ASGNI(addr,reg)	 "v_sti(%1,%0);\n"	1
stmt: ASGNP(addr,reg)	 "v_stp(%1,%0);\n"	1
stmt: ASGND(addr,reg)	 "v_std(%1,%0);\n"	1
stmt: ASGNF(addr,reg)	 "v_stf(%1,%0);\n"	1
reg: INDIRC(addr)	 "v_ldc(%c,%0);\n"	1
reg: INDIRS(addr)	 "v_lds(%c,%0);\n" 	1
reg: INDIRI(addr)	 "v_ldi(%c,%0);\n" 	1
reg: INDIRP(addr)	 "v_ldp(%c,%0);\n" 	1
reg: INDIRD(addr)	 "v_ldd(%c,%0);\n" 	1
reg: INDIRF(addr)	 "v_ldf(%c,%0);\n" 	1
reg: CVCI(INDIRC(addr))	 "v_ldc(%c,%0);\n"	1
reg: CVCU(INDIRC(addr))	 "v_lduc(%c,%0);\n"	1
reg: CVSI(INDIRS(addr))	 "v_lds(%c,%0);\n" 	1
reg: CVSU(INDIRS(addr))	 "v_ldus(%c,%0);\n" 	1

reg: DIVI(reg,reg)  "v_divi(%c,%0,%1);\n"	2
reg: DIVI(reg,imm)  "v_divii(%c,%0,%1);\n"	1
reg: DIVU(reg,reg)  "v_divu(%c,%0,%1);\n"	2
reg: DIVU(reg,imm)  "v_divui(%c,%0,%1);\n"	1
reg: MODI(reg,reg)  "v_modi(%c,%0,%1);\n"	2
reg: MODI(reg,imm)  "v_modii(%c,%0,%1);\n"	1
reg: MODU(reg,reg)  "v_modu(%c,%0,%1);\n"	2
reg: MODU(reg,imm)  "v_modui(%c,%0,%1);\n"	1
reg: MULI(reg,reg)  "v_muli(%c,%0,%1);\n"	2
reg: MULI(imm,reg)  "v_cmuli(%c,%1,%0);\n"	1
reg: MULU(reg,reg)  "v_mulu(%c,%0,%1);\n"	2
reg: MULU(imm,reg)  "v_cmulu(%c,%1,%0);\n"	1

reg: ADDI(reg,reg)  "v_addi(%c,%0,%1);\n"	2
reg: ADDI(reg,imm)  "v_addii(%c,%0,%1);\n"	1
reg: ADDP(reg,reg)  "v_addp(%c,%0,%1);\n"	2
reg: ADDP(reg,imm)  "v_addpi(%c,%0,%1);\n"	1
reg: ADDU(reg,reg)  "v_addu(%c,%0,%1);\n"	2
reg: ADDU(reg,imm)  "v_addui(%c,%0,%1);\n"	1
reg: BANDU(reg,reg) "v_andu(%c,%0,%1);\n"	2
reg: BANDU(reg,imm) "v_andui(%c,%0,%1);\n"	1
reg: BORU(reg,reg)  "v_oru(%c,%0,%1);\n"	2
reg: BORU(reg,imm)  "v_orui(%c,%0,%1);\n"	1
reg: BXORU(reg,reg) "v_xoru(%c,%0,%1);\n"	2
reg: BXORU(reg,imm) "v_xorui(%c,%0,%1);\n"	1
reg: SUBI(reg,reg)  "v_subi(%c,%0,%1);\n"	2
reg: SUBI(reg,imm)  "v_subii(%c,%0,%1);\n"	1
reg: SUBP(reg,reg)  "v_subp(%c,%0,%1);\n"	2
reg: SUBP(reg,imm)  "v_subpi(%c,%0,%1);\n"	1
reg: SUBU(reg,reg)  "v_subu(%c,%0,%1);\n"	2
reg: SUBU(reg,imm)  "v_subui(%c,%0,%1);\n"	1

reg: LSHI(reg,reg)  "v_lshi(%c,%0,%1);\n"	2
reg: LSHI(reg,imm)  "v_lshii(%c,%0,%1);\n"	1
reg: LSHU(reg,reg)  "v_lshu(%c,%0,%1);\n"	2
reg: LSHU(reg,imm)  "v_lshui(%c,%0,%1);\n"	1
reg: RSHI(reg,reg)  "v_rshi(%c,%0,%1);\n"	2
reg: RSHI(reg,imm)  "v_rshii(%c,%0,%1);\n"	1
reg: RSHU(reg,reg)  "v_rshu(%c,%0,%1);\n"	2
reg: RSHU(reg,imm)  "v_rshui(%c,%0,%1);\n"	1

reg: BCOMU(reg)     "v_comu(%c,%0);\n"		2
reg: BCOMU(imm)     "v_comui(%c,%0);\n"		1
reg: NEGI(reg)      "v_negi(%c,%0);\n"		2
reg: NEGI(imm)      "v_negii(%c,%0);\n"		1

reg: LOADC(reg)     "v_movi(%c,%0);\n"		1
reg: LOADC(imm)     "v_seti(%c,%0);\n"		1
reg: LOADS(reg)     "v_movi(%c,%0);\n"		1
reg: LOADS(imm)     "v_seti(%c,%0);\n"		1
reg: LOADP(reg)     "v_movp(%c,%0);\n"		1
reg: LOADP(imm)     "v_setp(%c,%0);\n"		1
reg: LOADI(reg)     "v_movi(%c,%0);\n"		1
reg: LOADI(imm)     "v_seti(%c,%0);\n"		1
reg: LOADU(reg)     "v_movu(%c,%0);\n"		1
reg: LOADU(imm)     "v_setu(%c,%0);\n"		1
reg: LOADD(reg)     "v_movd(%c,%0);\n"		1
reg: LOADF(reg)     "v_movf(%c,%0);\n"		1

reg: ADDD(reg,reg)  "v_addd(%c,%0,%1);\n"	1
reg: ADDF(reg,reg)  "v_addf(%c,%0,%1);\n"	1
reg: DIVD(reg,reg)  "v_divd(%c,%0,%1);\n"	1
reg: DIVF(reg,reg)  "v_divf(%c,%0,%1);\n"	1
reg: MULD(reg,reg)  "v_muld(%c,%0,%1);\n"	1
reg: MULF(reg,reg)  "v_mulf(%c,%0,%1);\n"	1
reg: SUBD(reg,reg)  "v_subd(%c,%0,%1);\n"	1
reg: SUBF(reg,reg)  "v_subf(%c,%0,%1);\n"	1
reg: NEGD(reg)      "v_negd(%c,%0);\n"		1
reg: NEGF(reg)      "v_negf(%c,%0);\n"		1

reg: CVCI(reg)      "v_cvc2i(%c,%0);\n"		1
reg: CVCI(imm)      "v_cvc2ii(%c,%0);\n"	1
reg: CVIC(reg)      "v_cvi2c(%c,%0);\n"		1
reg: CVIC(imm)      "v_cvi2ci(%c,%0);\n"	1
reg: CVSI(reg)      "v_cvs2i(%c,%0);\n"		1
reg: CVSI(imm)      "v_cvs2ii(%c,%0);\n"	1
reg: CVIS(reg)      "v_cvi2s(%c,%0);\n"		1
reg: CVIS(imm)      "v_cvi2si(%c,%0);\n"	1
reg: CVCU(reg)      "v_cvc2u(%c,%0);\n"		1
reg: CVCU(imm)      "v_cvc2ui(%c,%0);\n"	1
reg: CVUC(reg)      "v_cvu2c(%c,%0);\n"		1
reg: CVUC(imm)      "v_cvu2ci(%c,%0);\n"	1
reg: CVSU(reg)      "v_cvs2u(%c,%0);\n"		1
reg: CVSU(imm)      "v_cvs2ui(%c,%0);\n"	1
reg: CVUS(reg)      "v_cvu2s(%c,%0);\n"		1
reg: CVUS(imm)      "v_cvu2si(%c,%0);\n"	1
reg: CVIU(reg)      "v_cvi2u(%c,%0);\n"		1
reg: CVIU(imm)      "v_cvi2ui(%c,%0);\n"	1
reg: CVUI(reg)      "v_cvu2l(%c,%0);\n"		1
reg: CVUI(imm)      "v_cvu2li(%c,%0);\n"	1
reg: CVPU(reg)      "v_cvp2ul(%c,%0);\n"	1
reg: CVPU(imm)      "v_cvp2uli(%c,%0);\n"	1
reg: CVUP(reg)      "v_movu(%c,%0);\n"		1
reg: CVDF(reg)      "v_cvd2f(%c,%0);\n"		1
reg: CVFD(reg)      "v_cvf2d(%c,%0);\n"		1
reg: CVID(reg)      "v_cvi2d(%c,%0);\n"		1
reg: CVDI(reg)      "v_cvd2i(%c,%0);\n"		1

stmt: LABELV	    "v_label(%a);\n"		0
stmt: JUMPV(immp)   "v_jv((long)%0);\n"		1
stmt: JUMPV(reg)    "v_j(%0);\n"		1

stmt: EQI(reg,reg)  "v_beqi(%0,%1,%a);\n"	2
stmt: EQI(reg,imm)  "v_beqii(%0,%1,%a);\n"	1
stmt: GEI(reg,reg)  "v_bgei(%0,%1,%a);\n"	2
stmt: GEI(reg,imm)  "v_bgeii(%0,%1,%a);\n"	1
stmt: GEU(reg,reg)  "v_bgeu(%0,%1,%a);\n"	2
stmt: GEU(reg,imm)  "v_bgeui(%0,%1,%a);\n"	1
stmt: GTI(reg,reg)  "v_bgti(%0,%1,%a);\n"	2
stmt: GTI(reg,imm)  "v_bgtii(%0,%1,%a);\n"	1
stmt: GTU(reg,reg)  "v_bgtu(%0,%1,%a);\n"	2
stmt: GTU(reg,imm)  "v_bgtui(%0,%1,%a);\n"	1
stmt: LEI(reg,reg)  "v_blei(%0,%1,%a);\n"	2
stmt: LEI(reg,imm)  "v_bleii(%0,%1,%a);\n"	1
stmt: LEU(reg,reg)  "v_bleu(%0,%1,%a);\n"	2
stmt: LEU(reg,imm)  "v_bleui(%0,%1,%a);\n"	1
stmt: LTI(reg,reg)  "v_blti(%0,%1,%a);\n"	2
stmt: LTI(reg,imm)  "v_bltii(%0,%1,%a);\n"	1
stmt: LTU(reg,reg)  "v_bltu(%0,%1,%a);\n"	2
stmt: LTU(reg,imm)  "v_bltui(%0,%1,%a);\n"	1
stmt: NEI(reg,reg)  "v_bnei(%0,%1,%a);\n"	2
stmt: NEI(reg,imm)  "v_bneii(%0,%1,%a);\n"	1
stmt: EQD(reg,reg)  "v_beqd(%0,%1,%a);\n"	2
stmt: EQF(reg,reg)  "v_beqf(%0,%1,%a);\n"	2
stmt: LED(reg,reg)  "v_bled(%0,%1,%a);\n"	2
stmt: LEF(reg,reg)  "v_blef(%0,%1,%a);\n"	2
stmt: LTD(reg,reg)  "v_bltd(%0,%1,%a);\n"	2
stmt: LTF(reg,reg)  "v_bltf(%0,%1,%a);\n"	2
stmt: GED(reg,reg)  "v_bged(%0,%1,%a);\n"	2
stmt: GEF(reg,reg)  "v_bgef(%0,%1,%a);\n"	2
stmt: GTD(reg,reg)  "v_bgtd(%0,%1,%a);\n"	2
stmt: GTF(reg,reg)  "v_bgtf(%0,%1,%a);\n"	2
stmt: NED(reg,reg)  "v_bned(%0,%1,%a);\n"	2
stmt: NEF(reg,reg)  "v_bnef(%0,%1,%a);\n"	2

stmt: ARGD(reg)     "# v_argd(%0);\n"		1
stmt: ARGF(reg)     "# v_argf(%0);\n"		1
stmt: ARGI(reg)     "# v_argi(%0);\n"		1
stmt: ARGP(reg)     "# v_argp(%0);\n"		1
stmt: ARGV	    "# dynamic call;\n"		1
stmt: ARGB(INDIRB(reg))		"# arg\n"       1

reg:  CALLD(immp) "@v_movd(%c,v_ccalld(&vcs,(v_dptr)%0));\n" 1
reg:  CALLF(immp) "@v_movf(%c,v_ccallf(&vcs,(v_fptr)%0));\n" 1
reg:  CALLI(immp) "@v_movi(%c,v_ccalli(&vcs,(v_iptr)%0));\n" 1
stmt: CALLV(immp) "@v_ccallv(&vcs,(v_vptr)%0);\n"			1

reg:  CALLD(reg) "@v_movd(%c,v_rccalld(&vcs,%0));\n"	1
reg:  CALLF(reg) "@v_movf(%c,v_rccallf(&vcs,%0));\n"	1
reg:  CALLI(reg) "@v_movi(%c,v_rccalli(&vcs,%0));\n"	1
stmt: CALLV(reg) "@v_rccallv(&vcs,%0);\n"		1

stmt: RETD(reg)     "v_retd(%0);\n"		1
stmt: RETF(reg)     "v_retf(%0);\n"		1
stmt: RETI(reg)     "v_reti(%0);\n"		2
stmt: RETI(imm)     "v_retii(%0);\n"		1

stmt: CRETB(INDIRB(reg))  "#return %0;\n"
stmt: CRETC(reg)    "#return %0;\n"
stmt: CRETD(reg)    "#return %0;\n"
stmt: CRETF(reg)    "#return %0;\n"
stmt: CRETI(reg)    "#return %0;\n"
stmt: CRETP(reg)    "#return %0;\n"
stmt: CRETS(reg)    "#return %0;\n"
stmt: CRETU(reg)    "#return %0;\n"

stmt: CRETB(rtcb)   "# return reg(rtcb)\n"
stmt: CRETC(imm)    "# imm->reg; return reg\n"
stmt: CRETD(imm)    "# imm->reg; return reg\n"
stmt: CRETF(imm)    "# imm->reg; return reg\n"
stmt: CRETI(imm)    "# imm->reg; return reg\n"
stmt: CRETP(imm)    "# imm->reg; return reg\n"
stmt: CRETS(imm)    "# imm->reg; return reg\n"
stmt: CRETU(imm)    "# imm->reg; return reg\n"

stmt: KLABELV       "%a:\n"
stmt: KJUMPV	    "goto %a;\n"
stmt: KTEST	    "if ((%a)==0)\n"

stmt: DJUMP         "v_jv(%a->lab);\n"
%%
static void progbeg (int argc, char *argv[]) {
     int i;

     for (i = 0; i < 31; i += 2)
	  freg[i] = mkreg("fl%d", i, 3, FREG);
     for (i = 0; i < 32; i++)
	  ireg[i]  = mkreg("il%d", i, 1, IREG);
     tr = mkreg("_tc_tr0", 0, 1, IREG);
     for (i = 0; i < 3; i++)
	  sreg[i] = ireg[tmpregs[i]];
     blkreg = ireg[8];
				/* set reg state */
     bcopy((char*)rmap, (char*)oldrmap, 16*sizeof(Symbol));

     oldtmask[IREG] = tmask[IREG]; oldtmask[FREG] = tmask[FREG];
     oldvmask[IREG] = vmask[IREG]; oldvmask[FREG] = vmask[FREG];
     tmask[IREG] = INTTMP; tmask[FREG] = FLTTMP;
     vmask[IREG] = INTVAR; vmask[FREG] = FLTVAR;
     
     rmap[CS] = rmap[VS] =
	  rmap[C] = rmap[S] = rmap[P] = rmap[B] = 
	       rmap[U] = rmap[I] = mkwildcard(ireg);
     rmap[F] = rmap[D] = mkwildcard(freg);
}
static void progend (void) {
     bcopy((char*)oldrmap, (char*)rmap, 16*sizeof(Symbol));
     tmask[IREG] = oldtmask[IREG];
     tmask[FREG] = oldtmask[FREG];
     vmask[IREG] = oldvmask[IREG];
     vmask[FREG] = oldvmask[FREG];
}
static void target (Node p) {
     assert (p);
     switch (generic(p->op)) {
     case CRET:
	  if (p->kids[0]->op == INDIRB)
	       setreg(p->kids[0]->kids[0], blkreg); /* rtarget */
	  else if (p->kids[0]->op == RTCB)
	       setreg(p->kids[0], blkreg);
	  break;
     case CALL:
	  eval.currentclosure->u.closure.hascall = 1;
	  break;
     }
}
static void clobber (Node p) {}
static void emit2 (Node p) {
     switch (p->op) {
     case GETREG: {
	  Symbol r = p->syms[0];
	  Node n = p->kids[0];
	  print("v_getreg(&%s,%s,V_VAR);\n",
		r->x.name, tfVerbose(optype(n->op)));
	  break;
     }
     case PUTREG: {
	  Symbol r = p->syms[0];
	  Node n = p->kids[0];
	  print("v_putreg(%s,%s);\n", r->x.name, tfVerbose(optype(n->op)));
	  break;
     }
     case ICSV:
     case ICSF: case ICSD: case ICSC: case ICSS:
     case ICSI: case ICSU: case ICSP: case ICSB: {
	  Symbol s;
	  s = p->syms[0];	assert(s);
	  print("%s->%s((void*)%s);%c", s->name, tcname.Cgffield, s->name,
		p->op == ICSV ? '\n' : ' ');
     }
     break;
     case RTCC: case RTCI: case RTCP: case RTCS: case RTCU: {
	  assert(p->syms[0]->rtcp);
	  print("(long)%s", p->syms[0]->x.name);
     }
     break;
     case RTCD: case RTCF: {
	  assert(p->syms[0]->rtcp);
	  print("%s", p->syms[0]->x.name);
     }
     break;
     case CRETB: {
	  Node k = p->kids[0];
	  Symbol s = k->syms[0];
	  assert(p->kids[0]->op == RTCB);
	  print("v_setp(%s,(long)&%s);\n", blkreg->x.name, s->x.name);
	  print("return %s;\n", blkreg->x.name);
     }
     break;
     case CRETC: case CRETD: case CRETF: 
     case CRETI: case CRETP: case CRETS: case CRETU: {
	  Node k = p->kids[0];
	  Symbol s;
	  char *rname;
/*	  if (generic(k->op) == RTC || generic(k->op) == CNST) {*/
	  if (!k->syms[RX] || !k->syms[RX]->x.regnode) {
	       s = k->syms[0];
	       rname = p->syms[RX]->x.name;
	       print("v_set%c(%s,%s);\n", tfSparse(s->type), rname, s->x.name);
	  } else {
	       s = k->syms[RX]; rname = s->x.name;
	       freelocal(eval.currentclosure, s->x.regnode->number, 
			 optype(p->op));
	  }
	  print("return %s;\n", rname);
     }
     break;
     case ASGNC: case ASGNS: case ASGNI: case ASGNP: 
     case ASGND: case ASGNF:
	  assert(p->kids[0]->op);
	  if (p->kids[0]->op == ADRVS) {
	       Symbol vs, rs;
	       char *vn, *rn;
	       Type ty;
	       assert(p->kids[0]->syms[0] && p->kids[1]->syms[RX]);
	       vs = p->kids[0]->syms[0];
	       assert(vs->type->op == VSPEC && vs->type->type
		      || vs->lclp);
	       ty = btot(optype(p->op)); /*vs->lclp?vs->type:vs->type->type;*/
	       rs = p->kids[1]->syms[RX];
	       vn = vs->name; rn = rs->x.name;
	       print("if (_tc_stackp(%s)) {\n"
		     "  v_st%ci(%s,v_lp,_tc_offset(%s));\n"
		     "} else {\n"
		     "  v_mov%c(_tc_reg(%s), %s);\n"
		     "}\n", vn, tfDense(ty), rn, vn, tfSparse(ty), vn, rn);
	  }
	  break;
     case INDIRC: case INDIRS: case INDIRI: case INDIRP: 
     case INDIRD: case INDIRF:
	  assert(p->kids[0]);
	  if (p->kids[0]->op == ADRVS) {
	       Symbol vs, rs;
	       char *vn, *rn;
	       Type ty;
	       assert(p->kids[0]->syms[0] && p->syms[RX]);
	       vs = p->kids[0]->syms[0];
	       assert(vs->type->op == VSPEC && vs->type->type
		      || vs->lclp);
	       ty = btot(optype(p->op)); /*vs->lclp?vs->type:vs->type->type;*/
	       rs = p->syms[RX];
	       vn = vs->name; rn = rs->x.name;
	       print("if (_tc_stackp(%s)) {\n"
		     "  v_ld%ci(%s,v_lp,_tc_offset(%s));\n"
		     "} else {\n"
		     "  v_mov%c(%s,_tc_reg(%s));\n"
		     "}\n", vn, tfDense(ty), rn, vn, tfSparse(ty), rn, vn);
	  }
	  break;
     case ARGB: case ARGD: case ARGF: case ARGI: case ARGP: {
	  Symbol s = p->kids[0]->syms[RX];
	  char *r;

	  if (!callinit) {
	       callinit = 1; print("v_push_init(&vcs);\n");
	  }

	  r = s->x.name;
	  print("v_arg_push(&vcs,%s,%s);\n", tfVerbose(optype(p->op)), r);
     }
     break;
     case ARGV:
	  assert(p->syms[0]);
	  if (!callinit) {
	       callinit = 1; print("v_push_init(&vcs);\n");
	  }
	  print("_tc_vdargs((_tc_dcall_t*)%s,&vcs);\n", 
		p->syms[0]->name);
	  break;
     case ASGNB:
	  blkcopy(getregnum(p->x.kids[0]), 0,
		  getregnum(p->x.kids[1]), 0,
		  p->syms[0]->u.c.v.i, tmpregs);
	  break;
     }
}
static void emit3(p) Node p; {
     switch (generic(p->op)) {
     case CALL:
	  if (!callinit)	/* In case there are no arguments */
	       print("v_push_init(&vcs);\n");
	  callinit = 0;
	  break;
     }
}
static void doarg (Node p) {}
static void iLocal (Symbol p) {
     if (p->rtcp) {		/* This is a meta-variable used for dynamic */
	  p->x.name = p->name;	/*   loop unrolling; need not be in closure */
	  return;
     }
     p->name = mangle(p->name);
     p->x.offset = -1;
     switch (p->sclass) {
     case AUTO:			/* Fall-through */
     case REGISTER:
	  if (askregvar(p, rmap[ttob(p->type)]) == 0) {
	       assert(p->sclass == AUTO);
	       p->x.name = p->name;		
	       eval.currentclosure->u.closure.locals = 
		    append(p, eval.currentclosure->u.closure.locals);
	  } else if (p->x.regnode)
	       eval.currentclosure->u.closure.locreg[p->x.regnode->set] |=
		    1<<p->x.regnode->number;
	  return;
     case STATIC:
	  if (lookup(p->name, eval.currentclosure->u.closure.statics) == NULL)
	       installcopy(p, &eval.currentclosure->u.closure.statics, 0,FUNC);
	  return;
     default:
	  assert(0);
     }
}
static void mkfreevar(Symbol p, void *v) {
     p->x.name = mangle(p->name);
     p->x.offset = -1;
}
static void mkvs (Symbol p, void *v) {
     p->x.name = p->name;
     p->x.offset = -1;
}
static void mklcl (Symbol p, void *v) {
     p->x.name = p->name;
     p->x.offset = -1;
}
static void function (Symbol f, Symbol callee[], Symbol caller[], int ncalls) {
     eval.currentclosure = NULL;
     
     /* define global and static symbols */
     foreach(eval.esymaddr, 0, defesymaddr, NULL);
     foreach(eval.esymval, 0, defesymval, NULL);
     foreach(eval.esymcnst, 0, defesymcnst, NULL);
     foreach(eval.esymstr, 0, defesymstr, NULL);
     
     gencode(NULL,NULL);
     if (!tflag)
	  emitcode();
     
     /* reset the x.name fields */
     foreach(eval.esymaddr, 0, undefesym, NULL);
     foreach(eval.esymval, 0, undefesym, NULL);
     foreach(eval.esymcnst, 0, undefesym, NULL);
     foreach(eval.esymstr, 0, undefesym, NULL);
}
static void genclosurebeg (Code cp) {
     offset = argoffset = maxoffset = maxargoffset = 0;
#if 0
     IR->t.uniqueid = 0;
#endif
     usedmask[IREG] = usedmask[FREG] = 0;
     freemask[IREG] = freemask[FREG] = ~(unsigned)0;

     cp->u.closure.locreg[IREG] = cp->u.closure.locreg[FREG] = 0;
     cp->u.closure.hascall = 0;
     cp->u.closure.locals = NULL;
     cp->u.closure.fakeregs = NULL;
     
     forall(eval.currentclosure->u.closure.lcl, mklcl, NULL);
     foreach(eval.currentclosure->u.closure.tfv, 0, mkfreevar, NULL);
     foreach(eval.currentclosure->u.closure.fvs, 0, mkvs, NULL);
     foreach(eval.currentclosure->u.closure.fcs, 0, mkvs, NULL);
}
static void genclosureend (Code cp) {
     cp->u.begin->u.closure.regmask[0] = 
	  usedmask[0]&(~cp->u.begin->u.closure.locreg[0]);
     cp->u.begin->u.closure.regmask[1] = 
	  usedmask[1]&(~cp->u.begin->u.closure.locreg[1]);
}
static void emitclosurebeg (Code cp) {
     char *typename = deref(cp->u.closure.sym->type)->u.sym->name;
     char *codename = cp->u.closure.cgf->name;
     
     /* boilerplate */
     print("%s (void *c_arg)\n{\n",
	   typestring(0, cp->u.closure.cgf->type->type, codename));
     print("struct %s *c = c_arg;\n", typename);

     if (cp->u.closure.hascall)
	  print("struct v_cstate vcs;\n");
     
     emitcomment("declare and init labels");
     decllabel(cp);

     emitcomment("declare and init locals");
     decllocal(cp);

     emitcomment("declare and init c/vspecs and run-time constants");
     forall(cp->u.closure.lcl, decllcl, NULL);
     foreach(cp->u.closure.fvs, 0, declspec, NULL);
     foreach(cp->u.closure.fcs, 0, declspec, NULL);
     foreach(cp->u.closure.rtc, 0, declrtc, NULL);

     emitcomment("declare and init statics");
     foreach(cp->u.closure.statics, 0, declstatic, NULL);
     
     emitcomment("declare and init free vars");
     foreach(cp->u.closure.tfv, 0, declfreevar, NULL);
     
     emitcomment("init more locals");
     initlocal(cp);

     emitcomment("create label, if necessary");
     print("if (c->lab) v_label(c->lab);\n");

     emitcomment("code");
}
static void declfreevar (Symbol s, void *v) {
     assert(s && s->tfvp);
     if (s->nfvp) return;	/* Do not print if s was pruned by cleanfvs */
     print("unsigned long %s = (unsigned long)c->%s;\n",s->x.name,s->name);
}
static void declspec(s, v) Symbol s; void *v; {
     if (isvspec(s->type)) {
	  print("_tc_vspec_t %s = (_tc_vspec_t)c->%s;\n",s->name,s->name);
     } else {
	  assert(iscspec(s->type));
	  print("_tc_closure_t %s = (_tc_closure_t) c->%s;\n",
		s->name, s->name);
     }
}
static void decllcl(s, v) Symbol s; void *v; {
     print("_tc_vspec_t %s = (_tc_vspec_t)c->%s;\n",s->name,s->name);
}
static void declrtc (Symbol s, void *v) {
     if (!(s->nrtcp || s->drtcp))
	  print("%s = c->%s;\n",typestring(0, s->type,s->name),s->name);
}
static void defesymaddr (Symbol s, void *v) {
     char *addrname;
     if (! s->generated) {
	  s->generated = 1;
	  addrname = stringf("__esymA%d_addr",unique++);
	  print("static void *%s = %s;\n",addrname,s->name);
	  s->x.name = addrname;
	  s->u.c.loc = 0;
     }
}
static void defesymval (Symbol s, void *v) {
     char *addrname;
     if (! s->generated) {
	  s->generated = 1;
	  addrname = stringf("*__esymV%d_addr", unique++);
	  if (isptr(s->type))
	       print("static %s = &%s;\n", 
		     typestring(0, s->type, addrname), s->name);
	  else if (isfunc(s->type) || isarray(s->type))
	       print("static void %s = %s;\n", addrname, s->name);
	  else
	       print("static void %s = &%s;\n", addrname, s->name);
	  s->x.name = &addrname[1];
	  s->u.c.loc = 0;
     }
}
static void defesymcnst (Symbol s, void *v) {
     char *name, *addrname, *t;
     if (! s->generated) {
	  s->generated = 1;
	  name = stringf("__esymC%d", unique++);
	  print("%s = %s;\n", typestring(0, s->type, name), s->name);
	  addrname = stringf("%s_addr", name);
	  print("void *%s = &%s;\n", addrname, name);
	  s->x.name = addrname;
	  s->u.c.loc = s->original->u.c.loc;
     }
     t = s->x.name;
     s->x.name = s->original->u.c.loc->x.name;
     s->original->u.c.loc->x.name = t;
}
static void defesymstr (Symbol s, void *v) {
     char *addrname, *t;
     if (! s->generated) {
	  s->generated = 1;
	  addrname = stringf("__esymS%d_addr", unique++);
	  print("char *%s = \"", addrname);
	  outsverbatim(s->name);
	  print("\";\n");
	  s->x.name = addrname;
	  s->u.c.loc = s->original->u.c.loc;
     }
     t = s->x.name;
     s->x.name = s->original->u.c.loc->x.name;
     s->original->u.c.loc->x.name = t;
}
static void undefesym (Symbol s, void *v) {
     char *t;
     Symbol orig;
     if (s->u.c.loc) {
	  orig = s->original->u.c.loc;
	  t = s->x.name;  s->x.name = orig->x.name;  orig->x.name = t;
     }
}
static void decllabel (Code cp) {
     List lc, lp;
     lc = lp = cp->u.closure.labels;
     if (lp)
	  do {
	       print("v_label_t %s = v_genlabel();\n",
		     ((Symbol)lc->x)->x.name);
	  } while  ((lc = lc->link) != lp);
}
static void decllocal (Code cp) {
     int i,j;
     List lc, lp;
				/* Locals on the stack */
     lc = lp = cp->u.closure.locals;
     if (lp)
	  do {
	       Symbol s = (Symbol)lc->x;
	       if (isarray(s->type) || isstruct(s->type))
		    print("_tc_vspec_t %s = v_localb(%d);\n", 
			  s->x.name, s->type->size);
	       else
		    print("_tc_vspec_t %s = v_local(%s);\n",
			  s->x.name, tfVerbose(ttob(s->type)));
	  } while ((lc = lc->link) != lp);
				/* Fake registers for spilled locals */
     lc = lp = cp->u.closure.fakeregs;
     if (lp)
	  do {
	       Symbol s = (Symbol)lc->x;
	       print("_tc_vspec_t %s;\n",
		     s->x.name, s->x.regnode->set == IREG ? "V_I" : "V_D");
	  } while  ((lc = lc->link) != lp);
				/* Temporaries in registers */
     for (i=1,j=0; i; i<<=1,j++) 
	  if (cp->u.closure.regmask[IREG]&i)
	       print("_tc_vspec_t %s;\n", ireg[j]->x.name);
     for (i=1,j=0; i; i<<=2,j+=2)
	  if (cp->u.closure.regmask[FREG]&i)
	       print("_tc_vspec_t %s;\n", freg[j]->x.name);
				/* Locals in registers */
     for (i=1,j=0; i; i<<=1,j++) 
	  if (cp->u.closure.locreg[IREG]&i)
	       print("_tc_vspec_t %s;\n", ireg[j]->x.name);
     for (i=1,j=0; i; i<<=2,j+=2)
	  if (cp->u.closure.locreg[FREG]&i)
	       print("_tc_vspec_t %s;\n", freg[j]->x.name);
}
static void initlocal (Code cp) {
     int i,j;
     List lc, lp;
     lc = lp = cp->u.closure.fakeregs;
     if (lp)
	  do {
	       Symbol s = (Symbol)lc->x;
	       print("v_getreg(&%s,%s,V_VAR);\n",
		     s->x.name, s->x.regnode->set == IREG ? "V_I" : "V_D");
	  } while  ((lc = lc->link) != lp);
				/* Locals in registers */
     for (i=1,j=0; i; i<<=1,j++)
	  if (cp->u.closure.locreg[IREG]&i)
	       print("v_getreg(&%s,V_I,V_VAR);\n", ireg[j]->x.name);
     for (i=1,j=0; i; i<<=2,j+=2)
	  if (cp->u.closure.locreg[FREG]&i)
	       print("v_getreg(&%s,V_D,V_VAR);\n", freg[j]->x.name);
}
static void freelocal (Code cp, int n, int t) {
     unsigned int ilocreg = cp->u.closure.locreg[IREG];
     unsigned int flocreg = cp->u.closure.locreg[FREG];
     int i,j;
     List lc, lp;

     switch (t) {
     case F: case D:
	  if (n >= 0 && n < 32)
	       flocreg &= ~(1<<n);
	  break;
     case C: case S: case I: case U: case P:
	  if (n >= 0 && n < 32)
	       ilocreg &= ~(1<<n);
	  break;
     case 0: 
	  break;
     default: 
	  assert(0);
     }

     lc = lp = cp->u.closure.fakeregs;
     if (lp)
	  do {
	       Symbol s = (Symbol)lc->x;
	       print("v_putreg(%s,%s);\n",
		     s->x.name, s->x.regnode->set == IREG ? "V_I" : "V_D");
	  } while  ((lc = lc->link) != lp);
     for (i=1,j=0; i; i<<=1,j++)
	  if (ilocreg&i) print("v_putreg(%s,V_I);\n", ireg[j]->x.name);
     for (i=1,j=0; i; i<<=2,j+=2)
	  if (flocreg&i) print("v_putreg(%s,V_D);\n", freg[j]->x.name);
}
static void declstatic (Symbol s, void *v) {
     print("static %s;\n",typestring(0, s->type,stringf("__%s",s->name)));
     print("unsigned long %s = (unsigned long)&__%s;\n",s->x.name,s->name);
}
static void emitcomment (char *c) {
     print("/* %s */\n",c);
}
static void emitclosureend (Code cp) {
     freelocal(cp->u.begin, -1, 0);
     outs("}\n\n");
}
static int isimmediate (n) Node n; {
     assert(n);
     switch (generic(n->op)) {
     case CNST: case RTC: case ADRFF: case ADRFL: case ADDRG:
	  return 1;
     default:
	  return 0;
     }
}
static void defconst(ty, v) int ty; Value v; {}
static void defaddress(p) Symbol p; {}
static void defstring(n, str) int n; char *str; {}
static void export(p) Symbol p; {}
static void import(p) Symbol p; {}
static void defsymbol (Symbol p) {
     p->x.name = p->name;
     if (p->scope == LABELS)
	  p->x.name = stringf("LL%s", p->x.name);
}
static void address (Symbol q, Symbol p, int n) {
     q->x.name = stringf("(%s+%d)\n",q->name,n);
     q->x.offset = p->x.offset + n;
}
static void global(p) Symbol p; {}
static void segment(n) int n; {}
static void space(n) int n; {}
static void blkloop(int dreg, int doff, int sreg, int soff, 
		    int size, int tmps[]) {
     int lab = genlabel(1);
     
     print("{\n");
     print("v_addui(r%d, r%d, %d);\n", sreg, sreg, size&~7);
     print("v_addui(r%d, r%d, %d);\n", tmps[2], dreg, size&~7);
     blkcopy(tmps[2], doff, sreg, soff, size&7, tmps);
     
     print("v_label_t L%d = v_genlabel();\n", lab);

     print("v_addui(r%d, r%d, %d);\n", sreg, sreg, -8);
     print("v_addui(r%d, r%d, %d);\n", tmps[2], tmps[2], -8);
     blkcopy(tmps[2], doff, sreg, soff, 8, tmps);
     print("v_bltu(r%d, r%d, L%d);\n",dreg, tmps[2], lab);
     print("}\n\n");
}
static void blkfetch (int size, int off, int reg, int tmp) {
     assert(size == 1 || size == 2 || size == 4);
     if (size == 1) {
	  print("v_lduci(r%d, r%d, %d);\n",tmp, reg, off);
     } else if (size == 2) {
	  print("v_ldusi(r%d, r%d, %d);\n",tmp, reg, off);
     } else {
	  print("v_lduli(r%d, r%d, %d);\n",tmp, reg, off);
     }
}
static void blkstore (int size, int off, int reg, int tmp) {
     assert(size == 1 || size == 2 || size == 4);
     if (size == 1) {
	  print("v_stuci(r%d, r%d, %d);\n",tmp, reg, off);
     } else if (size == 2) {
	  print("v_stusi(r%d, r%d, %d);\n",tmp, reg, off);
     } else {
	  print("v_stuli(r%d, r%d, %d);\n",tmp, reg, off);
     }
}

Interface sslittleVIR = {
	{ 1, 1, 0,},	  /* char */
	{ 2, 2, 0,},	  /* short */
	{ 4, 4, 0,},	  /* int */
	{ 4, 4, 1,},	  /* float */
	{ 8, 8, 1,},	  /* double */
	{ 4, 4, 0,},	  /* T * */
	{ 0, 1, 0,},	  /* struct */
	1,  /* little_endian */
	0,  /* mulops_calls */
	0,  /* wants_callb */
	1,  /* wants_argb */
	1,  /* left_to_right */
	0,  /* wants_dag */
address,
blockbeg,
blockend,
defaddress,
defconst,
defstring,
defsymbol,
emit,
export,
function,
gen,
global,
import,
iLocal,
progbeg,
progend,
segment,
space,
	0, 0, 0, 0, 0, 0, 0,
	{
		4,	/* max_unaligned_load */
		blkfetch, blkstore, blkloop,
		_label,
		_rule,
		_nts,
		_kids,
		_opname,
		_arity,
		_string,
		_templates,
		_isinstruction,
		_ntname,
		isimmediate,
		emit2,
		emit3,
		doarg,
		target,
		clobber,
	},
	{
	  genclosurebeg,
	  genclosureend,
	  emitclosurebeg,
	  emitclosureend,
	  0,
	}
};

Interface ssbigVIR = {
	{ 1, 1, 0,},	  /* char */
	{ 2, 2, 0,},	  /* short */
	{ 4, 4, 0,},	  /* int */
	{ 4, 4, 1,},	  /* float */
	{ 8, 8, 1,},	  /* double */
	{ 4, 4, 0,},	  /* T * */
	{ 0, 1, 0,},	  /* struct */
	0,  /* little_endian */
	0,  /* mulops_calls */
	0,  /* wants_callb */
	1,  /* wants_argb */
	1,  /* left_to_right */
	0,  /* wants_dag */
address,
blockbeg,
blockend,
defaddress,
defconst,
defstring,
defsymbol,
emit,
export,
function,
gen,
global,
import,
iLocal,
progbeg,
progend,
segment,
space,
	0, 0, 0, 0, 0, 0, 0,
	{
		4,	/* max_unaligned_load */
		blkfetch, blkstore, blkloop,
		_label,
		_rule,
		_nts,
		_kids,
		_opname,
		_arity,
		_string,
		_templates,
		_isinstruction,
		_ntname,
		isimmediate,
		emit2,
		emit3,
		doarg,
		target,
		clobber,
	},
	{
	  genclosurebeg,
	  genclosureend,
	  emitclosurebeg,
	  emitclosureend,
	  0,
	}
};

Interface mipselVIR = {
	{ 1, 1, 0,},	  /* char */
	{ 2, 2, 0,},	  /* short */
	{ 4, 4, 0,},	  /* int */
	{ 4, 4, 1,},	  /* float */
	{ 8, 8, 1,},	  /* double */
	{ 4, 4, 0,},	  /* T * */
	{ 0, 1, 0,},	  /* struct */
	1,  /* little_endian */
	0,  /* mulops_calls */
	0,  /* wants_callb */
	1,  /* wants_argb */
	1,  /* left_to_right */
	0,  /* wants_dag */
address,
blockbeg,
blockend,
defaddress,
defconst,
defstring,
defsymbol,
emit,
export,
function,
gen,
global,
import,
iLocal,
progbeg,
progend,
segment,
space,
	0, 0, 0, 0, 0, 0, 0,
	{
		4,	/* max_unaligned_load */
		blkfetch, blkstore, blkloop,
		_label,
		_rule,
		_nts,
		_kids,
		_opname,
		_arity,
		_string,
		_templates,
		_isinstruction,
		_ntname,
		isimmediate,
		emit2,
		emit3,
		doarg,
		target,
		clobber,
	},
	{
	  genclosurebeg,
	  genclosureend,
	  emitclosurebeg,
	  emitclosureend,
	  0,
	}
};

Interface sparcVIR = {
	{ 1, 1, 0,},	  /* char */
	{ 2, 2, 0,},	  /* short */
	{ 4, 4, 0,},	  /* int */
	{ 4, 4, 1,},	  /* float */
	{ 8, 8, 1,},	  /* double */
	{ 4, 4, 0,},	  /* T * */
	{ 0, 1, 0,},	  /* struct */
	0,  /* little_endian */
	1,  /* mulops_calls */
	1,  /* wants_callb */
	0,  /* wants_argb */
	1,  /* left_to_right */
	0,  /* wants_dag */
address,
blockbeg,
blockend,
defaddress,
defconst,
defstring,
defsymbol,
emit,
export,
function,
gen,
global,
import,
iLocal,
progbeg,
progend,
segment,
space,
	0, 0, 0, 0, 0, 0, 0,
	{
		4,	/* max_unaligned_load */
		blkfetch, blkstore, blkloop,
		_label,
		_rule,
		_nts,
		_kids,
		_opname,
		_arity,
		_string,
		_templates,
		_isinstruction,
		_ntname,
		isimmediate,
		emit2,
		emit3,
		doarg,
		target,
		clobber,
	},
	{
	  genclosurebeg,
	  genclosureend,
	  emitclosurebeg,
	  emitclosureend,
	  0,
	}
};

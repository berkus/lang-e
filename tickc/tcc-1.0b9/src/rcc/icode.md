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

static int unique = 0;

static Symbol sreg[3];
static Symbol blkreg; 
static Symbol ireg[32], freg[32];
static int tmpregs[] = {3, 9, 10};

static Symbol   oldrmap[16];
static unsigned oldtmask[2];
static unsigned oldvmask[2];

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
%term NOTE=832
%term KLABELV=872 KTEST=880 KJUMPV=904
%term VREGP=967 FASGN=976
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

stmt: NOTE   "%a\n"

rtcb: RTCB "#rname"	    idef("i_setp", 0)
rtcc: RTCC "#rname"	    idef("i_setc", 0)
rtcd: RTCD "#rname"	    idef("i_setd", 0)
rtcf: RTCF "#rname"	    idef("i_setf", 0)
rtci: RTCI "#rname"	    idef("i_seti", 0)
rtcs: RTCS "#rname"	    idef("i_sets", 0)
rtcu: RTCU "#rname"	    idef("i_setu", 0)
rtcp: RTCP "#rname"	    idef("i_setp", 0)

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

afv: ADDRGP    "__DRGP__ %a"	idef("i_setp", 0)
afv: ADRFFP    "__RFFP__ %a"	idef("i_setp", 0)
afv: ADRFLP    "__RFLP__ %a"	idef("i_setp", 0)

alv: ADDRFP    "__DRFP__ %a"	idef("i_leai", 0)
alv: ADDRLP    "__DRLP__ %a"	idef("i_leai", 0)

reg: SELFP     "i_self(%c);\n"	0

reg: alv       "i_lea%t0(%c,%0);\n"	1

immc: CNSTC    "%a"	idef("i_setc", 0)
immi: CNSTI    "%a"	idef("i_seti", 0)
imms: CNSTS    "%a"	idef("i_sets", 0)
immu: CNSTU    "%a"	idef("i_setu", 0)
immp: CNSTP    "%a"	idef("i_setp", 0)
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
reg: immc  "i_setc(%c,%0);\n"		1
reg: immi  "i_seti(%c,%0);\n"		1
reg: imms  "i_sets(%c,%0);\n"		1
reg: immu  "i_setu(%c,%0);\n"		1
reg: immp  "i_setp(%c,(long)%0);\n"	1

reg: rtcb  "i_setp(%c,(long)&%0);\n"	1
reg: rtcc  "i_setc(%c,%0);\n"       	1
reg: rtcf  "i_setf(%c,%0);\n"       	1
reg: rtcd  "i_setd(%c,%0);\n"       	1
reg: rtci  "i_seti(%c,%0);\n"       	1
reg: rtcp  "i_setp(%c,%0);\n"       	1
reg: rtcs  "i_sets(%c,%0);\n"       	1
reg: rtcu  "i_setu(%c,%0);\n"       	1

stmt: FASGN(alv, reg)    "#;\n" 0

addri: afv		"i_zero,(long)%0"	1
addri: ADDI(reg, imm)	"%0, %1"		1
addri: ADDP(reg, imm)	"%0, %1"		1
addri: ADDU(reg, imm)	"%0, %1"		1
stmt: ASGNC(addri,reg)	 "i_stci(%1,%0);\n" idef("i_stci", 1)
stmt: ASGNS(addri,reg)	 "i_stsi(%1,%0);\n" idef("i_stsi", 1)
stmt: ASGNI(addri,reg)	 "i_stii(%1,%0);\n" idef("i_stii", 1)
stmt: ASGNP(addri,reg)	 "i_stpi(%1,%0);\n" idef("i_stpi", 1)
stmt: ASGND(addri,reg)	 "i_stdi(%1,%0);\n" idef("i_stdi", 1)
stmt: ASGNF(addri,reg)	 "i_stfi(%1,%0);\n" idef("i_stfi", 1)
reg: INDIRC(addri)	 "i_ldci(%c,%0);\n" idef("i_ldci", 1)
reg: INDIRS(addri)	 "i_ldsi(%c,%0);\n" idef("i_ldsi", 1)
reg: INDIRI(addri)	 "i_ldii(%c,%0);\n" idef("i_ldii", 1)
reg: INDIRP(addri)	 "i_ldpi(%c,%0);\n" idef("i_ldpi", 1)
reg: INDIRD(addri)	 "i_lddi(%c,%0);\n" idef("i_lddi", 1)
reg: INDIRF(addri)	 "i_ldfi(%c,%0);\n" idef("i_ldfi", 1)
reg: CVCI(INDIRC(addri)) "i_ldci(%c,%0);\n"  idef("i_ldci", 1)
reg: CVCU(INDIRC(addri)) "i_lduci(%c,%0);\n" idef("i_lduci", 1)
reg: CVSI(INDIRS(addri)) "i_ldsi(%c,%0);\n"  idef("i_ldsi", 1)
reg: CVSU(INDIRS(addri)) "i_ldusi(%c,%0);\n" idef("i_ldusi", 1)

stmt: ASGNC(alv,reg)	"i_movc(%0,%1);\n"     idef("i_movc", 1)
stmt: ASGNS(alv,reg)	"i_movs(%0,%1);\n"     idef("i_movs", 1)
stmt: ASGNI(alv,reg)	"i_movi(%0,%1);\n"     idef("i_movi", 1)
stmt: ASGNP(alv,reg)	"i_movp(%0,%1);\n"     idef("i_movp", 1)
stmt: ASGND(alv,reg)	"i_movd(%0,%1);\n"     idef("i_movd", 1)
stmt: ASGNF(alv,reg)	"i_movf(%0,%1);\n"     idef("i_movf", 1)
reg: INDIRC(alv)	"i_movc(%c,%0);\n"     idef("i_movc", 1)
reg: INDIRS(alv)	"i_movs(%c,%0);\n"     idef("i_movs", 1)
reg: INDIRI(alv)	"i_movi(%c,%0);\n"     idef("i_movi", 1)
reg: INDIRP(alv)	"i_movp(%c,%0);\n"     idef("i_movp", 1)
reg: INDIRD(alv)	"i_movd(%c,%0);\n"     idef("i_movd", 1)
reg: INDIRF(alv)	"i_movf(%c,%0);\n"     idef("i_movf", 1)

addr: ADDI(reg, reg)	"%0,%1"		1
addr: ADDP(reg, reg)	"%0,%1"		1
addr: ADDU(reg, reg)	"%0,%1"		1
addr: reg		"i_zero,%0"	1
stmt: ASGNC(addr,reg)	 "i_stc(%1,%0);\n"	idef("i_stc", 1)
stmt: ASGNS(addr,reg)	 "i_sts(%1,%0);\n"	idef("i_sts", 1)
stmt: ASGNI(addr,reg)	 "i_sti(%1,%0);\n"	idef("i_sti", 1)
stmt: ASGNP(addr,reg)	 "i_stp(%1,%0);\n"	idef("i_stp", 1)
stmt: ASGND(addr,reg)	 "i_std(%1,%0);\n"	idef("i_std", 1)
stmt: ASGNF(addr,reg)	 "i_stf(%1,%0);\n"	idef("i_stf", 1)
reg: INDIRC(addr)	 "i_ldc(%c,%0);\n"	idef("i_ldc", 1)
reg: INDIRS(addr)	 "i_lds(%c,%0);\n" 	idef("i_lds", 1)
reg: INDIRI(addr)	 "i_ldi(%c,%0);\n" 	idef("i_ldi", 1)
reg: INDIRP(addr)	 "i_ldp(%c,%0);\n" 	idef("i_ldp", 1)
reg: INDIRD(addr)	 "i_ldd(%c,%0);\n" 	idef("i_ldd", 1)
reg: INDIRF(addr)	 "i_ldf(%c,%0);\n" 	idef("i_ldf", 1)
reg: CVCI(INDIRC(addr))	 "i_ldc(%c,%0);\n"	idef("i_ldc", 1)
reg: CVCU(INDIRC(addr))	 "i_lduc(%c,%0);\n"	idef("i_lduc", 1)
reg: CVSI(INDIRS(addr))	 "i_lds(%c,%0);\n" 	idef("i_lds", 1)
reg: CVSU(INDIRS(addr))	 "i_ldus(%c,%0);\n" 	idef("i_ldus", 1)

reg: DIVI(reg,reg)  "i_divi(%c,%0,%1);\n"	idef("i_divi", 2)
reg: DIVI(reg,imm)  "i_divii(%c,%0,%1);\n"	idef("i_divii", 1)
reg: DIVU(reg,reg)  "i_divu(%c,%0,%1);\n"	idef("i_divu", 2)
reg: DIVU(reg,imm)  "i_divui(%c,%0,%1);\n"	idef("i_divui", 1)
reg: MODI(reg,reg)  "i_modi(%c,%0,%1);\n"	idef("i_modi", 2)
reg: MODI(reg,imm)  "i_modii(%c,%0,%1);\n"	idef("i_modii", 1)
reg: MODU(reg,reg)  "i_modu(%c,%0,%1);\n"	idef("i_modu", 2)
reg: MODU(reg,imm)  "i_modui(%c,%0,%1);\n"	idef("i_modui", 1)
reg: MULI(reg,reg)  "i_muli(%c,%0,%1);\n"	idef("i_muli", 2)
reg: MULI(imm,reg)  "i_mulii(%c,%1,%0);\n"	idef("i_mulii", 1)
reg: MULU(reg,reg)  "i_mulu(%c,%0,%1);\n"	idef("i_mulu", 2)
reg: MULU(imm,reg)  "i_mului(%c,%1,%0);\n"	idef("i_mului", 1)

reg: ADDI(reg,reg)  "i_addi(%c,%0,%1);\n"	idef("i_addi", 2)
reg: ADDI(reg,imm)  "i_addii(%c,%0,%1);\n"	idef("i_addii", 1)
reg: ADDP(reg,reg)  "i_addp(%c,%0,%1);\n"	idef("i_addp", 2)
reg: ADDP(reg,imm)  "i_addpi(%c,%0,%1);\n"	idef("i_addpi", 1)
reg: ADDU(reg,reg)  "i_addu(%c,%0,%1);\n"	idef("i_addu", 2)
reg: ADDU(reg,imm)  "i_addui(%c,%0,%1);\n"	idef("i_addui", 1)
reg: BANDU(reg,reg) "i_andu(%c,%0,%1);\n"	idef("i_andu", 2)
reg: BANDU(reg,imm) "i_andui(%c,%0,%1);\n"	idef("i_andui", 1)
reg: BORU(reg,reg)  "i_oru(%c,%0,%1);\n"	idef("i_oru", 2)
reg: BORU(reg,imm)  "i_orui(%c,%0,%1);\n"	idef("i_orui", 1)
reg: BXORU(reg,reg) "i_xoru(%c,%0,%1);\n"	idef("i_xoru", 2)
reg: BXORU(reg,imm) "i_xorui(%c,%0,%1);\n"	idef("i_xorui", 1)
reg: SUBI(reg,reg)  "i_subi(%c,%0,%1);\n"	idef("i_subi", 2)
reg: SUBI(reg,imm)  "i_subii(%c,%0,%1);\n"	idef("i_subii", 1)
reg: SUBP(reg,reg)  "i_subp(%c,%0,%1);\n"	idef("i_subp", 2)
reg: SUBP(reg,imm)  "i_subpi(%c,%0,%1);\n"	idef("i_subpi", 1)
reg: SUBU(reg,reg)  "i_subu(%c,%0,%1);\n"	idef("i_subu", 2)
reg: SUBU(reg,imm)  "i_subui(%c,%0,%1);\n"	idef("i_subui", 1)

reg: LSHI(reg,reg)  "i_lshi(%c,%0,%1);\n"	idef("i_lshi", 2)
reg: LSHI(reg,imm)  "i_lshii(%c,%0,%1);\n"	idef("i_lshii", 1)
reg: LSHU(reg,reg)  "i_lshu(%c,%0,%1);\n"	idef("i_lshu", 2)
reg: LSHU(reg,imm)  "i_lshui(%c,%0,%1);\n"	idef("i_lshui", 1)
reg: RSHI(reg,reg)  "i_rshi(%c,%0,%1);\n"	idef("i_rshi", 2)
reg: RSHI(reg,imm)  "i_rshii(%c,%0,%1);\n"	idef("i_rshii", 1)
reg: RSHU(reg,reg)  "i_rshu(%c,%0,%1);\n"	idef("i_rshu", 2)
reg: RSHU(reg,imm)  "i_rshui(%c,%0,%1);\n"	idef("i_rshui", 1)

reg: BCOMU(reg)     "i_comu(%c,%0);\n"		idef("i_comu", 2)
reg: BCOMU(imm)     "i_comui(%c,%0);\n"		idef("i_comui", 1)
reg: NEGI(reg)      "i_negi(%c,%0);\n"		idef("i_negi", 2)
reg: NEGI(imm)      "i_negii(%c,%0);\n"		idef("i_negii", 1)

reg: LOADC(reg)     "i_movi(%c,%0);\n"		idef("i_movi", 1)
reg: LOADC(imm)     "i_seti(%c,%0);\n"		idef("i_seti", 1)
reg: LOADS(reg)     "i_movi(%c,%0);\n"		idef("i_movi", 1)
reg: LOADS(imm)     "i_seti(%c,%0);\n"		idef("i_seti", 1)
reg: LOADP(reg)     "i_movp(%c,%0);\n"		idef("i_movp", 1)
reg: LOADP(imm)     "i_setp(%c,%0);\n"		idef("i_setp", 1)
reg: LOADI(reg)     "i_movi(%c,%0);\n"		idef("i_movi", 1)
reg: LOADI(imm)     "i_seti(%c,%0);\n"		idef("i_seti", 1)
reg: LOADU(reg)     "i_movu(%c,%0);\n"		idef("i_movu", 1)
reg: LOADU(imm)     "i_setu(%c,%0);\n"		idef("i_setu", 1)
reg: LOADD(reg)     "i_movd(%c,%0);\n"		idef("i_movd", 1)
reg: LOADF(reg)     "i_movf(%c,%0);\n"		idef("i_movf", 1)

reg: ADDD(reg,reg)  "i_addd(%c,%0,%1);\n"	idef("i_addd", 1)
reg: ADDF(reg,reg)  "i_addf(%c,%0,%1);\n"	idef("i_addf", 1)
reg: DIVD(reg,reg)  "i_divd(%c,%0,%1);\n"	idef("i_divd", 1)
reg: DIVF(reg,reg)  "i_divf(%c,%0,%1);\n"	idef("i_divf", 1)
reg: MULD(reg,reg)  "i_muld(%c,%0,%1);\n"	idef("i_muld", 1)
reg: MULF(reg,reg)  "i_mulf(%c,%0,%1);\n"	idef("i_mulf", 1)
reg: SUBD(reg,reg)  "i_subd(%c,%0,%1);\n"	idef("i_subd", 1)
reg: SUBF(reg,reg)  "i_subf(%c,%0,%1);\n"	idef("i_subf", 1)
reg: NEGD(reg)      "i_negd(%c,%0);\n"		idef("i_negd", 1)
reg: NEGF(reg)      "i_negf(%c,%0);\n"		idef("i_negf", 1 )

reg: CVCI(reg)      "i_cvc2i(%c,%0);\n"		idef("i_cvc2i", 2)
reg: CVCI(imm)      "i_cvc2ii(%c,%0);\n"	idef("i_cvc2ii", 1)
reg: CVIC(reg)      "i_cvi2c(%c,%0);\n"		idef("i_cvi2c", 2)
reg: CVIC(imm)      "i_cvi2ci(%c,%0);\n"	idef("i_cvi2ci", 1)
reg: CVSI(reg)      "i_cvs2i(%c,%0);\n"		idef("i_cvs2i", 2)
reg: CVSI(imm)      "i_cvs2ii(%c,%0);\n"	idef("i_cvs2ii", 1)
reg: CVIS(reg)      "i_cvi2s(%c,%0);\n"		idef("i_cvi2s", 2)
reg: CVIS(imm)      "i_cvi2si(%c,%0);\n"	idef("i_cvi2si", 1)
reg: CVCU(reg)      "i_cvc2u(%c,%0);\n"		idef("i_cvc2u", 2)
reg: CVCU(imm)      "i_cvc2ui(%c,%0);\n"	idef("i_cvc2ui", 1)
reg: CVUC(reg)      "i_cvu2c(%c,%0);\n"		idef("i_cvu2c", 2)
reg: CVUC(imm)      "i_cvu2ci(%c,%0);\n"	idef("i_cvu2ci", 1)
reg: CVSU(reg)      "i_cvs2u(%c,%0);\n"		idef("i_cvs2u", 2)
reg: CVSU(imm)      "i_cvs2ui(%c,%0);\n"	idef("i_cvs2ui", 1)
reg: CVUS(reg)      "i_cvu2s(%c,%0);\n"		idef("i_cvu2s", 2)
reg: CVUS(imm)      "i_cvu2si(%c,%0);\n"	idef("i_cvu2si", 1)
reg: CVIU(reg)      "i_cvi2u(%c,%0);\n"		idef("i_cvi2u", 2)
reg: CVIU(imm)      "i_cvi2ui(%c,%0);\n"	idef("i_cvi2ui", 1)
reg: CVUI(reg)      "i_cvu2l(%c,%0);\n"		idef("i_cvu2l", 2)
reg: CVUI(imm)      "i_cvu2li(%c,%0);\n"	idef("i_cvu2li", 1)
reg: CVPU(reg)      "i_cvp2ul(%c,%0);\n"	idef("i_cvp2ul", 2)
reg: CVPU(imm)      "i_cvp2uli(%c,%0);\n"	idef("i_cvp2uli", 1)
reg: CVUP(reg)      "i_movu(%c,%0);\n"		idef("i_movu", 2)
reg: CVDF(reg)      "i_cvd2f(%c,%0);\n"		idef("i_cvd2f", 1)
reg: CVFD(reg)      "i_cvf2d(%c,%0);\n"		idef("i_cvf2d", 1)
reg: CVID(reg)      "i_cvi2d(%c,%0);\n"		idef("i_cvi2d", 1)
reg: CVDI(reg)      "i_cvd2i(%c,%0);\n"		idef("i_cvd2i", 1)

stmt: LABELV	    "i_label(%a);\n"		0
stmt: JUMPV(immp)   "i_jpi((long)%0);\n"	idef("i_jpi", 1)
stmt: JUMPV(reg)    "i_jp(%0);\n"		idef("i_jp", 1)

stmt: EQI(reg,reg)  "i_beqi(%0,%1,%a);\n"	idef("i_beqi", 2)
stmt: EQI(reg,imm)  "i_beqii(%0,%1,%a);\n"	idef("i_beqii", 1)
stmt: GEI(reg,reg)  "i_bgei(%0,%1,%a);\n"	idef("i_bgei", 2)
stmt: GEI(reg,imm)  "i_bgeii(%0,%1,%a);\n"	idef("i_bgeii", 1)
stmt: GEU(reg,reg)  "i_bgeu(%0,%1,%a);\n"	idef("i_bgeu", 2)
stmt: GEU(reg,imm)  "i_bgeui(%0,%1,%a);\n"	idef("i_bgeui", 1)
stmt: GTI(reg,reg)  "i_bgti(%0,%1,%a);\n"	idef("i_bgti", 2)
stmt: GTI(reg,imm)  "i_bgtii(%0,%1,%a);\n"	idef("i_bgtii", 1)
stmt: GTU(reg,reg)  "i_bgtu(%0,%1,%a);\n"	idef("i_bgtu", 2)
stmt: GTU(reg,imm)  "i_bgtui(%0,%1,%a);\n"	idef("i_bgtui", 1)
stmt: LEI(reg,reg)  "i_blei(%0,%1,%a);\n"	idef("i_blei", 2)
stmt: LEI(reg,imm)  "i_bleii(%0,%1,%a);\n"	idef("i_bleii", 1)
stmt: LEU(reg,reg)  "i_bleu(%0,%1,%a);\n"	idef("i_bleu", 2)
stmt: LEU(reg,imm)  "i_bleui(%0,%1,%a);\n"	idef("i_bleui", 1)
stmt: LTI(reg,reg)  "i_blti(%0,%1,%a);\n"	idef("i_blti", 2)
stmt: LTI(reg,imm)  "i_bltii(%0,%1,%a);\n"	idef("i_bltii", 1)
stmt: LTU(reg,reg)  "i_bltu(%0,%1,%a);\n"	idef("i_bltu", 2)
stmt: LTU(reg,imm)  "i_bltui(%0,%1,%a);\n"	idef("i_bltui", 1)
stmt: NEI(reg,reg)  "i_bnei(%0,%1,%a);\n"	idef("i_bnei", 2)
stmt: NEI(reg,imm)  "i_bneii(%0,%1,%a);\n"	idef("i_bneii", 1)
stmt: EQD(reg,reg)  "i_beqd(%0,%1,%a);\n"	idef("i_beqd", 2)
stmt: EQF(reg,reg)  "i_beqf(%0,%1,%a);\n"	idef("i_beqf", 2)
stmt: LED(reg,reg)  "i_bled(%0,%1,%a);\n"	idef("i_bled", 2)
stmt: LEF(reg,reg)  "i_blef(%0,%1,%a);\n"	idef("i_blef", 2)
stmt: LTD(reg,reg)  "i_bltd(%0,%1,%a);\n"	idef("i_bltd", 2)
stmt: LTF(reg,reg)  "i_bltf(%0,%1,%a);\n"	idef("i_bltf", 2)
stmt: GED(reg,reg)  "i_bged(%0,%1,%a);\n"	idef("i_bged", 2)
stmt: GEF(reg,reg)  "i_bgef(%0,%1,%a);\n"	idef("i_bgef", 2)
stmt: GTD(reg,reg)  "i_bgtd(%0,%1,%a);\n"	idef("i_bgtd", 2)
stmt: GTF(reg,reg)  "i_bgtf(%0,%1,%a);\n"	idef("i_bgtf", 2)
stmt: NED(reg,reg)  "i_bned(%0,%1,%a);\n"	idef("i_bned", 2)
stmt: NEF(reg,reg)  "i_bnef(%0,%1,%a);\n"	idef("i_bnef", 2)

stmt: ARGD(reg)     "i_argd(%0);\n"		idef("i_argd", 1)
stmt: ARGF(reg)     "i_argf(%0);\n"		idef("i_argf", 1)
stmt: ARGI(reg)     "i_argi(%0);\n"		idef("i_argi", 1)
stmt: ARGP(reg)     "i_argp(%0);\n"		idef("i_argp", 1)
stmt: ARGV	    "# dynamic call;\n"		1

reg:  CALLD(immp)   "i_calldi(%c,(v_dptr)%0);\n" idef("i_calldi", 1)
reg:  CALLF(immp)   "i_callfi(%c,(v_fptr)%0);\n" idef("i_callfi", 1)
reg:  CALLI(immp)   "i_callii(%c,(v_iptr)%0);\n" idef("i_callii", 1)
stmt: CALLV(immp)   "i_callvi((v_vptr)%0);\n"	idef("i_callvi", 1)
reg:  CALLD(reg)    "i_calld(%c,%0);\n"		idef("i_calld", 2)
reg:  CALLF(reg)    "i_callf(%c,%0);\n"		idef("i_callf", 2)
reg:  CALLI(reg)    "i_calli(%c,%0);\n"		idef("i_calli", 2)
stmt: CALLV(reg)    "i_callv(%0);\n"		idef("i_callv", 2)

stmt: RETD(reg)     "i_retd(%0);\n"		idef("i_retd", 1)
stmt: RETF(reg)     "i_retf(%0);\n"		idef("i_retf", 1)
stmt: RETI(reg)     "i_reti(%0);\n"		idef("i_reti", 2)
stmt: RETI(imm)     "i_retii(%0);\n"		idef("i_retii", 1)

stmt: CRETB(INDIRB(reg))  "return %0;\n"
stmt: CRETC(reg)    "return %0;\n"
stmt: CRETD(reg)    "return %0;\n"
stmt: CRETF(reg)    "return %0;\n"
stmt: CRETI(reg)    "return %0;\n"
stmt: CRETP(reg)    "return %0;\n"
stmt: CRETS(reg)    "return %0;\n"
stmt: CRETU(reg)    "return %0;\n"

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

stmt: DJUMP         "i_jpi(%a->lab);\n" idef("i_jpi", 0)
%%
#if 0
lor: VREGP	"%0"
lor: alv	"%0"
stmt: ASGNC(lor,imm)	 "i_setc(%0,%1);\n"		idef("i_setc", 1)
stmt: ASGNS(lor,imm)	 "i_sets(%0,%1);\n"		idef("i_sets", 1)
stmt: ASGNI(lor,imm)	 "i_seti(%0,%1);\n"		idef("i_seti", 1)
stmt: ASGNP(lor,imm)	 "i_setp(%0,%1);\n"		idef("i_setp", 1)
#endif
static void progbeg (int argc, char *argv[]) {
     int i;

     for (i = 0; i < 31; i += 2)
	  freg[i] = mkreg("fl%d", i, 3, FREG);
     for (i = 0; i < 32; i++)
	  ireg[i]  = mkreg("il%d", i, 1, IREG);
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
     case ASGN:
	  break;
     }
}
static void clobber (Node p) {}
static void emit2 (Node p) {
     switch (p->op) {
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
	  idef("i_setp", 0);
	  print("i_setp(%s,(long)&%s);\n", blkreg->x.name, s->x.name);
	  print("return %s;\n", blkreg->x.name);
     }
     break;
     case CRETC: case CRETD: case CRETF: 
     case CRETI: case CRETP: case CRETS: case CRETU: {
	  Node k = p->kids[0];
	  Symbol s = k->syms[0];
	  char *rname = p->syms[RX]->x.name;
	  char *opname = stringf("i_set%c", tfSparse(s->type));
	  idef(opname, 0);
	  print("%s(%s,%s);\n", opname, rname, s->x.name);
	  print("return %s;\n", rname);
     }
     break;
     case ARGV:
	  assert(p->syms[0]);
	  idef("i_argi", 0);  idef("i_argu", 0);
	  idef("i_argl", 0);  idef("i_argul", 0);
	  idef("i_argp", 0);  idef("i_argf", 0);  idef("i_argd", 0);
	  print("_tc_dargs((_tc_dcall_t*)%s);\n", p->syms[0]->name);
	  break;
     case ASGNB:
	  blkcopy(getregnum(p->x.kids[0]), 0,
		  getregnum(p->x.kids[1]), 0,
		  p->syms[0]->u.c.v.i, tmpregs);
	  break;
     }
}
static void doarg (Node p) {}
static void iLocal (Symbol p) {
     if (p->rtcp) {		/* Hack: this is a meta-variable used for */
	  p->x.name = p->name;	/*  dynamic loop unrolling */
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
	  }
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

     cp->u.closure.locals = NULL;
     cp->u.closure.fakeregs = NULL;
     
     forall(eval.currentclosure->u.closure.lcl, mklcl, NULL);
     foreach(eval.currentclosure->u.closure.tfv, 0, mkfreevar, NULL);
     foreach(eval.currentclosure->u.closure.fvs, 0, mkvs, NULL);
     foreach(eval.currentclosure->u.closure.fcs, 0, mkvs, NULL);
}
static void genclosureend (Code cp) {
     cp->u.begin->u.closure.regmask[0] = usedmask[0];
     cp->u.begin->u.closure.regmask[1] = usedmask[1];
}
static void emitclosurebeg (Code cp) {
     char *typename = deref(cp->u.closure.sym->type)->u.sym->name;
     char *codename = cp->u.closure.cgf->name;
     
     /* boilerplate */
     print("%s (void *c_arg)\n{\n",
	   typestring(0, cp->u.closure.cgf->type->type, codename));
     print("struct %s *c = c_arg;\n", typename);
     
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

     emitcomment("create label, if necessary");
     print("if (c->lab) i_label(c->lab);\n");

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
	  s->name = stringf("%s",s->name);
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
	       print("i_label_t %s = i_mklabel();\n",
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
		    print("_tc_vspec_t %s = _tc_localb(%d);\n", 
			  s->x.name, s->type->size);
	       else
		    print("_tc_vspec_t %s = _tc_local(I_MEMORY,%s);\n",
			  s->x.name, tfVerbose(ttob(s->type)));
	  } while ((lc = lc->link) != lp);
				/* Fake registers for spilled locals */
     lc = lp = cp->u.closure.fakeregs;
     if (lp)
	  do {
	       Symbol s = (Symbol)lc->x;
	       print("_tc_vspec_t %s = _tc_local(0,%s);\n",
		     s->x.name, s->x.regnode->set == IREG ? "I_I" : "I_D");
	  } while  ((lc = lc->link) != lp);
				/* Locals (and temporaries) in registers */
     for (i=1,j=0; i; i<<=1,j++)
	  if (cp->u.closure.regmask[IREG]&i)
	       print("_tc_vspec_t %s = _tc_local(0,I_I);\n", ireg[j]->x.name);
     
     for (i=1,j=0; i; i<<=2,j+=2)
	  if (cp->u.closure.regmask[FREG]&i)
	       print("_tc_vspec_t %s = _tc_local(0,I_D);\n", freg[j]->x.name);

}
static void declstatic (Symbol s, void *v) {
     print("static %s;\n",typestring(0, s->type,stringf("__%s",s->name)));
     print("unsigned long %s = (unsigned long)&__%s;\n",s->x.name,s->name);
}
static void emitcomment (char *c) {
     print("/* %s */\n",c);
}
static void emitclosureend (Code cp) {
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
     idef("i_addui", 0);
     idef("i_bltu", 0);
     print("i_addui(r%d, r%d, %d);\n", sreg, sreg, size&~7);
     print("i_addui(r%d, r%d, %d);\n", tmps[2], dreg, size&~7);
     blkcopy(tmps[2], doff, sreg, soff, size&7, tmps);
     
     print("i_label_t L%d = i_mklabel();\n", lab);

     print("i_addui(r%d, r%d, %d);\n", sreg, sreg, -8);
     print("i_addui(r%d, r%d, %d);\n", tmps[2], tmps[2], -8);
     blkcopy(tmps[2], doff, sreg, soff, 8, tmps);
     print("i_bltu(r%d, r%d, L%d);\n",dreg, tmps[2], lab);
     print("}\n\n");
}
static void blkfetch (int size, int off, int reg, int tmp) {
     assert(size == 1 || size == 2 || size == 4);
     if (size == 1) {
	  idef("i_lduci", 0);
	  print("i_lduci(r%d, r%d, %d);\n",tmp, reg, off);
     } else if (size == 2) {
	  idef("i_ldusi", 0);
	  print("i_ldusi(r%d, r%d, %d);\n",tmp, reg, off);
     } else {
	  idef("i_lduli", 0);
	  print("i_lduli(r%d, r%d, %d);\n",tmp, reg, off);
     }
}
static void blkstore (int size, int off, int reg, int tmp) {
     assert(size == 1 || size == 2 || size == 4);
     if (size == 1) {
	  idef("i_stuci", 0);
	  print("i_stuci(r%d, r%d, %d);\n",tmp, reg, off);
     } else if (size == 2) {
	  idef("i_stusi", 0);
	  print("i_stusi(r%d, r%d, %d);\n",tmp, reg, off);
     } else {
	  idef("i_stuli", 0);
	  print("i_stuli(r%d, r%d, %d);\n",tmp, reg, off);
     }
}

Interface sslittleIIR = {
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
		0,
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

Interface ssbigIIR = {
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
		0,
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

Interface mipselIIR = {
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
		0,
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

Interface sparcIIR = {
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
		0,
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

Interface i386IIR = {
	1, 1, 0,		/* char */
	2, 2, 0,		/* short */
	4, 4, 0,		/* int */
	4, 4, 1,		/* float */
	8, 4, 1,		/* double */
	4, 4, 0,		/* T * */
	0, 1, 0,		/* struct so that ARGB keeps stack aligned */
	1,			/* little_endian */
	0,			/* mulops_calls */
	0,			/* wants_callb */
	1,			/* wants_argb */
	0,			/* left_to_right */
	0,			/* wants_dag */
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
		1,	/* max_unaligned_load */
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
		0,
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

int v_errors;
#include <math.h>
#include "vcode.h"
#include "vdemand.h"
#include <values.h>


float c_fabs(float x) { return (x) < 0.0 ? -x : x; }
double c_abs(double x) { return (x) < 0.0 ? -x : x; }
float c_fceil(float x) { return (float)(int)(x + .5); }
double c_ceil(double x) { return (double)(int)(x + .5);}
float c_ffloor(float x) { return (float)(int)(x); }
double c_floor(double x) { return (double)(int)(x);}
float c_fsqrt(float x) { extern double sqrt(double); return (float)sqrt((double)x); }
double c_sqrt(double x) { extern double sqrt(double); 	return sqrt(x);}


int main(int argc, char *argv[]) {
	v_reg_type	arg_list[100];		/* make sure 100 is big enough */
	v_reg_type	reg;
	char		dc, s1c, s2c, cvc;
	unsigned char	duc, s1uc, s2uc, cvuc;
	short		ds, s1s, s2s, cvs;
	unsigned short	dus, s1us, s2us, cvus;
	int 	     	di, s1i, s2i, cvi;
	unsigned     	du, s1u, s2u, cvu;
	unsigned long   dul, s1ul, s2ul, cvul;
	long     	dl, s1l, s2l, cvl;
	float		df, s1f, s2f, cvf;
	double		dd, s1d, s2d, cvd;
	char 		*dp, *s1p, *s2p, *cvp;
	v_label_type	l;
	static unsigned insn[1000];
	static unsigned insn2[1000];
	v_iptr 	ip;
	v_iptr 	ip2;
	int 	iters = (argc == 2) ? atoi(argv[1]) : 10;


loop:

        s1p = (void *)rand();
        s2p = (void *)rand();

        s1i = rand() - rand();
        s2i = rand() - rand();
        if(!(s2i = rand() - rand()))
                s2i = rand() + 1;

        s1u = rand() - rand();
        if(!(s2u = rand() - rand()))
                s2u = rand() + 1;

        s1ul = rand() - rand();
        if(!(s2ul = rand() - rand()))
                s2ul = rand() + 1;

        s1l = rand() - rand();
        if(!(s2l = rand() - rand()))
                s2l = rand() + 1;

        s2us = rand() - rand();
        if(!(s2us = rand() - rand()))
                s2us = rand() + 1;

        s1f = (float)rand() / rand();
        s2f = (float)rand() / (rand()+1) * ((rand()%1) ? 1. : -1.);

        s1d = (double)rand() / rand();
        s2d = (double)rand() / (rand()+1) * ((rand()%1) ? 1. : -1.);



	v_lambda("param1i", "%i", arg_list, V_LEAF, insn, sizeof insn);
	v_reti(arg_list[1-1]);
	ip = v_end(0).i;
	vdemand(s2i == ((int(*)(int))ip)(s2i), param1i failed);

	v_lambda("call1i", "%i", arg_list, V_NLEAF, insn2, sizeof insn2);
	reg = v_scalli((v_iptr)ip, "%i", arg_list[0]);
	v_reti(reg);
	ip2 = v_end(0).i;
	vdemand2(s2i == ((int(*)(int))ip2)(s2i), call1i failed);

	v_lambda("param2i", "%i%i", arg_list, V_LEAF, insn, sizeof insn);
	v_reti(arg_list[2-1]);
	ip = v_end(0).i;
	vdemand(s2i == ((int(*)(int,int))ip)(s1i,s2i), param2i failed);

	v_lambda("call2i", "%i%i", arg_list, V_NLEAF, insn2, sizeof insn2);
	reg = v_scalli((v_iptr)ip, "%i%i", arg_list[0],arg_list[1]);
	v_reti(reg);
	ip2 = v_end(0).i;
	vdemand2(s2i == ((int(*)(int,int))ip2)(s1i,s2i), call2i failed);

	v_lambda("param3i", "%i%i%i", arg_list, V_LEAF, insn, sizeof insn);
	v_reti(arg_list[3-1]);
	ip = v_end(0).i;
	vdemand(s2i == ((int(*)(int,int,int))ip)(s1i,s1i,s2i), param3i failed);

	v_lambda("call3i", "%i%i%i", arg_list, V_NLEAF, insn2, sizeof insn2);
	reg = v_scalli((v_iptr)ip, "%i%i%i", arg_list[0],arg_list[1],arg_list[2]);
	v_reti(reg);
	ip2 = v_end(0).i;
	vdemand2(s2i == ((int(*)(int,int,int))ip2)(s1i,s1i,s2i), call3i failed);

	v_lambda("param4i", "%i%i%i%i", arg_list, V_LEAF, insn, sizeof insn);
	v_reti(arg_list[4-1]);
	ip = v_end(0).i;
	vdemand(s2i == ((int(*)(int,int,int,int))ip)(s1i,s1i,s1i,s2i), param4i failed);

	v_lambda("call4i", "%i%i%i%i", arg_list, V_NLEAF, insn2, sizeof insn2);
	reg = v_scalli((v_iptr)ip, "%i%i%i%i", arg_list[0],arg_list[1],arg_list[2],arg_list[3]);
	v_reti(reg);
	ip2 = v_end(0).i;
	vdemand2(s2i == ((int(*)(int,int,int,int))ip2)(s1i,s1i,s1i,s2i), call4i failed);

	v_lambda("param5i", "%i%i%i%i%i", arg_list, V_LEAF, insn, sizeof insn);
	v_reti(arg_list[5-1]);
	ip = v_end(0).i;
	vdemand(s2i == ((int(*)(int,int,int,int,int))ip)(s1i,s1i,s1i,s1i,s2i), param5i failed);

	v_lambda("call5i", "%i%i%i%i%i", arg_list, V_NLEAF, insn2, sizeof insn2);
	reg = v_scalli((v_iptr)ip, "%i%i%i%i%i", arg_list[0],arg_list[1],arg_list[2],arg_list[3],arg_list[4]);
	v_reti(reg);
	ip2 = v_end(0).i;
	vdemand2(s2i == ((int(*)(int,int,int,int,int))ip2)(s1i,s1i,s1i,s1i,s2i), call5i failed);

	v_lambda("param6i", "%i%i%i%i%i%i", arg_list, V_LEAF, insn, sizeof insn);
	v_reti(arg_list[6-1]);
	ip = v_end(0).i;
	vdemand(s2i == ((int(*)(int,int,int,int,int,int))ip)(s1i,s1i,s1i,s1i,s1i,s2i), param6i failed);

	v_lambda("call6i", "%i%i%i%i%i%i", arg_list, V_NLEAF, insn2, sizeof insn2);
	reg = v_scalli((v_iptr)ip, "%i%i%i%i%i%i", arg_list[0],arg_list[1],arg_list[2],arg_list[3],arg_list[4],arg_list[5]);
	v_reti(reg);
	ip2 = v_end(0).i;
	vdemand2(s2i == ((int(*)(int,int,int,int,int,int))ip2)(s1i,s1i,s1i,s1i,s1i,s2i), call6i failed);

	v_lambda("param1u", "%u", arg_list, V_LEAF, insn, sizeof insn);
	v_retu(arg_list[1-1]);
	ip = v_end(0).i;
	vdemand(s2u == ((unsigned(*)(unsigned))ip)(s2u), param1u failed);

	v_lambda("call1u", "%u", arg_list, V_NLEAF, insn2, sizeof insn2);
	reg = v_scallu((v_uptr)ip, "%u", arg_list[0]);
	v_retu(reg);
	ip2 = v_end(0).i;
	vdemand2(s2u == ((unsigned(*)(unsigned))ip2)(s2u), call1u failed);

	v_lambda("param2u", "%u%u", arg_list, V_LEAF, insn, sizeof insn);
	v_retu(arg_list[2-1]);
	ip = v_end(0).i;
	vdemand(s2u == ((unsigned(*)(unsigned,unsigned))ip)(s1u,s2u), param2u failed);

	v_lambda("call2u", "%u%u", arg_list, V_NLEAF, insn2, sizeof insn2);
	reg = v_scallu((v_uptr)ip, "%u%u", arg_list[0],arg_list[1]);
	v_retu(reg);
	ip2 = v_end(0).i;
	vdemand2(s2u == ((unsigned(*)(unsigned,unsigned))ip2)(s1u,s2u), call2u failed);

	v_lambda("param3u", "%u%u%u", arg_list, V_LEAF, insn, sizeof insn);
	v_retu(arg_list[3-1]);
	ip = v_end(0).i;
	vdemand(s2u == ((unsigned(*)(unsigned,unsigned,unsigned))ip)(s1u,s1u,s2u), param3u failed);

	v_lambda("call3u", "%u%u%u", arg_list, V_NLEAF, insn2, sizeof insn2);
	reg = v_scallu((v_uptr)ip, "%u%u%u", arg_list[0],arg_list[1],arg_list[2]);
	v_retu(reg);
	ip2 = v_end(0).i;
	vdemand2(s2u == ((unsigned(*)(unsigned,unsigned,unsigned))ip2)(s1u,s1u,s2u), call3u failed);

	v_lambda("param4u", "%u%u%u%u", arg_list, V_LEAF, insn, sizeof insn);
	v_retu(arg_list[4-1]);
	ip = v_end(0).i;
	vdemand(s2u == ((unsigned(*)(unsigned,unsigned,unsigned,unsigned))ip)(s1u,s1u,s1u,s2u), param4u failed);

	v_lambda("call4u", "%u%u%u%u", arg_list, V_NLEAF, insn2, sizeof insn2);
	reg = v_scallu((v_uptr)ip, "%u%u%u%u", arg_list[0],arg_list[1],arg_list[2],arg_list[3]);
	v_retu(reg);
	ip2 = v_end(0).i;
	vdemand2(s2u == ((unsigned(*)(unsigned,unsigned,unsigned,unsigned))ip2)(s1u,s1u,s1u,s2u), call4u failed);

	v_lambda("param5u", "%u%u%u%u%u", arg_list, V_LEAF, insn, sizeof insn);
	v_retu(arg_list[5-1]);
	ip = v_end(0).i;
	vdemand(s2u == ((unsigned(*)(unsigned,unsigned,unsigned,unsigned,unsigned))ip)(s1u,s1u,s1u,s1u,s2u), param5u failed);

	v_lambda("call5u", "%u%u%u%u%u", arg_list, V_NLEAF, insn2, sizeof insn2);
	reg = v_scallu((v_uptr)ip, "%u%u%u%u%u", arg_list[0],arg_list[1],arg_list[2],arg_list[3],arg_list[4]);
	v_retu(reg);
	ip2 = v_end(0).i;
	vdemand2(s2u == ((unsigned(*)(unsigned,unsigned,unsigned,unsigned,unsigned))ip2)(s1u,s1u,s1u,s1u,s2u), call5u failed);

	v_lambda("param6u", "%u%u%u%u%u%u", arg_list, V_LEAF, insn, sizeof insn);
	v_retu(arg_list[6-1]);
	ip = v_end(0).i;
	vdemand(s2u == ((unsigned(*)(unsigned,unsigned,unsigned,unsigned,unsigned,unsigned))ip)(s1u,s1u,s1u,s1u,s1u,s2u), param6u failed);

	v_lambda("call6u", "%u%u%u%u%u%u", arg_list, V_NLEAF, insn2, sizeof insn2);
	reg = v_scallu((v_uptr)ip, "%u%u%u%u%u%u", arg_list[0],arg_list[1],arg_list[2],arg_list[3],arg_list[4],arg_list[5]);
	v_retu(reg);
	ip2 = v_end(0).i;
	vdemand2(s2u == ((unsigned(*)(unsigned,unsigned,unsigned,unsigned,unsigned,unsigned))ip2)(s1u,s1u,s1u,s1u,s1u,s2u), call6u failed);

	v_lambda("param1l", "%l", arg_list, V_LEAF, insn, sizeof insn);
	v_retl(arg_list[1-1]);
	ip = v_end(0).i;
	vdemand(s2l == ((long(*)(long))ip)(s2l), param1l failed);

	v_lambda("call1l", "%l", arg_list, V_NLEAF, insn2, sizeof insn2);
	reg = v_scalll((v_lptr)ip, "%l", arg_list[0]);
	v_retl(reg);
	ip2 = v_end(0).i;
	vdemand2(s2l == ((long(*)(long))ip2)(s2l), call1l failed);

	v_lambda("param2l", "%l%l", arg_list, V_LEAF, insn, sizeof insn);
	v_retl(arg_list[2-1]);
	ip = v_end(0).i;
	vdemand(s2l == ((long(*)(long,long))ip)(s1l,s2l), param2l failed);

	v_lambda("call2l", "%l%l", arg_list, V_NLEAF, insn2, sizeof insn2);
	reg = v_scalll((v_lptr)ip, "%l%l", arg_list[0],arg_list[1]);
	v_retl(reg);
	ip2 = v_end(0).i;
	vdemand2(s2l == ((long(*)(long,long))ip2)(s1l,s2l), call2l failed);

	v_lambda("param3l", "%l%l%l", arg_list, V_LEAF, insn, sizeof insn);
	v_retl(arg_list[3-1]);
	ip = v_end(0).i;
	vdemand(s2l == ((long(*)(long,long,long))ip)(s1l,s1l,s2l), param3l failed);

	v_lambda("call3l", "%l%l%l", arg_list, V_NLEAF, insn2, sizeof insn2);
	reg = v_scalll((v_lptr)ip, "%l%l%l", arg_list[0],arg_list[1],arg_list[2]);
	v_retl(reg);
	ip2 = v_end(0).i;
	vdemand2(s2l == ((long(*)(long,long,long))ip2)(s1l,s1l,s2l), call3l failed);

	v_lambda("param4l", "%l%l%l%l", arg_list, V_LEAF, insn, sizeof insn);
	v_retl(arg_list[4-1]);
	ip = v_end(0).i;
	vdemand(s2l == ((long(*)(long,long,long,long))ip)(s1l,s1l,s1l,s2l), param4l failed);

	v_lambda("call4l", "%l%l%l%l", arg_list, V_NLEAF, insn2, sizeof insn2);
	reg = v_scalll((v_lptr)ip, "%l%l%l%l", arg_list[0],arg_list[1],arg_list[2],arg_list[3]);
	v_retl(reg);
	ip2 = v_end(0).i;
	vdemand2(s2l == ((long(*)(long,long,long,long))ip2)(s1l,s1l,s1l,s2l), call4l failed);

	v_lambda("param5l", "%l%l%l%l%l", arg_list, V_LEAF, insn, sizeof insn);
	v_retl(arg_list[5-1]);
	ip = v_end(0).i;
	vdemand(s2l == ((long(*)(long,long,long,long,long))ip)(s1l,s1l,s1l,s1l,s2l), param5l failed);

	v_lambda("call5l", "%l%l%l%l%l", arg_list, V_NLEAF, insn2, sizeof insn2);
	reg = v_scalll((v_lptr)ip, "%l%l%l%l%l", arg_list[0],arg_list[1],arg_list[2],arg_list[3],arg_list[4]);
	v_retl(reg);
	ip2 = v_end(0).i;
	vdemand2(s2l == ((long(*)(long,long,long,long,long))ip2)(s1l,s1l,s1l,s1l,s2l), call5l failed);

	v_lambda("param6l", "%l%l%l%l%l%l", arg_list, V_LEAF, insn, sizeof insn);
	v_retl(arg_list[6-1]);
	ip = v_end(0).i;
	vdemand(s2l == ((long(*)(long,long,long,long,long,long))ip)(s1l,s1l,s1l,s1l,s1l,s2l), param6l failed);

	v_lambda("call6l", "%l%l%l%l%l%l", arg_list, V_NLEAF, insn2, sizeof insn2);
	reg = v_scalll((v_lptr)ip, "%l%l%l%l%l%l", arg_list[0],arg_list[1],arg_list[2],arg_list[3],arg_list[4],arg_list[5]);
	v_retl(reg);
	ip2 = v_end(0).i;
	vdemand2(s2l == ((long(*)(long,long,long,long,long,long))ip2)(s1l,s1l,s1l,s1l,s1l,s2l), call6l failed);

	v_lambda("param1ul", "%ul", arg_list, V_LEAF, insn, sizeof insn);
	v_retul(arg_list[1-1]);
	ip = v_end(0).i;
	vdemand(s2ul == ((unsigned long(*)(unsigned long))ip)(s2ul), param1ul failed);

	v_lambda("call1ul", "%ul", arg_list, V_NLEAF, insn2, sizeof insn2);
	reg = v_scallul((v_ulptr)ip, "%ul", arg_list[0]);
	v_retul(reg);
	ip2 = v_end(0).i;
	vdemand2(s2ul == ((unsigned long(*)(unsigned long))ip2)(s2ul), call1ul failed);

	v_lambda("param2ul", "%ul%ul", arg_list, V_LEAF, insn, sizeof insn);
	v_retul(arg_list[2-1]);
	ip = v_end(0).i;
	vdemand(s2ul == ((unsigned long(*)(unsigned long,unsigned long))ip)(s1ul,s2ul), param2ul failed);

	v_lambda("call2ul", "%ul%ul", arg_list, V_NLEAF, insn2, sizeof insn2);
	reg = v_scallul((v_ulptr)ip, "%ul%ul", arg_list[0],arg_list[1]);
	v_retul(reg);
	ip2 = v_end(0).i;
	vdemand2(s2ul == ((unsigned long(*)(unsigned long,unsigned long))ip2)(s1ul,s2ul), call2ul failed);

	v_lambda("param3ul", "%ul%ul%ul", arg_list, V_LEAF, insn, sizeof insn);
	v_retul(arg_list[3-1]);
	ip = v_end(0).i;
	vdemand(s2ul == ((unsigned long(*)(unsigned long,unsigned long,unsigned long))ip)(s1ul,s1ul,s2ul), param3ul failed);

	v_lambda("call3ul", "%ul%ul%ul", arg_list, V_NLEAF, insn2, sizeof insn2);
	reg = v_scallul((v_ulptr)ip, "%ul%ul%ul", arg_list[0],arg_list[1],arg_list[2]);
	v_retul(reg);
	ip2 = v_end(0).i;
	vdemand2(s2ul == ((unsigned long(*)(unsigned long,unsigned long,unsigned long))ip2)(s1ul,s1ul,s2ul), call3ul failed);

	v_lambda("param4ul", "%ul%ul%ul%ul", arg_list, V_LEAF, insn, sizeof insn);
	v_retul(arg_list[4-1]);
	ip = v_end(0).i;
	vdemand(s2ul == ((unsigned long(*)(unsigned long,unsigned long,unsigned long,unsigned long))ip)(s1ul,s1ul,s1ul,s2ul), param4ul failed);

	v_lambda("call4ul", "%ul%ul%ul%ul", arg_list, V_NLEAF, insn2, sizeof insn2);
	reg = v_scallul((v_ulptr)ip, "%ul%ul%ul%ul", arg_list[0],arg_list[1],arg_list[2],arg_list[3]);
	v_retul(reg);
	ip2 = v_end(0).i;
	vdemand2(s2ul == ((unsigned long(*)(unsigned long,unsigned long,unsigned long,unsigned long))ip2)(s1ul,s1ul,s1ul,s2ul), call4ul failed);

	v_lambda("param5ul", "%ul%ul%ul%ul%ul", arg_list, V_LEAF, insn, sizeof insn);
	v_retul(arg_list[5-1]);
	ip = v_end(0).i;
	vdemand(s2ul == ((unsigned long(*)(unsigned long,unsigned long,unsigned long,unsigned long,unsigned long))ip)(s1ul,s1ul,s1ul,s1ul,s2ul), param5ul failed);

	v_lambda("call5ul", "%ul%ul%ul%ul%ul", arg_list, V_NLEAF, insn2, sizeof insn2);
	reg = v_scallul((v_ulptr)ip, "%ul%ul%ul%ul%ul", arg_list[0],arg_list[1],arg_list[2],arg_list[3],arg_list[4]);
	v_retul(reg);
	ip2 = v_end(0).i;
	vdemand2(s2ul == ((unsigned long(*)(unsigned long,unsigned long,unsigned long,unsigned long,unsigned long))ip2)(s1ul,s1ul,s1ul,s1ul,s2ul), call5ul failed);

	v_lambda("param6ul", "%ul%ul%ul%ul%ul%ul", arg_list, V_LEAF, insn, sizeof insn);
	v_retul(arg_list[6-1]);
	ip = v_end(0).i;
	vdemand(s2ul == ((unsigned long(*)(unsigned long,unsigned long,unsigned long,unsigned long,unsigned long,unsigned long))ip)(s1ul,s1ul,s1ul,s1ul,s1ul,s2ul), param6ul failed);

	v_lambda("call6ul", "%ul%ul%ul%ul%ul%ul", arg_list, V_NLEAF, insn2, sizeof insn2);
	reg = v_scallul((v_ulptr)ip, "%ul%ul%ul%ul%ul%ul", arg_list[0],arg_list[1],arg_list[2],arg_list[3],arg_list[4],arg_list[5]);
	v_retul(reg);
	ip2 = v_end(0).i;
	vdemand2(s2ul == ((unsigned long(*)(unsigned long,unsigned long,unsigned long,unsigned long,unsigned long,unsigned long))ip2)(s1ul,s1ul,s1ul,s1ul,s1ul,s2ul), call6ul failed);

	v_lambda("param1f", "%f", arg_list, V_LEAF, insn, sizeof insn);
	v_retf(arg_list[1-1]);
	ip = v_end(0).i;
	vdemand(s2f == ((float(*)(float))ip)(s2f), param1f failed);

	v_lambda("call1f", "%f", arg_list, V_NLEAF, insn2, sizeof insn2);
	reg = v_scallf((v_fptr)ip, "%f", arg_list[0]);
	v_retf(reg);
	ip2 = v_end(0).i;
	vdemand2(s2f == ((float(*)(float))ip2)(s2f), call1f failed);

	v_lambda("param2f", "%f%f", arg_list, V_LEAF, insn, sizeof insn);
	v_retf(arg_list[2-1]);
	ip = v_end(0).i;
	vdemand(s2f == ((float(*)(float,float))ip)(s1f,s2f), param2f failed);

	v_lambda("call2f", "%f%f", arg_list, V_NLEAF, insn2, sizeof insn2);
	reg = v_scallf((v_fptr)ip, "%f%f", arg_list[0],arg_list[1]);
	v_retf(reg);
	ip2 = v_end(0).i;
	vdemand2(s2f == ((float(*)(float,float))ip2)(s1f,s2f), call2f failed);

	v_lambda("param3f", "%f%f%f", arg_list, V_LEAF, insn, sizeof insn);
	v_retf(arg_list[3-1]);
	ip = v_end(0).i;
	vdemand(s2f == ((float(*)(float,float,float))ip)(s1f,s1f,s2f), param3f failed);

	v_lambda("call3f", "%f%f%f", arg_list, V_NLEAF, insn2, sizeof insn2);
	reg = v_scallf((v_fptr)ip, "%f%f%f", arg_list[0],arg_list[1],arg_list[2]);
	v_retf(reg);
	ip2 = v_end(0).i;
	vdemand2(s2f == ((float(*)(float,float,float))ip2)(s1f,s1f,s2f), call3f failed);

	v_lambda("param4f", "%f%f%f%f", arg_list, V_LEAF, insn, sizeof insn);
	v_retf(arg_list[4-1]);
	ip = v_end(0).i;
	vdemand(s2f == ((float(*)(float,float,float,float))ip)(s1f,s1f,s1f,s2f), param4f failed);

	v_lambda("call4f", "%f%f%f%f", arg_list, V_NLEAF, insn2, sizeof insn2);
	reg = v_scallf((v_fptr)ip, "%f%f%f%f", arg_list[0],arg_list[1],arg_list[2],arg_list[3]);
	v_retf(reg);
	ip2 = v_end(0).i;
	vdemand2(s2f == ((float(*)(float,float,float,float))ip2)(s1f,s1f,s1f,s2f), call4f failed);

	v_lambda("param5f", "%f%f%f%f%f", arg_list, V_LEAF, insn, sizeof insn);
	v_retf(arg_list[5-1]);
	ip = v_end(0).i;
	vdemand(s2f == ((float(*)(float,float,float,float,float))ip)(s1f,s1f,s1f,s1f,s2f), param5f failed);

	v_lambda("call5f", "%f%f%f%f%f", arg_list, V_NLEAF, insn2, sizeof insn2);
	reg = v_scallf((v_fptr)ip, "%f%f%f%f%f", arg_list[0],arg_list[1],arg_list[2],arg_list[3],arg_list[4]);
	v_retf(reg);
	ip2 = v_end(0).i;
	vdemand2(s2f == ((float(*)(float,float,float,float,float))ip2)(s1f,s1f,s1f,s1f,s2f), call5f failed);

	v_lambda("param6f", "%f%f%f%f%f%f", arg_list, V_LEAF, insn, sizeof insn);
	v_retf(arg_list[6-1]);
	ip = v_end(0).i;
	vdemand(s2f == ((float(*)(float,float,float,float,float,float))ip)(s1f,s1f,s1f,s1f,s1f,s2f), param6f failed);

	v_lambda("call6f", "%f%f%f%f%f%f", arg_list, V_NLEAF, insn2, sizeof insn2);
	reg = v_scallf((v_fptr)ip, "%f%f%f%f%f%f", arg_list[0],arg_list[1],arg_list[2],arg_list[3],arg_list[4],arg_list[5]);
	v_retf(reg);
	ip2 = v_end(0).i;
	vdemand2(s2f == ((float(*)(float,float,float,float,float,float))ip2)(s1f,s1f,s1f,s1f,s1f,s2f), call6f failed);

	v_lambda("param1d", "%d", arg_list, V_LEAF, insn, sizeof insn);
	v_retd(arg_list[1-1]);
	ip = v_end(0).i;
	vdemand(s2d == ((double(*)(double))ip)(s2d), param1d failed);

	v_lambda("call1d", "%d", arg_list, V_NLEAF, insn2, sizeof insn2);
	reg = v_scalld((v_dptr)ip, "%d", arg_list[0]);
	v_retd(reg);
	ip2 = v_end(0).i;
	vdemand2(s2d == ((double(*)(double))ip2)(s2d), call1d failed);

	v_lambda("param2d", "%d%d", arg_list, V_LEAF, insn, sizeof insn);
	v_retd(arg_list[2-1]);
	ip = v_end(0).i;
	vdemand(s2d == ((double(*)(double,double))ip)(s1d,s2d), param2d failed);

	v_lambda("call2d", "%d%d", arg_list, V_NLEAF, insn2, sizeof insn2);
	reg = v_scalld((v_dptr)ip, "%d%d", arg_list[0],arg_list[1]);
	v_retd(reg);
	ip2 = v_end(0).i;
	vdemand2(s2d == ((double(*)(double,double))ip2)(s1d,s2d), call2d failed);

	v_lambda("param3d", "%d%d%d", arg_list, V_LEAF, insn, sizeof insn);
	v_retd(arg_list[3-1]);
	ip = v_end(0).i;
	vdemand(s2d == ((double(*)(double,double,double))ip)(s1d,s1d,s2d), param3d failed);

	v_lambda("call3d", "%d%d%d", arg_list, V_NLEAF, insn2, sizeof insn2);
	reg = v_scalld((v_dptr)ip, "%d%d%d", arg_list[0],arg_list[1],arg_list[2]);
	v_retd(reg);
	ip2 = v_end(0).i;
	vdemand2(s2d == ((double(*)(double,double,double))ip2)(s1d,s1d,s2d), call3d failed);

	v_lambda("param4d", "%d%d%d%d", arg_list, V_LEAF, insn, sizeof insn);
	v_retd(arg_list[4-1]);
	ip = v_end(0).i;
	vdemand(s2d == ((double(*)(double,double,double,double))ip)(s1d,s1d,s1d,s2d), param4d failed);

	v_lambda("call4d", "%d%d%d%d", arg_list, V_NLEAF, insn2, sizeof insn2);
	reg = v_scalld((v_dptr)ip, "%d%d%d%d", arg_list[0],arg_list[1],arg_list[2],arg_list[3]);
	v_retd(reg);
	ip2 = v_end(0).i;
	vdemand2(s2d == ((double(*)(double,double,double,double))ip2)(s1d,s1d,s1d,s2d), call4d failed);

	v_lambda("param5d", "%d%d%d%d%d", arg_list, V_LEAF, insn, sizeof insn);
	v_retd(arg_list[5-1]);
	ip = v_end(0).i;
	vdemand(s2d == ((double(*)(double,double,double,double,double))ip)(s1d,s1d,s1d,s1d,s2d), param5d failed);

	v_lambda("call5d", "%d%d%d%d%d", arg_list, V_NLEAF, insn2, sizeof insn2);
	reg = v_scalld((v_dptr)ip, "%d%d%d%d%d", arg_list[0],arg_list[1],arg_list[2],arg_list[3],arg_list[4]);
	v_retd(reg);
	ip2 = v_end(0).i;
	vdemand2(s2d == ((double(*)(double,double,double,double,double))ip2)(s1d,s1d,s1d,s1d,s2d), call5d failed);

	v_lambda("param6d", "%d%d%d%d%d%d", arg_list, V_LEAF, insn, sizeof insn);
	v_retd(arg_list[6-1]);
	ip = v_end(0).i;
	vdemand(s2d == ((double(*)(double,double,double,double,double,double))ip)(s1d,s1d,s1d,s1d,s1d,s2d), param6d failed);

	v_lambda("call6d", "%d%d%d%d%d%d", arg_list, V_NLEAF, insn2, sizeof insn2);
	reg = v_scalld((v_dptr)ip, "%d%d%d%d%d%d", arg_list[0],arg_list[1],arg_list[2],arg_list[3],arg_list[4],arg_list[5]);
	v_retd(reg);
	ip2 = v_end(0).i;
	vdemand2(s2d == ((double(*)(double,double,double,double,double,double))ip2)(s1d,s1d,s1d,s1d,s1d,s2d), call6d failed);

		v_lambda("param3%u%u%ul", "%u%u%ul", arg_list, V_LEAF, insn, sizeof insn);
		v_retul(arg_list[3-1]);
		ip = v_end(0).i;
		vdemand(s2ul == ((unsigned long(*)(unsigned,unsigned,unsigned long))ip)(s1u,s1u,s2ul), param3%u%u%ul failed);

		v_lambda("call3%u%u%ul", "%u%u%ul", arg_list, V_NLEAF, insn2, sizeof insn2);
		reg = v_scallul((v_ulptr)ip, "%u%u%ul", arg_list[0],arg_list[1],arg_list[2]);
		v_retul(reg);
		ip2 = v_end(0).i;
		vdemand2(s2ul == ((unsigned long(*)(unsigned,unsigned,unsigned long))ip2)(s1u,s1u,s2ul), call3ul failed);


		v_lambda("param5%u%f%u%l%i", "%u%f%u%l%i", arg_list, V_LEAF, insn, sizeof insn);
		v_reti(arg_list[5-1]);
		ip = v_end(0).i;
		vdemand(s2i == ((int(*)(unsigned,float,unsigned,long,int))ip)(s1u,s1f,s1u,s1l,s2i), param5%u%f%u%l%i failed);

		v_lambda("call5%u%f%u%l%i", "%u%f%u%l%i", arg_list, V_NLEAF, insn2, sizeof insn2);
		reg = v_scalli((v_iptr)ip, "%u%f%u%l%i", arg_list[0],arg_list[1],arg_list[2],arg_list[3],arg_list[4]);
		v_reti(reg);
		ip2 = v_end(0).i;
		vdemand2(s2i == ((int(*)(unsigned,float,unsigned,long,int))ip2)(s1u,s1f,s1u,s1l,s2i), call5i failed);


		v_lambda("param1%l", "%l", arg_list, V_LEAF, insn, sizeof insn);
		v_retl(arg_list[1-1]);
		ip = v_end(0).i;
		vdemand(s2l == ((long(*)(long))ip)(s2l), param1%l failed);

		v_lambda("call1%l", "%l", arg_list, V_NLEAF, insn2, sizeof insn2);
		reg = v_scalll((v_lptr)ip, "%l", arg_list[0]);
		v_retl(reg);
		ip2 = v_end(0).i;
		vdemand2(s2l == ((long(*)(long))ip2)(s2l), call1l failed);


		v_lambda("param2%l%d", "%l%d", arg_list, V_LEAF, insn, sizeof insn);
		v_retd(arg_list[2-1]);
		ip = v_end(0).i;
		vdemand(s2d == ((double(*)(long,double))ip)(s1l,s2d), param2%l%d failed);

		v_lambda("call2%l%d", "%l%d", arg_list, V_NLEAF, insn2, sizeof insn2);
		reg = v_scalld((v_dptr)ip, "%l%d", arg_list[0],arg_list[1]);
		v_retd(reg);
		ip2 = v_end(0).i;
		vdemand2(s2d == ((double(*)(long,double))ip2)(s1l,s2d), call2d failed);


		v_lambda("param3%f%ul%f", "%f%ul%f", arg_list, V_LEAF, insn, sizeof insn);
		v_retf(arg_list[3-1]);
		ip = v_end(0).i;
		vdemand(s2f == ((float(*)(float,unsigned long,float))ip)(s1f,s1ul,s2f), param3%f%ul%f failed);

		v_lambda("call3%f%ul%f", "%f%ul%f", arg_list, V_NLEAF, insn2, sizeof insn2);
		reg = v_scallf((v_fptr)ip, "%f%ul%f", arg_list[0],arg_list[1],arg_list[2]);
		v_retf(reg);
		ip2 = v_end(0).i;
		vdemand2(s2f == ((float(*)(float,unsigned long,float))ip2)(s1f,s1ul,s2f), call3f failed);


		v_lambda("param4%f%i%ul%u", "%f%i%ul%u", arg_list, V_LEAF, insn, sizeof insn);
		v_retu(arg_list[4-1]);
		ip = v_end(0).i;
		vdemand(s2u == ((unsigned(*)(float,int,unsigned long,unsigned))ip)(s1f,s1i,s1ul,s2u), param4%f%i%ul%u failed);

		v_lambda("call4%f%i%ul%u", "%f%i%ul%u", arg_list, V_NLEAF, insn2, sizeof insn2);
		reg = v_scallu((v_uptr)ip, "%f%i%ul%u", arg_list[0],arg_list[1],arg_list[2],arg_list[3]);
		v_retu(reg);
		ip2 = v_end(0).i;
		vdemand2(s2u == ((unsigned(*)(float,int,unsigned long,unsigned))ip2)(s1f,s1i,s1ul,s2u), call4u failed);


		v_lambda("param2%d%ul", "%d%ul", arg_list, V_LEAF, insn, sizeof insn);
		v_retul(arg_list[2-1]);
		ip = v_end(0).i;
		vdemand(s2ul == ((unsigned long(*)(double,unsigned long))ip)(s1d,s2ul), param2%d%ul failed);

		v_lambda("call2%d%ul", "%d%ul", arg_list, V_NLEAF, insn2, sizeof insn2);
		reg = v_scallul((v_ulptr)ip, "%d%ul", arg_list[0],arg_list[1]);
		v_retul(reg);
		ip2 = v_end(0).i;
		vdemand2(s2ul == ((unsigned long(*)(double,unsigned long))ip2)(s1d,s2ul), call2ul failed);


		v_lambda("param3%ul%f%d", "%ul%f%d", arg_list, V_LEAF, insn, sizeof insn);
		v_retd(arg_list[3-1]);
		ip = v_end(0).i;
		vdemand(s2d == ((double(*)(unsigned long,float,double))ip)(s1ul,s1f,s2d), param3%ul%f%d failed);

		v_lambda("call3%ul%f%d", "%ul%f%d", arg_list, V_NLEAF, insn2, sizeof insn2);
		reg = v_scalld((v_dptr)ip, "%ul%f%d", arg_list[0],arg_list[1],arg_list[2]);
		v_retd(reg);
		ip2 = v_end(0).i;
		vdemand2(s2d == ((double(*)(unsigned long,float,double))ip2)(s1ul,s1f,s2d), call3d failed);


		v_lambda("param5%d%f%f%ul%l", "%d%f%f%ul%l", arg_list, V_LEAF, insn, sizeof insn);
		v_retl(arg_list[5-1]);
		ip = v_end(0).i;
		vdemand(s2l == ((long(*)(double,float,float,unsigned long,long))ip)(s1d,s1f,s1f,s1ul,s2l), param5%d%f%f%ul%l failed);

		v_lambda("call5%d%f%f%ul%l", "%d%f%f%ul%l", arg_list, V_NLEAF, insn2, sizeof insn2);
		reg = v_scalll((v_lptr)ip, "%d%f%f%ul%l", arg_list[0],arg_list[1],arg_list[2],arg_list[3],arg_list[4]);
		v_retl(reg);
		ip2 = v_end(0).i;
		vdemand2(s2l == ((long(*)(double,float,float,unsigned long,long))ip2)(s1d,s1f,s1f,s1ul,s2l), call5l failed);


		v_lambda("param2%u%f", "%u%f", arg_list, V_LEAF, insn, sizeof insn);
		v_retf(arg_list[2-1]);
		ip = v_end(0).i;
		vdemand(s2f == ((float(*)(unsigned,float))ip)(s1u,s2f), param2%u%f failed);

		v_lambda("call2%u%f", "%u%f", arg_list, V_NLEAF, insn2, sizeof insn2);
		reg = v_scallf((v_fptr)ip, "%u%f", arg_list[0],arg_list[1]);
		v_retf(reg);
		ip2 = v_end(0).i;
		vdemand2(s2f == ((float(*)(unsigned,float))ip2)(s1u,s2f), call2f failed);


		v_lambda("param3%l%d%i", "%l%d%i", arg_list, V_LEAF, insn, sizeof insn);
		v_reti(arg_list[3-1]);
		ip = v_end(0).i;
		vdemand(s2i == ((int(*)(long,double,int))ip)(s1l,s1d,s2i), param3%l%d%i failed);

		v_lambda("call3%l%d%i", "%l%d%i", arg_list, V_NLEAF, insn2, sizeof insn2);
		reg = v_scalli((v_iptr)ip, "%l%d%i", arg_list[0],arg_list[1],arg_list[2]);
		v_reti(reg);
		ip2 = v_end(0).i;
		vdemand2(s2i == ((int(*)(long,double,int))ip2)(s1l,s1d,s2i), call3i failed);


		v_lambda("param2%d%f", "%d%f", arg_list, V_LEAF, insn, sizeof insn);
		v_retf(arg_list[2-1]);
		ip = v_end(0).i;
		vdemand(s2f == ((float(*)(double,float))ip)(s1d,s2f), param2%d%f failed);

		v_lambda("call2%d%f", "%d%f", arg_list, V_NLEAF, insn2, sizeof insn2);
		reg = v_scallf((v_fptr)ip, "%d%f", arg_list[0],arg_list[1]);
		v_retf(reg);
		ip2 = v_end(0).i;
		vdemand2(s2f == ((float(*)(double,float))ip2)(s1d,s2f), call2f failed);


		v_lambda("param3%f%f%d", "%f%f%d", arg_list, V_LEAF, insn, sizeof insn);
		v_retd(arg_list[3-1]);
		ip = v_end(0).i;
		vdemand(s2d == ((double(*)(float,float,double))ip)(s1f,s1f,s2d), param3%f%f%d failed);

		v_lambda("call3%f%f%d", "%f%f%d", arg_list, V_NLEAF, insn2, sizeof insn2);
		reg = v_scalld((v_dptr)ip, "%f%f%d", arg_list[0],arg_list[1],arg_list[2]);
		v_retd(reg);
		ip2 = v_end(0).i;
		vdemand2(s2d == ((double(*)(float,float,double))ip2)(s1f,s1f,s2d), call3d failed);


		v_lambda("param1%u", "%u", arg_list, V_LEAF, insn, sizeof insn);
		v_retu(arg_list[1-1]);
		ip = v_end(0).i;
		vdemand(s2u == ((unsigned(*)(unsigned))ip)(s2u), param1%u failed);

		v_lambda("call1%u", "%u", arg_list, V_NLEAF, insn2, sizeof insn2);
		reg = v_scallu((v_uptr)ip, "%u", arg_list[0]);
		v_retu(reg);
		ip2 = v_end(0).i;
		vdemand2(s2u == ((unsigned(*)(unsigned))ip2)(s2u), call1u failed);


		v_lambda("param4%u%ul%i%d", "%u%ul%i%d", arg_list, V_LEAF, insn, sizeof insn);
		v_retd(arg_list[4-1]);
		ip = v_end(0).i;
		vdemand(s2d == ((double(*)(unsigned,unsigned long,int,double))ip)(s1u,s1ul,s1i,s2d), param4%u%ul%i%d failed);

		v_lambda("call4%u%ul%i%d", "%u%ul%i%d", arg_list, V_NLEAF, insn2, sizeof insn2);
		reg = v_scalld((v_dptr)ip, "%u%ul%i%d", arg_list[0],arg_list[1],arg_list[2],arg_list[3]);
		v_retd(reg);
		ip2 = v_end(0).i;
		vdemand2(s2d == ((double(*)(unsigned,unsigned long,int,double))ip2)(s1u,s1ul,s1i,s2d), call4d failed);


		v_lambda("param2%i%l", "%i%l", arg_list, V_LEAF, insn, sizeof insn);
		v_retl(arg_list[2-1]);
		ip = v_end(0).i;
		vdemand(s2l == ((long(*)(int,long))ip)(s1i,s2l), param2%i%l failed);

		v_lambda("call2%i%l", "%i%l", arg_list, V_NLEAF, insn2, sizeof insn2);
		reg = v_scalll((v_lptr)ip, "%i%l", arg_list[0],arg_list[1]);
		v_retl(reg);
		ip2 = v_end(0).i;
		vdemand2(s2l == ((long(*)(int,long))ip2)(s1i,s2l), call2l failed);


		v_lambda("param5%ul%i%ul%u%l", "%ul%i%ul%u%l", arg_list, V_LEAF, insn, sizeof insn);
		v_retl(arg_list[5-1]);
		ip = v_end(0).i;
		vdemand(s2l == ((long(*)(unsigned long,int,unsigned long,unsigned,long))ip)(s1ul,s1i,s1ul,s1u,s2l), param5%ul%i%ul%u%l failed);

		v_lambda("call5%ul%i%ul%u%l", "%ul%i%ul%u%l", arg_list, V_NLEAF, insn2, sizeof insn2);
		reg = v_scalll((v_lptr)ip, "%ul%i%ul%u%l", arg_list[0],arg_list[1],arg_list[2],arg_list[3],arg_list[4]);
		v_retl(reg);
		ip2 = v_end(0).i;
		vdemand2(s2l == ((long(*)(unsigned long,int,unsigned long,unsigned,long))ip2)(s1ul,s1i,s1ul,s1u,s2l), call5l failed);


		v_lambda("param2%l%l", "%l%l", arg_list, V_LEAF, insn, sizeof insn);
		v_retl(arg_list[2-1]);
		ip = v_end(0).i;
		vdemand(s2l == ((long(*)(long,long))ip)(s1l,s2l), param2%l%l failed);

		v_lambda("call2%l%l", "%l%l", arg_list, V_NLEAF, insn2, sizeof insn2);
		reg = v_scalll((v_lptr)ip, "%l%l", arg_list[0],arg_list[1]);
		v_retl(reg);
		ip2 = v_end(0).i;
		vdemand2(s2l == ((long(*)(long,long))ip2)(s1l,s2l), call2l failed);


		v_lambda("param5%l%l%l%i%i", "%l%l%l%i%i", arg_list, V_LEAF, insn, sizeof insn);
		v_reti(arg_list[5-1]);
		ip = v_end(0).i;
		vdemand(s2i == ((int(*)(long,long,long,int,int))ip)(s1l,s1l,s1l,s1i,s2i), param5%l%l%l%i%i failed);

		v_lambda("call5%l%l%l%i%i", "%l%l%l%i%i", arg_list, V_NLEAF, insn2, sizeof insn2);
		reg = v_scalli((v_iptr)ip, "%l%l%l%i%i", arg_list[0],arg_list[1],arg_list[2],arg_list[3],arg_list[4]);
		v_reti(reg);
		ip2 = v_end(0).i;
		vdemand2(s2i == ((int(*)(long,long,long,int,int))ip2)(s1l,s1l,s1l,s1i,s2i), call5i failed);


		v_lambda("param3%f%i%l", "%f%i%l", arg_list, V_LEAF, insn, sizeof insn);
		v_retl(arg_list[3-1]);
		ip = v_end(0).i;
		vdemand(s2l == ((long(*)(float,int,long))ip)(s1f,s1i,s2l), param3%f%i%l failed);

		v_lambda("call3%f%i%l", "%f%i%l", arg_list, V_NLEAF, insn2, sizeof insn2);
		reg = v_scalll((v_lptr)ip, "%f%i%l", arg_list[0],arg_list[1],arg_list[2]);
		v_retl(reg);
		ip2 = v_end(0).i;
		vdemand2(s2l == ((long(*)(float,int,long))ip2)(s1f,s1i,s2l), call3l failed);


		v_lambda("param4%u%l%u%u", "%u%l%u%u", arg_list, V_LEAF, insn, sizeof insn);
		v_retu(arg_list[4-1]);
		ip = v_end(0).i;
		vdemand(s2u == ((unsigned(*)(unsigned,long,unsigned,unsigned))ip)(s1u,s1l,s1u,s2u), param4%u%l%u%u failed);

		v_lambda("call4%u%l%u%u", "%u%l%u%u", arg_list, V_NLEAF, insn2, sizeof insn2);
		reg = v_scallu((v_uptr)ip, "%u%l%u%u", arg_list[0],arg_list[1],arg_list[2],arg_list[3]);
		v_retu(reg);
		ip2 = v_end(0).i;
		vdemand2(s2u == ((unsigned(*)(unsigned,long,unsigned,unsigned))ip2)(s1u,s1l,s1u,s2u), call4u failed);


		v_lambda("param2%d%i", "%d%i", arg_list, V_LEAF, insn, sizeof insn);
		v_reti(arg_list[2-1]);
		ip = v_end(0).i;
		vdemand(s2i == ((int(*)(double,int))ip)(s1d,s2i), param2%d%i failed);

		v_lambda("call2%d%i", "%d%i", arg_list, V_NLEAF, insn2, sizeof insn2);
		reg = v_scalli((v_iptr)ip, "%d%i", arg_list[0],arg_list[1]);
		v_reti(reg);
		ip2 = v_end(0).i;
		vdemand2(s2i == ((int(*)(double,int))ip2)(s1d,s2i), call2i failed);


		v_lambda("param4%l%f%f%d", "%l%f%f%d", arg_list, V_LEAF, insn, sizeof insn);
		v_retd(arg_list[4-1]);
		ip = v_end(0).i;
		vdemand(s2d == ((double(*)(long,float,float,double))ip)(s1l,s1f,s1f,s2d), param4%l%f%f%d failed);

		v_lambda("call4%l%f%f%d", "%l%f%f%d", arg_list, V_NLEAF, insn2, sizeof insn2);
		reg = v_scalld((v_dptr)ip, "%l%f%f%d", arg_list[0],arg_list[1],arg_list[2],arg_list[3]);
		v_retd(reg);
		ip2 = v_end(0).i;
		vdemand2(s2d == ((double(*)(long,float,float,double))ip2)(s1l,s1f,s1f,s2d), call4d failed);


		v_lambda("param2%d%d", "%d%d", arg_list, V_LEAF, insn, sizeof insn);
		v_retd(arg_list[2-1]);
		ip = v_end(0).i;
		vdemand(s2d == ((double(*)(double,double))ip)(s1d,s2d), param2%d%d failed);

		v_lambda("call2%d%d", "%d%d", arg_list, V_NLEAF, insn2, sizeof insn2);
		reg = v_scalld((v_dptr)ip, "%d%d", arg_list[0],arg_list[1]);
		v_retd(reg);
		ip2 = v_end(0).i;
		vdemand2(s2d == ((double(*)(double,double))ip2)(s1d,s2d), call2d failed);


		v_lambda("param4%l%f%f%i", "%l%f%f%i", arg_list, V_LEAF, insn, sizeof insn);
		v_reti(arg_list[4-1]);
		ip = v_end(0).i;
		vdemand(s2i == ((int(*)(long,float,float,int))ip)(s1l,s1f,s1f,s2i), param4%l%f%f%i failed);

		v_lambda("call4%l%f%f%i", "%l%f%f%i", arg_list, V_NLEAF, insn2, sizeof insn2);
		reg = v_scalli((v_iptr)ip, "%l%f%f%i", arg_list[0],arg_list[1],arg_list[2],arg_list[3]);
		v_reti(reg);
		ip2 = v_end(0).i;
		vdemand2(s2i == ((int(*)(long,float,float,int))ip2)(s1l,s1f,s1f,s2i), call4i failed);


		v_lambda("param2%i%d", "%i%d", arg_list, V_LEAF, insn, sizeof insn);
		v_retd(arg_list[2-1]);
		ip = v_end(0).i;
		vdemand(s2d == ((double(*)(int,double))ip)(s1i,s2d), param2%i%d failed);

		v_lambda("call2%i%d", "%i%d", arg_list, V_NLEAF, insn2, sizeof insn2);
		reg = v_scalld((v_dptr)ip, "%i%d", arg_list[0],arg_list[1]);
		v_retd(reg);
		ip2 = v_end(0).i;
		vdemand2(s2d == ((double(*)(int,double))ip2)(s1i,s2d), call2d failed);


		v_lambda("param4%u%i%u%f", "%u%i%u%f", arg_list, V_LEAF, insn, sizeof insn);
		v_retf(arg_list[4-1]);
		ip = v_end(0).i;
		vdemand(s2f == ((float(*)(unsigned,int,unsigned,float))ip)(s1u,s1i,s1u,s2f), param4%u%i%u%f failed);

		v_lambda("call4%u%i%u%f", "%u%i%u%f", arg_list, V_NLEAF, insn2, sizeof insn2);
		reg = v_scallf((v_fptr)ip, "%u%i%u%f", arg_list[0],arg_list[1],arg_list[2],arg_list[3]);
		v_retf(reg);
		ip2 = v_end(0).i;
		vdemand2(s2f == ((float(*)(unsigned,int,unsigned,float))ip2)(s1u,s1i,s1u,s2f), call4f failed);


		v_lambda("param2%ul%f", "%ul%f", arg_list, V_LEAF, insn, sizeof insn);
		v_retf(arg_list[2-1]);
		ip = v_end(0).i;
		vdemand(s2f == ((float(*)(unsigned long,float))ip)(s1ul,s2f), param2%ul%f failed);

		v_lambda("call2%ul%f", "%ul%f", arg_list, V_NLEAF, insn2, sizeof insn2);
		reg = v_scallf((v_fptr)ip, "%ul%f", arg_list[0],arg_list[1]);
		v_retf(reg);
		ip2 = v_end(0).i;
		vdemand2(s2f == ((float(*)(unsigned long,float))ip2)(s1ul,s2f), call2f failed);


		v_lambda("param1%d", "%d", arg_list, V_LEAF, insn, sizeof insn);
		v_retd(arg_list[1-1]);
		ip = v_end(0).i;
		vdemand(s2d == ((double(*)(double))ip)(s2d), param1%d failed);

		v_lambda("call1%d", "%d", arg_list, V_NLEAF, insn2, sizeof insn2);
		reg = v_scalld((v_dptr)ip, "%d", arg_list[0]);
		v_retd(reg);
		ip2 = v_end(0).i;
		vdemand2(s2d == ((double(*)(double))ip2)(s2d), call1d failed);


		v_lambda("param2%l%f", "%l%f", arg_list, V_LEAF, insn, sizeof insn);
		v_retf(arg_list[2-1]);
		ip = v_end(0).i;
		vdemand(s2f == ((float(*)(long,float))ip)(s1l,s2f), param2%l%f failed);

		v_lambda("call2%l%f", "%l%f", arg_list, V_NLEAF, insn2, sizeof insn2);
		reg = v_scallf((v_fptr)ip, "%l%f", arg_list[0],arg_list[1]);
		v_retf(reg);
		ip2 = v_end(0).i;
		vdemand2(s2f == ((float(*)(long,float))ip2)(s1l,s2f), call2f failed);


		v_lambda("param5%d%d%l%l%i", "%d%d%l%l%i", arg_list, V_LEAF, insn, sizeof insn);
		v_reti(arg_list[5-1]);
		ip = v_end(0).i;
		vdemand(s2i == ((int(*)(double,double,long,long,int))ip)(s1d,s1d,s1l,s1l,s2i), param5%d%d%l%l%i failed);

		v_lambda("call5%d%d%l%l%i", "%d%d%l%l%i", arg_list, V_NLEAF, insn2, sizeof insn2);
		reg = v_scalli((v_iptr)ip, "%d%d%l%l%i", arg_list[0],arg_list[1],arg_list[2],arg_list[3],arg_list[4]);
		v_reti(reg);
		ip2 = v_end(0).i;
		vdemand2(s2i == ((int(*)(double,double,long,long,int))ip2)(s1d,s1d,s1l,s1l,s2i), call5i failed);


		v_lambda("param3%u%d%ul", "%u%d%ul", arg_list, V_LEAF, insn, sizeof insn);
		v_retul(arg_list[3-1]);
		ip = v_end(0).i;
		vdemand(s2ul == ((unsigned long(*)(unsigned,double,unsigned long))ip)(s1u,s1d,s2ul), param3%u%d%ul failed);

		v_lambda("call3%u%d%ul", "%u%d%ul", arg_list, V_NLEAF, insn2, sizeof insn2);
		reg = v_scallul((v_ulptr)ip, "%u%d%ul", arg_list[0],arg_list[1],arg_list[2]);
		v_retul(reg);
		ip2 = v_end(0).i;
		vdemand2(s2ul == ((unsigned long(*)(unsigned,double,unsigned long))ip2)(s1u,s1d,s2ul), call3ul failed);


		v_lambda("param4%u%l%f%i", "%u%l%f%i", arg_list, V_LEAF, insn, sizeof insn);
		v_reti(arg_list[4-1]);
		ip = v_end(0).i;
		vdemand(s2i == ((int(*)(unsigned,long,float,int))ip)(s1u,s1l,s1f,s2i), param4%u%l%f%i failed);

		v_lambda("call4%u%l%f%i", "%u%l%f%i", arg_list, V_NLEAF, insn2, sizeof insn2);
		reg = v_scalli((v_iptr)ip, "%u%l%f%i", arg_list[0],arg_list[1],arg_list[2],arg_list[3]);
		v_reti(reg);
		ip2 = v_end(0).i;
		vdemand2(s2i == ((int(*)(unsigned,long,float,int))ip2)(s1u,s1l,s1f,s2i), call4i failed);


		v_lambda("param3%i%u%d", "%i%u%d", arg_list, V_LEAF, insn, sizeof insn);
		v_retd(arg_list[3-1]);
		ip = v_end(0).i;
		vdemand(s2d == ((double(*)(int,unsigned,double))ip)(s1i,s1u,s2d), param3%i%u%d failed);

		v_lambda("call3%i%u%d", "%i%u%d", arg_list, V_NLEAF, insn2, sizeof insn2);
		reg = v_scalld((v_dptr)ip, "%i%u%d", arg_list[0],arg_list[1],arg_list[2]);
		v_retd(reg);
		ip2 = v_end(0).i;
		vdemand2(s2d == ((double(*)(int,unsigned,double))ip2)(s1i,s1u,s2d), call3d failed);


		v_lambda("param4%d%l%f%f", "%d%l%f%f", arg_list, V_LEAF, insn, sizeof insn);
		v_retf(arg_list[4-1]);
		ip = v_end(0).i;
		vdemand(s2f == ((float(*)(double,long,float,float))ip)(s1d,s1l,s1f,s2f), param4%d%l%f%f failed);

		v_lambda("call4%d%l%f%f", "%d%l%f%f", arg_list, V_NLEAF, insn2, sizeof insn2);
		reg = v_scallf((v_fptr)ip, "%d%l%f%f", arg_list[0],arg_list[1],arg_list[2],arg_list[3]);
		v_retf(reg);
		ip2 = v_end(0).i;
		vdemand2(s2f == ((float(*)(double,long,float,float))ip2)(s1d,s1l,s1f,s2f), call4f failed);


		v_lambda("param2%l%ul", "%l%ul", arg_list, V_LEAF, insn, sizeof insn);
		v_retul(arg_list[2-1]);
		ip = v_end(0).i;
		vdemand(s2ul == ((unsigned long(*)(long,unsigned long))ip)(s1l,s2ul), param2%l%ul failed);

		v_lambda("call2%l%ul", "%l%ul", arg_list, V_NLEAF, insn2, sizeof insn2);
		reg = v_scallul((v_ulptr)ip, "%l%ul", arg_list[0],arg_list[1]);
		v_retul(reg);
		ip2 = v_end(0).i;
		vdemand2(s2ul == ((unsigned long(*)(long,unsigned long))ip2)(s1l,s2ul), call2ul failed);


		v_lambda("param1%ul", "%ul", arg_list, V_LEAF, insn, sizeof insn);
		v_retul(arg_list[1-1]);
		ip = v_end(0).i;
		vdemand(s2ul == ((unsigned long(*)(unsigned long))ip)(s2ul), param1%ul failed);

		v_lambda("call1%ul", "%ul", arg_list, V_NLEAF, insn2, sizeof insn2);
		reg = v_scallul((v_ulptr)ip, "%ul", arg_list[0]);
		v_retul(reg);
		ip2 = v_end(0).i;
		vdemand2(s2ul == ((unsigned long(*)(unsigned long))ip2)(s2ul), call1ul failed);


		v_lambda("param2%i%f", "%i%f", arg_list, V_LEAF, insn, sizeof insn);
		v_retf(arg_list[2-1]);
		ip = v_end(0).i;
		vdemand(s2f == ((float(*)(int,float))ip)(s1i,s2f), param2%i%f failed);

		v_lambda("call2%i%f", "%i%f", arg_list, V_NLEAF, insn2, sizeof insn2);
		reg = v_scallf((v_fptr)ip, "%i%f", arg_list[0],arg_list[1]);
		v_retf(reg);
		ip2 = v_end(0).i;
		vdemand2(s2f == ((float(*)(int,float))ip2)(s1i,s2f), call2f failed);


		v_lambda("param4%l%f%i%d", "%l%f%i%d", arg_list, V_LEAF, insn, sizeof insn);
		v_retd(arg_list[4-1]);
		ip = v_end(0).i;
		vdemand(s2d == ((double(*)(long,float,int,double))ip)(s1l,s1f,s1i,s2d), param4%l%f%i%d failed);

		v_lambda("call4%l%f%i%d", "%l%f%i%d", arg_list, V_NLEAF, insn2, sizeof insn2);
		reg = v_scalld((v_dptr)ip, "%l%f%i%d", arg_list[0],arg_list[1],arg_list[2],arg_list[3]);
		v_retd(reg);
		ip2 = v_end(0).i;
		vdemand2(s2d == ((double(*)(long,float,int,double))ip2)(s1l,s1f,s1i,s2d), call4d failed);


		v_lambda("param3%f%ul%ul", "%f%ul%ul", arg_list, V_LEAF, insn, sizeof insn);
		v_retul(arg_list[3-1]);
		ip = v_end(0).i;
		vdemand(s2ul == ((unsigned long(*)(float,unsigned long,unsigned long))ip)(s1f,s1ul,s2ul), param3%f%ul%ul failed);

		v_lambda("call3%f%ul%ul", "%f%ul%ul", arg_list, V_NLEAF, insn2, sizeof insn2);
		reg = v_scallul((v_ulptr)ip, "%f%ul%ul", arg_list[0],arg_list[1],arg_list[2]);
		v_retul(reg);
		ip2 = v_end(0).i;
		vdemand2(s2ul == ((unsigned long(*)(float,unsigned long,unsigned long))ip2)(s1f,s1ul,s2ul), call3ul failed);


		v_lambda("param4%u%u%u%f", "%u%u%u%f", arg_list, V_LEAF, insn, sizeof insn);
		v_retf(arg_list[4-1]);
		ip = v_end(0).i;
		vdemand(s2f == ((float(*)(unsigned,unsigned,unsigned,float))ip)(s1u,s1u,s1u,s2f), param4%u%u%u%f failed);

		v_lambda("call4%u%u%u%f", "%u%u%u%f", arg_list, V_NLEAF, insn2, sizeof insn2);
		reg = v_scallf((v_fptr)ip, "%u%u%u%f", arg_list[0],arg_list[1],arg_list[2],arg_list[3]);
		v_retf(reg);
		ip2 = v_end(0).i;
		vdemand2(s2f == ((float(*)(unsigned,unsigned,unsigned,float))ip2)(s1u,s1u,s1u,s2f), call4f failed);


		v_lambda("param2%ul%l", "%ul%l", arg_list, V_LEAF, insn, sizeof insn);
		v_retl(arg_list[2-1]);
		ip = v_end(0).i;
		vdemand(s2l == ((long(*)(unsigned long,long))ip)(s1ul,s2l), param2%ul%l failed);

		v_lambda("call2%ul%l", "%ul%l", arg_list, V_NLEAF, insn2, sizeof insn2);
		reg = v_scalll((v_lptr)ip, "%ul%l", arg_list[0],arg_list[1]);
		v_retl(reg);
		ip2 = v_end(0).i;
		vdemand2(s2l == ((long(*)(unsigned long,long))ip2)(s1ul,s2l), call2l failed);


		v_lambda("param2%l%ul", "%l%ul", arg_list, V_LEAF, insn, sizeof insn);
		v_retul(arg_list[2-1]);
		ip = v_end(0).i;
		vdemand(s2ul == ((unsigned long(*)(long,unsigned long))ip)(s1l,s2ul), param2%l%ul failed);

		v_lambda("call2%l%ul", "%l%ul", arg_list, V_NLEAF, insn2, sizeof insn2);
		reg = v_scallul((v_ulptr)ip, "%l%ul", arg_list[0],arg_list[1]);
		v_retul(reg);
		ip2 = v_end(0).i;
		vdemand2(s2ul == ((unsigned long(*)(long,unsigned long))ip2)(s1l,s2ul), call2ul failed);


		v_lambda("param3%ul%l%i", "%ul%l%i", arg_list, V_LEAF, insn, sizeof insn);
		v_reti(arg_list[3-1]);
		ip = v_end(0).i;
		vdemand(s2i == ((int(*)(unsigned long,long,int))ip)(s1ul,s1l,s2i), param3%ul%l%i failed);

		v_lambda("call3%ul%l%i", "%ul%l%i", arg_list, V_NLEAF, insn2, sizeof insn2);
		reg = v_scalli((v_iptr)ip, "%ul%l%i", arg_list[0],arg_list[1],arg_list[2]);
		v_reti(reg);
		ip2 = v_end(0).i;
		vdemand2(s2i == ((int(*)(unsigned long,long,int))ip2)(s1ul,s1l,s2i), call3i failed);


		v_lambda("param2%i%d", "%i%d", arg_list, V_LEAF, insn, sizeof insn);
		v_retd(arg_list[2-1]);
		ip = v_end(0).i;
		vdemand(s2d == ((double(*)(int,double))ip)(s1i,s2d), param2%i%d failed);

		v_lambda("call2%i%d", "%i%d", arg_list, V_NLEAF, insn2, sizeof insn2);
		reg = v_scalld((v_dptr)ip, "%i%d", arg_list[0],arg_list[1]);
		v_retd(reg);
		ip2 = v_end(0).i;
		vdemand2(s2d == ((double(*)(int,double))ip2)(s1i,s2d), call2d failed);


		v_lambda("param1%l", "%l", arg_list, V_LEAF, insn, sizeof insn);
		v_retl(arg_list[1-1]);
		ip = v_end(0).i;
		vdemand(s2l == ((long(*)(long))ip)(s2l), param1%l failed);

		v_lambda("call1%l", "%l", arg_list, V_NLEAF, insn2, sizeof insn2);
		reg = v_scalll((v_lptr)ip, "%l", arg_list[0]);
		v_retl(reg);
		ip2 = v_end(0).i;
		vdemand2(s2l == ((long(*)(long))ip2)(s2l), call1l failed);


		v_lambda("param4%l%ul%l%ul", "%l%ul%l%ul", arg_list, V_LEAF, insn, sizeof insn);
		v_retul(arg_list[4-1]);
		ip = v_end(0).i;
		vdemand(s2ul == ((unsigned long(*)(long,unsigned long,long,unsigned long))ip)(s1l,s1ul,s1l,s2ul), param4%l%ul%l%ul failed);

		v_lambda("call4%l%ul%l%ul", "%l%ul%l%ul", arg_list, V_NLEAF, insn2, sizeof insn2);
		reg = v_scallul((v_ulptr)ip, "%l%ul%l%ul", arg_list[0],arg_list[1],arg_list[2],arg_list[3]);
		v_retul(reg);
		ip2 = v_end(0).i;
		vdemand2(s2ul == ((unsigned long(*)(long,unsigned long,long,unsigned long))ip2)(s1l,s1ul,s1l,s2ul), call4ul failed);


		v_lambda("param3%i%ul%u", "%i%ul%u", arg_list, V_LEAF, insn, sizeof insn);
		v_retu(arg_list[3-1]);
		ip = v_end(0).i;
		vdemand(s2u == ((unsigned(*)(int,unsigned long,unsigned))ip)(s1i,s1ul,s2u), param3%i%ul%u failed);

		v_lambda("call3%i%ul%u", "%i%ul%u", arg_list, V_NLEAF, insn2, sizeof insn2);
		reg = v_scallu((v_uptr)ip, "%i%ul%u", arg_list[0],arg_list[1],arg_list[2]);
		v_retu(reg);
		ip2 = v_end(0).i;
		vdemand2(s2u == ((unsigned(*)(int,unsigned long,unsigned))ip2)(s1i,s1ul,s2u), call3u failed);


		v_lambda("param3%u%d%d", "%u%d%d", arg_list, V_LEAF, insn, sizeof insn);
		v_retd(arg_list[3-1]);
		ip = v_end(0).i;
		vdemand(s2d == ((double(*)(unsigned,double,double))ip)(s1u,s1d,s2d), param3%u%d%d failed);

		v_lambda("call3%u%d%d", "%u%d%d", arg_list, V_NLEAF, insn2, sizeof insn2);
		reg = v_scalld((v_dptr)ip, "%u%d%d", arg_list[0],arg_list[1],arg_list[2]);
		v_retd(reg);
		ip2 = v_end(0).i;
		vdemand2(s2d == ((double(*)(unsigned,double,double))ip2)(s1u,s1d,s2d), call3d failed);


		v_lambda("param5%d%ul%ul%u%ul", "%d%ul%ul%u%ul", arg_list, V_LEAF, insn, sizeof insn);
		v_retul(arg_list[5-1]);
		ip = v_end(0).i;
		vdemand(s2ul == ((unsigned long(*)(double,unsigned long,unsigned long,unsigned,unsigned long))ip)(s1d,s1ul,s1ul,s1u,s2ul), param5%d%ul%ul%u%ul failed);

		v_lambda("call5%d%ul%ul%u%ul", "%d%ul%ul%u%ul", arg_list, V_NLEAF, insn2, sizeof insn2);
		reg = v_scallul((v_ulptr)ip, "%d%ul%ul%u%ul", arg_list[0],arg_list[1],arg_list[2],arg_list[3],arg_list[4]);
		v_retul(reg);
		ip2 = v_end(0).i;
		vdemand2(s2ul == ((unsigned long(*)(double,unsigned long,unsigned long,unsigned,unsigned long))ip2)(s1d,s1ul,s1ul,s1u,s2ul), call5ul failed);



	if(!v_errors && iters-- > 0) goto loop;

	if(!v_errors) {
		printf("No errors!
");
		return 0;
	}

	printf("*** %d Errors! ****
", v_errors);
	return 1;
}


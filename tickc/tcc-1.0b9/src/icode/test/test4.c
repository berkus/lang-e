#include "icode.h"

int gg(int a, int b) {
     return a+b;
}
int ff(int a, int b, int c, int d, int e, int f, int g, int h, int i, int j) {
     return a+b+c+d+e+f+g+h+i+j;
}

void a () {
     i_local_t a,b,c,d,e,f,g,h,i,j,w,z,func,cnt;
     i_label_t L1;
     v_iptr ip;
     
     i_init(100);

     L1 = i_mklabel();
/*     func = i_local(0, I_P);*/
     cnt = i_local(0, I_P);
     a = i_local(0, I_I);
     b = i_local(0, I_I);
     c = i_local(0, I_I);
     d = i_local(0, I_I);
     e = i_local(0, I_I);
     f = i_local(0, I_I);
     g = i_local(0, I_I);
     h = i_local(0, I_I);
     i = i_local(0, I_I);
     j = i_local(0, I_I);
     z = i_local(0, I_I);
     w = i_local(0, I_I);

/*     i_setp(func, ff);*/
     i_seti(a, 1);
     i_seti(b, 2);
     i_seti(c, 3);
     i_seti(d, 4);
     i_seti(e, 5);
     i_seti(f, 6);
     i_seti(g, 7);
     i_seti(h, 8);
     i_seti(i, 9);
     i_seti(j, 0);
     i_seti(z, 0);
     i_seti(cnt, 0);
     i_label(L1);
     i_argi(a);
     i_argi(b);
     i_argi(c);
     i_argi(d);
     i_argi(e);
     i_argi(f);
     i_argi(g);
     i_argi(h);
     i_argi(i);
     i_argi(j);
     i_callii(w, ff);
     i_addi(z,z,w);
     i_addii(cnt,cnt,1);
     i_bltii(cnt,2,L1);
     i_addi(z, z, a);
     i_addi(z, z, b);
     i_addi(z, z, c);
     i_addi(z, z, d);
     i_addi(z, z, e);
     i_addi(z, z, f);
     i_addi(z, z, g);
     i_addi(z, z, h);
     i_addi(z, z, i);
     i_addi(z, z, j);
     i_reti(z);			/* (9*10/2)*3 = 135 */
     i_end();

     i_unparse();
     ip = i_emit().i;
     v_dump((void*)ip);
     printf("**135=%d\n", (*ip)());
}

void b () {
     i_local_t f;
     i_label_t L1;
     v_pptr pp;
     
     i_init(100);

     L1 = i_mklabel();
     f = i_local(0, I_P);
     i_setp(f, gg);
     i_nop();
     i_nop();
     i_nop();
     i_nop();
     i_retp(f);
     i_end();

     i_unparse();
     pp = i_emit().p;
     v_dump((void*)pp);
     printf("**3=%d\n", (*(int (*)(int, int))((*pp)()))(1,2));
}

void c () {
     i_local_t a,b,f;
     v_iptr ip;
     
     i_init(100);

     a = i_local(0, I_I);
     b = i_local(0, I_I);
     f = i_local(0, I_I);

     i_setp(f, gg);
     i_seti(a, 1);
     i_seti(b, 2);  
     i_argi(a);
     i_argi(b);
     i_calli(a, f);
     i_reti(a);
     i_end();

     i_unparse();
     ip = i_emit().i;
     v_dump((void*)ip);
     printf("**3=%d\n", (*ip)());
}

void d () {
     i_local_t a,b;
     v_iptr ip;
     
     i_init(100);

     a = i_local(0, I_I);
     b = i_local(0, I_I);

     i_seti(a, 1);
     i_seti(b, 2);  
     i_argi(a);
     i_argi(b);
     i_callii(a, gg);
     i_reti(a);
     i_end();

     i_unparse();
     ip = i_emit().i;
     v_dump((void*)ip);
     printf("**3=%d\n", (*ip)());
}

void e () {
     i_local_t a,b,c,d,e,f,g,h,i,j,k,l;
     int (*ip)(int (*)(int,int),int,int,int,int,int,int,int,int,int);
     
     i_init(100);

     a = i_paramk(I_I,1);
     b = i_paramk(I_I,2);
     c = i_param(I_I);
     d = i_param(I_I);
     e = i_param(I_I);
     g = i_param(I_I);
     h = i_param(I_I);
     i = i_param(I_I);
     j = i_param(I_I);
     k = i_local(0,I_I);
     l = i_local(0,I_I);
     f = i_paramk(I_P,0);

     i_argi(a);
     i_argi(b);
     i_calli(k, f);
     i_argi(c);
     i_argi(d);
     i_calli(d, f);
     i_addi(k,k,d);
     i_argi(e);
     i_argi(g);
     i_calli(g, f);
     i_addi(k,k,g);
     i_argi(h);
     i_argi(i);
     i_calli(i, f);
     i_addi(k,k,i);
     i_addi(k,k,j);
     i_addii(l,k,3);
     i_reti(l);
     i_end();

     i_unparse();
     ip = (int (*)(int (*)(int,int),int,int,int,int,int,int,int,int,int))
	  i_emit().i;
     v_dump((void*)ip);
     printf("**48=%d\n", (*ip)(gg,1,2,3,4,5,6,7,8,9));
}

void main() {
     printf("##########\n");
     a();
#if 1
     printf("##########\n");
     b();
     printf("##########\n");
     c();
     printf("##########\n");
     d();
     printf("##########\n");
     e();
     printf("##########\n");
#endif
}

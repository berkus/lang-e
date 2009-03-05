#include "../icode.h"

void a () {
     i_local_t a, b;
     int c;
     v_iptr ip;
     
     i_init(20);

     a = i_local(0, I_I);
     b = i_local(I_VOLATILE, I_I);

     i_seti(a, 5);
     i_seti(b, 7);
     i_addi(a, a, b); 
     i_reti(a);
     i_end();

     i_unparse();
     ip = i_emit().i;
     v_dump((void*)ip);
     c = (*ip)();
     printf("**12=%d\n", c);
}

void b (int _i, int _c, int _t) {
     i_local_t i, c, t;
     i_label_t L1;
     v_iptr ip;
     
     i_init(20);

     i = i_local(_i, I_I);
     c = i_local(_c, I_I);
     t = i_local(_t, I_I);
     L1 = i_mklabel();

     i_seti(i, 0);
     i_seti(t, 0);
     i_label(L1);
     i_movi(c, i);
     i_addi(t, t, c);
     i_addii(i, i, 1);
     i_bleii(i, 100, L1);
     i_reti(t);
     i_end();

     i_unparse();
     ip = i_emit().i;
     v_dump((void*)ip);
     printf("**5050=%d\n", (*ip)());
}

void c () {
     i_local_t a,b,c,d,e,f,g,h,i,j,k,l,t,w;
     v_iptr ip;
     
     i_init(30);

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
     k = i_local(0, I_I);
     l = i_local(0, I_I);
     t = i_local(0, I_I);
     w = i_local(0, I_I);

     i_seti(a, 1);
     i_seti(b, 2);
     i_seti(c, 3);
     i_seti(d, 4);
     i_seti(e, 5);
     i_seti(f, 6);
     i_seti(g, 7);
     i_seti(h, 8);
     i_seti(i, 9);
     i_seti(j, 10);
     i_seti(k, 11);
     i_seti(l, 12); /* 12*13/2 = 78 */
     i_seti(t, 0);
     i_addi(t, a, b); 
     i_addi(t, t, c); 
     i_addi(t, t, d); 
     i_addi(t, t, e); 
     i_addi(t, t, f); 
     i_addi(t, t, g); 
     i_addi(t, t, h); 
     i_addi(t, t, i); 
     i_addi(t, t, j); 
     i_addi(t, t, k); 
     i_addi(t, t, l); 
     i_reti(t);
     i_end();

     i_unparse();
     ip = i_emit().i;
     v_dump((void*)ip);
     printf("**78=%d\n", (*ip)());
}

void main() {
     printf("##########\n");
     a();
     i_reset(); printf("##########\n");
     b(0,0,0);
     i_reset(); printf("##########\n");
     b(I_VOLATILE,0,0);
     i_reset(); printf("##########\n");
     b(0,I_VOLATILE,0);
     i_reset(); printf("##########\n");
     b(0,0,I_VOLATILE);
     i_reset(); printf("##########\n");
     b(I_VOLATILE,I_VOLATILE,0);
     i_reset(); printf("##########\n");
     b(I_VOLATILE,0,I_VOLATILE);
     i_reset(); printf("##########\n");
     b(0,I_VOLATILE,I_VOLATILE);
     i_reset(); printf("##########\n");
     b(I_VOLATILE,I_VOLATILE,I_VOLATILE);
     i_reset(); printf("##########\n");
     c();
}

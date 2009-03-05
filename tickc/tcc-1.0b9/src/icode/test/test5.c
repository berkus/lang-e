#include "icode.h"

void a () {
     i_local_t a, b, c,d;
     v_vptr ip;
     
     i_init(20);

     a = i_local(0, I_I);
     b = i_local(0, I_F);
     c = i_local(0, I_F);
     d = i_local(0, I_F);

     i_seti(a, 5);
     i_setf(b, 7.5);
     i_setf(c, 4.5);
     i_movf(d, c);
     i_addf(c, c, b); /* c=12 */
     i_cvi2f(b, a);   /* b=5.0 */
     i_addf(d, d, b); /* d=4.5+5.0=9.5 */
     i_addf(c, d, c); /* c=9.5+12=21.5 */
     i_retf(c);
     i_end();

     i_unparse();
     ip = i_emit().v;
     v_dump((void*)ip);
     printf("**21.5=%f\n", (*(v_fptr)ip)());
}

double dsum(int a, float b, double c, float d, int e, double f) {
     return (double)a+(double)b+c+(double)d+(double)e+f;
}

void b () {
     i_local_t a,b,c,d,e,f;
     v_vptr ip;
     
     i_init(20);

     a = i_local(0, I_I);
     b = i_local(0, I_F);
     c = i_local(0, I_D);
     d = i_local(0, I_F);
     e = i_local(0, I_I);
     f = i_local(0, I_D);

     i_seti(a, 1);
     i_setf(b, 2.0);
     i_setd(c, 3.0);
     i_setf(d, 4.0);
     i_seti(e, 5);
     i_setd(f, 6.0);
     i_argi(a);
     i_argf(b);
     i_argd(c);
     i_argf(d);
     i_argi(e);
     i_argd(f);
     i_calldi(f, dsum);
     i_retd(f); /*21.0*/
     i_end();

     i_unparse();
     ip = i_emit().v;
     v_dump((void*)ip);
     printf("**21.0=%f\n", (*(v_dptr)ip)());
}

void main() {
     printf("##########\n");
     a();
     i_reset(); printf("##########\n");
     b();
}

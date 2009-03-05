#include "icode.h"

void a () {
     i_label_t L1, L2;
     i_local_t a, b, c, d, e, f;

     i_init(20);

     L1 = i_mklabel();
     L2 = i_mklabel();

     a = i_local(0);     b = i_local(0);     c = i_local(0);
     d = i_local(0);     e = i_local(0);     f = i_local(0);

     i_seti(a, 1);
     i_seti(b, 1);
     i_addi(c, a, b); 
     i_addi(d, b, a);		/* c = d = 2 */
     i_seti(e, 1);
     i_muli(e, e, 2);		/* e = 2 */
     i_label(L1);
     i_addi(f, d, e);		/* f = 4 ... 10 */
     i_bltii(f, 10, L1);
     i_addii(e, e, 1);
     i_reti(0);
     i_end();

     i_unparse();
     i_buildfg();
     i_unparsefg();
}

void b () {
     i_label_t L1, L2;
     i_local_t a, b, c, d, e, f;

     i_init(1);

     L1 = i_mklabel();
     L2 = i_mklabel();

     a = i_local(0);     b = i_local(0);     c = i_local(0);
     d = i_local(0);     e = i_local(0);     f = i_local(0);

     i_bump(1);
     i_seti(a, 1);
     i_bump(1);
     i_seti(b, 1);
     i_bump(1);
     i_addi(c, a, b); 
     i_bump(1);
     i_addi(d, b, a);		/* c = d = 2 */
     i_bump(1);
     i_seti(e, 1);
     i_bump(1);
     i_muli(e, e, 2);		/* e = 2 */
     i_bump(1);
     i_label(L1);
     i_bump(1);
     i_addi(f, d, e);		/* f = 4 ... 10 */
     i_bump(1);
     i_bltii(f, 10, L1);
     i_bump(1);
     i_addii(e, e, 1);
     i_end();

     i_unparse();
}

void c () {
     i_label_t L0, L1, L2, L3, L4, L5, L6;
     i_local_t a, b, c, d, e, f;

     i_init(30);

     L0 = i_mklabel();     L1 = i_mklabel();     L2 = i_mklabel();
     L3 = i_mklabel();     L4 = i_mklabel();     L5 = i_mklabel();
     L6 = i_mklabel();

     a = i_local(0);     b = i_local(0);     c = i_local(0);
     d = i_local(0);     e = i_local(0);     f = i_local(0);

     i_seti(a, 1);
     i_seti(c, 10);
     i_label(L0);
     i_subi(c, c, a);
     i_mwj(L2);
     i_mwj(L3);
     i_mwj(L4);
     i_mwj(L5);
     i_jp(a);
     i_label(L2);
     i_seti(b, 2);
     i_jpi(L6);
     i_label(L3);
     i_seti(b, 3);
     i_jpi(L6);
     i_label(L4);
     i_seti(b, 4);
     i_jpi(L6);
     i_label(L5);
     i_seti(b, 5);
     i_jpi(L6);
     i_label(L6);
     i_subii(b,b,1);
     i_bgtii(c, 5, L0);
     i_reti(b);
     i_end();

     i_unparse();
     i_buildfg();
     i_livevars();
     i_unparsefg();
}

void d () {
     i_local_t a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z;
     i_local_t aa,bb,cc,dd,ee,ff,gg,hh,ii,jj,kk,ll,mm,nn,oo,pp,qq,rr,ss,tt,
	  uu,vv,ww,xx,yy,zz;
     i_label_t L0, L1, L2, L3, L4, L5;

     i_init(200);

     L0 = i_mklabel();     L1 = i_mklabel();     L2 = i_mklabel();
     L3 = i_mklabel();     L4 = i_mklabel();     L5 = i_mklabel();
     
     a = i_local(0);     b = i_local(0);     c = i_local(0);
     d = i_local(0);     e = i_local(0);     f = i_local(0);
     g = i_local(0);     h = i_local(0);     i = i_local(0);
     j = i_local(0);     k = i_local(0);     l = i_local(0);
     m = i_local(0);     n = i_local(0);     o = i_local(0);
     p = i_local(0);     q = i_local(0);     r = i_local(0);
     s = i_local(0);     t = i_local(0);     u = i_local(0);
     v = i_local(0);     w = i_local(0);     x = i_local(0);
     y = i_local(0);     z = i_local(0);
     aa = i_local(0);     bb = i_local(0);     cc = i_local(0);
     dd = i_local(0);     ee = i_local(0);     ff = i_local(0);
     gg = i_local(0);     hh = i_local(0);     ii = i_local(0);
     jj = i_local(0);     kk = i_local(0);     ll = i_local(0);
     mm = i_local(0);     nn = i_local(0);     oo = i_local(0);
     pp = i_local(0);     qq = i_local(0);     rr = i_local(0);
     ss = i_local(0);     tt = i_local(0);     uu = i_local(0);
     vv = i_local(0);     ww = i_local(0);     xx = i_local(0);
     yy = i_local(0);     zz = i_local(0);

     i_label(L0);
     i_movi(a,a);
     i_movi(b,b);
     i_movi(c,c);
     i_movi(d,d);
     i_movi(e,e);
     i_blei(a,b,L0);
     i_movi(f,f);
     i_movi(g,g);
     i_movi(h,h);
     i_movi(i,i);
     i_movi(j,j);
     i_label(L2);
     i_movi(k,k);
     i_movi(l,l);
     i_movi(m,m);
     i_movi(n,n);
     i_movi(o,o);
     i_movi(p,p);
     i_movi(q,q);
     i_blei(a,b,L1);
     i_movi(r,r);
     i_movi(s,s);
     i_movi(t,t);
     i_movi(u,u);
     i_movi(v,v);
     i_label(L1);
     i_movi(w,w);
     i_movi(x,x);
     i_movi(y,y);
     i_movi(z,z);
     i_blei(a,b,L2);
     i_movi(aa,aa);
     i_movi(bb,bb);
     i_movi(cc,cc);
     i_label(L3);
     i_movi(dd,dd);
     i_movi(ee,ee);
     i_movi(ff,ff);
     i_blei(a,b,L3);
     i_movi(gg,gg);
     i_movi(hh,hh);
     i_movi(ii,ii);
     i_movi(jj,jj);
     i_movi(kk,kk);
     i_movi(ll,ll);
     i_movi(mm,mm);
     i_blei(a,b,L4);
     i_movi(nn,nn);
     i_movi(oo,oo);
     i_movi(pp,pp);
     i_movi(qq,qq);
     i_label(L5);
     i_movi(rr,rr);
     i_movi(ss,ss);
     i_label(L4);
     i_blei(a,b,L5);
     i_movi(tt,tt);
     i_movi(uu,uu);
     i_movi(vv,vv);
     i_movi(ww,ww);
     i_movi(xx,xx);
     i_movi(yy,yy);
     i_movi(zz,zz);
     i_reti(zz);
     i_end();

     i_unparse();
     i_buildfg();
     i_livevars();
     i_unparsefg();
     i_regalloc();
}

void e () {
     i_label_t L1;
     i_local_t a, b, c, d, e, f;

     i_init(20);

     L1 = i_mklabel();

     a = i_local(0);     b = i_local(0);
     c = i_local(0);     d = i_local(0);
     e = i_local(0);     f = i_local(0);

     i_label(L1);
     i_seti(a, 1);
     i_seti(b, 1);
     i_seti(c, 1);
     i_seti(d, 1);
     i_seti(e, 1);
     i_addi(f, a, a);
     i_addi(f, b, b);
     i_addi(f, c, c);
     i_addi(f, d, d);
     i_addi(f, e, e);
     i_bltii(f, 1, L1);
     i_reti(f);
     i_end();

     i_unparse();
     i_buildfg();
     i_livevars();
     i_unparsefg();
     i_regalloc();
}

void f () {
     i_label_t L1;
     i_local_t a, b, c, d, e;

     i_init(20);

     L1 = i_mklabel();

     a = i_local(0);     b = i_local(0);
     c = i_local(0);     d = i_local(0);
     e = i_local(0);

     i_seti(e, 2);
     i_label(L1);
     i_seti(a, 1);
     i_addi(a, a, a);
     i_bltii(a, 1, L1);
     i_seti(b, 2);
     i_addi(b, b, b);
     i_seti(a, 1);
     i_addi(a, a, a);
     i_bltii(a, 1, L1);
     i_seti(c, 2);
     i_addi(c, c, c);
     i_seti(d, 1);
     i_addi(d, d, d);
     i_bltii(d, 1, L1);
     i_addi(e, e, e);
     i_reti(e);
     i_end();

     i_unparse();
     i_buildfg();
     i_livevars();
     i_unparsefg();
     i_regalloc();
}

void main() {
     printf("##########\n");
     a();
     i_reset(); printf("##########\n");
     b();
     i_reset(); printf("##########\n");
     c();
     i_reset(); printf("##########\n");
     d();
     i_reset(); printf("##########\n");
     e();
     i_reset(); printf("##########\n");
     f();
}

#if 0
Function calls and return values: ints, floats, doubles
Literals
Enumerated types
Free variables: global, local
Casts, Typedefs
Type qualifiers
Pointers, Arrays
Aggregates
*Control-flow structures
Composition with cspecs
Composition with vspecs: locals, params
Unquoting, spec conversion
Run-time constants
Optimizations based on run-time constants
Dynamic function call construction
Dynamic jumps/labels
Semantics checks (non-local gotos, decompile)
Decompile
Dynamic functions with many arguments
#endif

/**/

/*
 * FUNCTION CALLS AND RETURN VALUES; LITERALS
 */

#include "test.h"

typedef int (*ip)();
typedef char (*cp)();
typedef float (*fp)();
typedef double (*dp)();

void cspec f(void) {
     return  `{ printf("foo\\\?\"\'\n"); };
}
void simple(void)
{
     ip a; cp b; fp c; dp d;

     printf("EXPECT: foo\\\?\"\'\n");
     (*compile(f(),int))();

     printf("EXPECT: integer 1 character A float %f double %e\n", 2., 3.);
     a = compile(`{printf("integer "); return 1; }, int);
     printf("%d ",(*a)());
     b = compile(`{printf("character "); return 65; }, char);
     printf("%c ",(*b)());
     c = compile(`{printf("float "); return (float)2.0; }, float);
     printf("%f ",(*c)());
     d = compile(`{printf("double "); return (double)3.0; }, double);
     printf("%e\n",(*d)());
}

void ma1(int a1, char b1, long c1, double d1,
	 int a2, char b2, long c2, double d2) { 
     printf("a1%d b1%c c1%d d1%e a2%d b2%c c2%d d2%e\n",
	    a1,b1,c1,d1,a2,b2,c2,d2);
}
void ma2(int* a1, char* b1, long* c1, double* d1,
	 int* a2, char* b2, long* c2, double* d2) { 
     *a1 = 1;  *b1 = 'a'; *c1 = 2; *d1 =3.; 
     *a2 = 2;  *b2 = 'b'; *c2 = 3; *d2 =4.; 
}
void manyargs(void)
{
     void cspec cs = 
	  `{ int a1 = 10; char b1 = 'k'; long c1 = 11; double d1 = 12.;
	     int a2 = 11; char b2 = 'l'; long c2 = 12; double d2 = 13.;
	     ma1(a1,b1,c1,d1,a2,b2,c2,d2);
	     ma2(&a1,&b1,&c1,&d1,&a2,&b2,&c2,&d2);
	     ma1(a1,b1,c1,d1,a2,b2,c2,d2);
	     return -1.;
	};
     dp func = compile(cs, double);  
     printf("EXPECT: "); ma1(10,'k',11,12.,11,'l',12,13.);
     printf("EXPECT: "); ma1(1,'a',2,3.,2,'b',3,4.);
     printf("EXPECT: %e\n", -1.);
     printf("%e\n",(*func)());
}

void main(int argc, char **argv)
{
     doargs(argc, argv);
     simple();
     manyargs();
}

/**/

/*
 * ENUMERATED TYPES
 */

#include "test.h"

enum {a=1,b,c=4};
void main(int argc, char **argv)
{
     int (*f)();
     doargs(argc, argv);
     f = compile(`{ return (a+b)*$c; }, int);
     printf("EXPECT: 12\n%d\n", (*f)());

     f = compile(`{ enum {a=2,b,c=5}; return (a+b)*c; }, int);
     printf("EXPECT: 25\n%d\n", (*f)());
}

/**/

/*
 * FREE VARIABLES (GLOBAL AND LOCAL)
 */

#include "test.h"

void expect() {
     int gi;
     float gf;
     double gd;
     int * gpi;
     int li;
     float lf;
     double ld;
     int * lpi;
     gi = 3;
     gf = (float)gi + 4.;
     gd = 2*gf;
     gpi = &li;
     lpi = &gi;
     lf = gf+6;
     ld = gd/7;
     *gpi = 1;
     *lpi = 2;
     printf("EXPECT: %d %d %f %e %d %d %f %e\n", 
	    gi, *gpi, gf, gd, li, *lpi, lf, ld);
}
int gi;
float gf;
double gd;
int * gpi;
void main(int argc, char **argv)
{
     int li;
     float lf;
     double ld;
     int * lpi;
     void cspec c;
     void (*f)();

     doargs(argc, argv);
     c = `{ gi = 3;
	    gf = (float)gi + 4.;
	    gd = 2*gf;
	    gpi = &li;
	    lpi = &gi;
	    lf = gf+6;
	    ld = gd/7;
	    *gpi = 1;
	    *lpi = 2;
     };
     f = compile(c, void);
     expect();
     (*f)();
     printf("%d %d %f %e %d %d %f %e\n", gi, *gpi, gf, gd, li, *lpi, lf, ld);
}

/**/

/*
 * TYPEDEFS; CASTS
 */

#include "test.h"

typedef void cspec vcs;
int sf() { 
     typedef unsigned short us;
     typedef unsigned int ui;
     us x; ui y; char *p;
     x = 2; y = 3;
     x = x + (long)y;
     y = (unsigned)((char)((long)'A'))+x;
     p = (char *)&y;
     x = (short)(long)*((unsigned *)p);
     return (int)x;
}
void main(int argc, char **argv) {
     vcs c;
     int (*f)();
     
     doargs(argc, argv);
     c = `{ typedef unsigned short us;
	    typedef unsigned int ui;
	    us x; ui y; char *p;
	    x = 2; y = 3;
	    x = x + (long)y;
	    y = (unsigned)((char)((long)'A'))+x;
	    p = (char *)&y;
	    x = (short)(long)*((unsigned *)p);
	    return (int)x;
     };
     f = compile(c, int);
     printf("EXPECT: %d\n", sf());
     printf("%d\n", (*f)());
}

/**/

/*
 * TYPE QUALIFIERS
 */

#include "test.h"

void r(void)
{
     int (*f)();
     int k0 = 4, k1 = 5, k2 = 6;
     void cspec c = `{ register int k0;
		       register int k3;
		       int k4;
		       k0 = 1;
		       k3 = k0+k1+k2+2; /* 14 */
		       k4 = 7;
		       return k3+k4; /* 21 */
		  };
     f = compile(c, int);
     printf("EXPECT: 21\n");
     printf("%d\n",(*f)());
}
static int x = 1;
void s1(void)
{
     int (*f)();
     void cspec c = `{ int j = 2; x = j;};
     f = compile(c, int);
     printf("EXPECT: 2\n");
     (*f)();
     printf("%d\n", x);
}
void s2(void)
{
     static int x = 1;
     int (*f)();
     void cspec c = `{ int j = 2; x = j;};
     f = compile(c, int);
     printf("EXPECT: 2\n");
     (*f)();
     printf("%d\n", x);
}
void s3(void)
{
     int a = 3;
     int (*f)();
     void cspec b = `{ static int z = 0; 
		       z = z+a;
		       return z;
     };
     f = compile(b, int);
     printf("EXPECT: 3\n");
     printf("%d\n", (*f)());

     b = `{ static int z = 0; 
		       z = z+a;
		       return z;
     };
     f = compile(b, int);
     printf("EXPECT: 3\n");
     printf("%d\n", (*f)());
}

main(int argc, char **argv)
{
     doargs(argc, argv);
     r();
     s1();
     s2();
     s3();
}

/**/

/*
 * POINTERS, ARRAYS
 */

#include "test.h"

typedef int (*ip)();
typedef double (*dp)(void);

void easy(void)
{
     int a = 10;
     int *b = &a;

     void cspec  cs = `{ int x = 11; int *y = &x; int **z = &b;
			 *y = 2; **z = 1; return x+a; 
     };
     ip f = compile(cs, int);
     dump(f);  
     printf("EXPECT: 3\n");
     printf("%d\n",(*f)());
}

int fa(int x) { return x; }
int fb(int x) { return -x; }
void fptrs(void)
{
     ip f, g[2] = { fa, fb }, h = fa;
     int x, y = 2;
     void cspec c = `{ int z = (g[0])(y);
		       ip h2 = h;
		       x = (*h2)(y);
		       x = x+z+fb(y); 
		       return x;
     };
     f = compile(c, int);
     printf("EXPECT: 2\n");
     printf("%d\n",(*f)());
}

double ra[5] = {0.6,1.,2.,3.,4.};
double *rb = ra;
void arrays(void)
{
     void cspec cs; 
     double (*dfp)();
     int (*ifp)();
     int x, *xp, xa[2] = {1,2};
     {
	  cs = `{ return (rb[2] = 3., rb[1]=rb[0]+1., rb[0] ); };
	  dfp = compile(cs, double);
	  printf("EXPECT: %e %e %e %e\n", 0.6, 0.6, 1.6, 3.0);
	  printf("%e ",(*dfp)()); printf("%e %e %e\n",rb[0],rb[1],rb[2]);
     }
     {
	  cs = `{ return (rb[2] = 3., rb[1]++, ++rb[0] ); };
	  dfp = compile(cs, double);  
	  printf("EXPECT: %e %e %e %e\n", 1.6, 1.6, 2.6, 3.0);
	  printf("%e ",(*dfp)());
	  printf("%e %e %e\n",rb[0],rb[1],rb[2]);
     }
     {
	  x = 5; xp = &x;
	  cs = `{ return (*xp)++; };
	  ifp = compile(cs, int);
	  printf("EXPECT: 5 6\n");
	  printf("%d ",(*ifp)()); printf("%d\n",x);
     }
     {
	  xp = &xa[0];
	  cs = `{ return (xp[1])++; };
	  ifp = compile(cs, int);
	  printf("EXPECT: 2 3\n");
	  printf("%d ",(*ifp)()); printf("%d\n",xp[1]);
     }
}

void locarrays(void)
{
     void cspec c;
     ip f;
     c = `{ int x[5];
	    x[0] = 1; x[1] = 2; x[2] = x[1]+x[0];
	    return x[2]+x[0];
     };
     f = compile(c, int);
     printf("EXPECT: 4\n");
     printf("%d\n", (*f)());

     c = `{ char x[5][5];
	    int i,j,cnt = 0;
	    for (i = 0; i < 5; i++)
	        for (j = 0; j < 5; j++)
	    	    x[i][j] = cnt++;
	    cnt = 0;
	    for (i = 0; i < 5; i++)
	        for (j = 0; j < 5; j++)
		    cnt += x[i][j];
	    return cnt;
     };
     f = compile(c, int);
     printf("EXPECT: 300\n");	/* Sum of ints from 1 to 24 */
     printf("%d\n", (*f)());

     {
	  typedef char c5x5[5][5];
	  c5x5 vspec x = local(c5x5);
	  c = `{ int i,j,cnt = 0;
		 for (i = 0; i < 5; i++)
		    for (j = 0; j < 5; j++)
		       x[i][j] = cnt++;
		 cnt = 0;
		 for (i = 0; i < 5; i++)
		    for (j = 0; j < 5; j++)
		       cnt += x[i][j];
		 return cnt;
	  };
	  f = compile(c, int);
	  printf("EXPECT: 300\n");	/* Sum of ints from 1 to 24 */
	  printf("%d\n", (*f)());
     }

}

main(int argc, char **argv)
{
     doargs(argc, argv);
     easy();
     fptrs();
     arrays();
     locarrays();
}

/**/

/*
 * AGGREGATES
 *
    Structs and unions:
      global, local
      assignments from one field to another
      assignments of one struct to another
 */

#include "test.h"

typedef struct {char a; int b;} X;
X x = {'A',1}, y = {'B',2};
void glob(void)
{
     int (*f)();
     X (*xf)();
     void cspec c;

     c = `{ x.b = (int)x.a; return x.b; };
     f = compile(c, int);
     printf("EXPECT: 1 65 65 \n");
     printf("%d ",x.b); printf("%d ",(*f)()); printf("%d\n",x.b);
     
     c = `{ X x; x.b = (int)y.a; return x.b; };
     f = compile(c, int);
     printf("EXPECT: 65 66 65 \n");
     printf("%d ",x.b); printf("%d ",(*f)()); printf("%d\n",x.b);

     c = `{ x = y; };
     f = compile(c, int);
     printf("EXPECT: 1 2 1 \n");
     printf("%d ",x.b); printf("%d ",(*f)()); printf("%d\n",x.b);

     c = `{ X y = x; X z = {'C',3 }; z = y; return (int)z.a; };
     f = compile(c, int);
     printf("EXPECT: 2 67 2 \n");
     printf("%d ",y.b); printf("%d ",(*f)()); printf("%d\n",y.b);

     c = `{ X x = y; y.b++; return x; };
     xf = compile(c, X);
     printf("EXPECT: 66 2 66 3 66 4\n");
     printf("%d %d ", y.a, y.b);
     printf("%d ", ((*xf)()).a); printf("%d ", ((*xf)()).b);
     printf("%d %d\n", y.a, y.b);
}

void loc(void)
{
     X x = {'a',2};
     int (*f)();
     X (*xf)();
     void cspec c;

     c = `{ x.b = (int)x.a; return x.b; };
     f = compile(c, int);
     printf("EXPECT: 2 97 97\n");
     printf("%d ",x.b); printf("%d ",(*f)()); printf("%d\n",x.b);
     
     c = `{ X x; x.b = (int)y.a; return x.b; };
     f = compile(c, int);
     printf("EXPECT: 97 66 97\n");
     printf("%d ",x.b); printf("%d ",(*f)()); printf("%d\n",x.b);

     c = `{ x = y; };
     f = compile(c, int);
     printf("EXPECT: 1 2 1 \n");
     printf("%d ",x.b); printf("%d ",(*f)()); printf("%d\n",x.b);

     c = `{ X y = x; X z = {'C',3 }; z = y; return (int)z.a; };
     f = compile(c, int);
     printf("EXPECT: 2 67 2 \n");
     printf("%d ",y.b); printf("%d ",(*f)()); printf("%d\n",y.b);

     c = `{ X x = y; y.b++; return x; };
     xf = compile(c, X);
     printf("EXPECT: 66 2 66 3 66 4\n");
     printf("%d %d ", y.a, y.b);
     printf("%d ", ((*xf)()).a); printf("%d ", ((*xf)()).b);
     printf("%d %d\n", y.a, y.b);
}

main(int argc, char **argv)
{
     doargs(argc, argv);
     loc();
     glob();
}

/**/

/*
 * COMPOSITION WITH CSPECS
 */

#include "test.h"

void expr(void)
{
     int x = 4, y = 6, z = 8;
     int cspec c, cspec d, cspec e;
     void cspec f, cspec g;
     int (*fp)();
     c = `4;
     d = `(x+(y*(z/@c))); /* 4+(6*(8/4)) = 4+6*2 = 16 */
     e = `(((x/@c)*y)+z); /* ((4/4)*6) + 8 = 14 */

     f = `{ x = (@e+@d)/(x+(z/@c))+3; }; /* 30/(4+2)+3 = 8 */
     /* So after f is executed, x is 8, not 4;
	so d ==> 20, e ==> ((8/4)*6)+8 = 20 */
     g = `{ @f; y = @d-@e; z = @e/2; return @d+x; };
     /* y = 0, so z = (0+z)/2 = 4, so retval = (8+(0))+8 = 16*/
     fp = compile(g, int);
     printf("EXPECT: 16 8 0 4\n");
     printf("%d ",(*fp)());
     printf("%d %d %d\n",x,y,z);
}
void loopexpr(void)
{
     int x[10] = {1,2,3,4,5,6,7,8,9,10};
     int cspec z = `0;
     int * cspec y;
     int (*f)();
     int i;

     for(i=0;i<3;i++)
	  z = `(@z+$x[0]*$x[1]);
     /* z is now 6 */

     f = compile(`{ return @z; }, int);
     dump(f);
     printf("EXPECT: 6\n");
     printf("%d\n",(*f)());

     y = `(int*)x;
     z = `0;
     for(i=0;i<10;i++)
	  z = `((@y)[$i]*(@y)[$i]+@z);
     /* z is now 385 */
     f = compile(`{ return @z; }, int);
     dump(f);
     printf("EXPECT: 385\n");
     printf("%d\n",(*f)());
}
int cspec icf(int x) { return `$x; }
void func(void)
{
     int cspec y = `@icf(4);
     void cspec z = `{ return @y+3; };
     int (*f)() = compile(z,int);
     printf("EXPECT: 7\n");
     printf("%d\n",(*f)());
}
void qual(void)
{
     const int cspec cicx = `1;
     int const cspec iccx = `2;
     int const cspec ciccx = `4;

     int (*f)() = compile(`{return @cicx+@iccx+@ciccx;}, int);
     printf("EXPECT: 7\n");
     printf("%d\n",(*f)());
}

main(int argc, char **argv)
{
     doargs(argc, argv);
     expr();
     loopexpr();
     func();
     qual();
}

/**/


/*
 * COMPOSITION WITH VSPECS
 */

#include "test.h"

void misc(void)
{
     {
	  int vspec i, vspec n, vspec k;
	  void cspec c1, cspec c2, cspec c4;
	  int cspec c3;
	  void (*f)();

	  i = local(int); n = local(int); k = local(int);

	  c1 = `{ int a = 1, b = 2;
		  @i = 0;
		  if (@i) 
		       return b;
		  else 
		       printf("%d",a);
		  @k = 1; @n = b;
	     };
	  c2 = `{ for (; @i < @n; (@i)++) @k *= 2; };
	  c3 = `(@k==@n+2);
	  c4 = `{ @c1; @c2; 
		  if (@c3)
		       printf("%d\n",@k);
		  else
		       printf("Something is wrong.\n");
	     };

	  f = compile(c4, void);
	  printf("EXPECT: 14\n");
	  (*f)();
     }
     
     {
	  /* Only check compile output */

	  int vspec a, vspec b, vspec c; int cspec j;
	  j = `(@a==@b);  j = `(@a<@b);     j = `(@a&&@b);
	  j = `((@a)++);  j = `(@a+=@b);    j = `(@a-@b);
	  j = `(@a<<=@b); j = `(@a?@b:@c);  j = `(@a^=@b^=@c);
     } 
     {
	  int x = 0;
	  int (*f)();
	  int cspec a = `x++, cspec b = `x++, 
	       cspec c = `x++, cspec d = `x++;
	  a = `(a,b,d,b,c);
	  f = compile(`{ return a; }, int);
	  printf("EXPECT: 5 6\n");
	  printf("EXPECT: %d %d\n", (*f)(), x);
     }

     {
	  int * a = (int *)malloc(10*sizeof(int));
	  int * vspec va = local(int *);
	  int cspec cx[3], vspec vx[3];
	  int cspec y;
	  void cspec c;
	  void (*f)();

	  a[2] = 7;  a[3] = 1;

	  cx[0] = `0;	  cx[1] = `1;	  cx[2] = `2;
	  vx[0] = local(int); vx[1] = local(int); vx[2] = local(int);

	  y = `(@cx[0]+@cx[1]-@cx[2]);

	  c = `{ @vx[0] = @y; @vx[1] = 4; @vx[2] = @vx[1]+@vx[0]; };
	  c = `{ @c; @((va)) = a; (@va)[2] = (@y, @vx[2]); }; /* va[2] = 3 */
	  c = `{ @c; (@va)[3] *= 5; }; /* va[3] = 5 */
	  f = compile(c, void);
	  (*f)();
	  
	  printf("EXPECT: 3 5\n");
	  printf("%d %d\n", a[2], a[3]);
     }

     {
	  int vspec i, vspec j;
	  int cspec ci, cspec cj;
	  double vspec k;
	  void cspec c;
	  double (*f)();

	  i = local(int);  j = local(int);
	  k = local(double);

	  ci = `4;
	  cj = `(@j=3);

	  c = `{ @cj; @i = @ci;
		 (@i)++; @j = @j-1;
		 @k = (double)@i+2.; 
		 return (double)@j+@ci+@k;};
	  f = compile(c, double);
	  printf("EXPECT: 13.\n");
	  printf("%e\n",(*f)());
     }

     {
	  typedef struct {char a; int b;} X;
	  X vspec a, * vspec b;
	  int cspec j1; char cspec j2;
	  void cspec c;
	  int (*f)();

	  a = local(X); b = local(X*);

	  j1 = `a.b; j2 = `b->a;
	  c = `{ a.b = -1; b = &a; b->a = 'B'; return (char)((int)j2-j1); };
	  f = compile(c, int);
	  printf("EXPECT: C\n");
	  printf("%d\n",(*f)());
     }
}
void aggregate(void)
{
     typedef struct {int x;} X;
     int (*f)();
     void cspec c;

     {
	  int vspec v = local(int);
	  c = `{ @v = 2; return @v; };
	  f = compile(c, int);
	  printf("EXPECT: 2\n");
	  printf("%d\n",(*f)());
     }
     {
	  X vspec v = local(X);
	  c = `{ (@v).x = 3; return (@v).x; };
	  f = compile(c,int);
	  printf("EXPECT: 3\n");
	  printf("%d\n",(*f)());
     }
}

main(int argc, char **argv)
{
     doargs(argc, argv);
     misc();
     aggregate();
     locals();
     params();
}

/**/

/*
 * UNQUOTING, SPEC CONVERSION
 */

#include "test.h"

int cspec f1(int x) {
     return `x;
}
int cspec f2(int vspec v) {
     return `v;
}
int f3(int vspec v) {
     return 1;
}
int vspec f4(int vspec v) {
     return v;
}
int vspec f5(int i) {
     return local(int);
}

void small() 
{
     void cspec c; int vspec v; int i = 1;
     int (*f)();
	  
     c = `{ int l; v = 2; l = i+v; return l; };
     f = compile(c, int);
     printf("EXPECT: 3\n%d\n", (*f)());

     c = `{ int l = f1(2)+i; return l; };
     f = compile(c, int);
     printf("EXPECT: 3\n%d\n", (*f)());

     c = `{ int l = f1(i); return l; };
     f = compile(c, int);
     printf("EXPECT: 1\n%d\n", (*f)());

     c = `{ int l; v = 5; l = f2(v); return l; };
     f = compile(c, int);
     printf("EXPECT: 5\n%d\n", (*f)());

     c = `{ int l = 1; l = f2(l+2); return l; };
     f = compile(c, int);
     printf("EXPECT: 3\n%d\n", (*f)());

     c = `{ int l = 3; v = 4; l = f2(l+v); return l; };
     f = compile(c, int);
     printf("EXPECT: 7\n%d\n", (*f)());
}

void big(void)
{
     void cspec c; int vspec v; int i = 1;
     int (*f)();

     c = `{ int l = 2; l = f2(l) + f1(i); return l; };
     f = compile(c, int);
     printf("EXPECT: 3\n%d\n", (*f)());

     c = `{ int l = 2; v = 1; l = f4(l+f4(f4(v))); return l; };
     f = compile(c, int);
     printf("EXPECT: 3\n%d\n", (*f)());

     c = `{ int l = f5(f3(v)); return 1; };
     f = compile(c, int);
     printf("EXPECT: 1\n%d\n", (*f)());
}

void ptrs(void)
{ 
     int aa[3];
     int * vspec a;
     int vspec *b;
     void cspec c;
     int (*f)();

     a = local(int *);

     b = malloc(2*sizeof(int vspec));
     b[0] = local(int);
     b[1] = local(int);

     c = `{ int c;
            a = malloc(2*sizeof(int));
	    a[0] = 1; a[1] = 2;
	    b[0] = a[1];
	    b[1] = a[0];
            c = f2(f4(a[1] + b[1]));
	    return c;
       };
     f = compile(c, int);
     printf("EXPECT: 3\n%d\n", (*f)());
}

typedef struct {char a; int vspec v;} X;
void structs(void)
{
     void cspec c;
     X * a;
     X b;

     a = &b;
     b.a = 'A';
     b.v = local(int);

     c = `{ int j, k;
	    b.v = 1;
	    j = a->v; k = b.v;
	    return f4((*a).v) + j + k;
     };
     f = compile(c, int);
     printf("EXPECT: 3\n%d\n", (*f)());
}

main(int argc, char **argv)
{
     doargs(argc, argv);
     small();
     big();
     ptrs();
     structs();
}

/**/


/*
 * RUN-TIME CONSTANTS
 */

#include "test.h"

void easy(void)
{
     int x = 4;
     void cspec z = `{ return x = 4+$x+$x+x;};
     x = 5;
     printf("EXPECT: 17\n");
     printf("%d\n",(*compile(z,int))());
}

int vspec uqf(int vspec v)
{
     return v;
}
void unquoting(void)
{
     void cspec c;
     int vspec v;
     int i = 0;
     int (*f)();

     v = local(int);
     c = `{ int a = 1;
	    v = 2;
            return uqf(a+v+$i); /* should work; unquoted $ is a no-op */
     };
     i = 1;
     f = compile(c, int);
     printf("EXPECT: 3\n");
     printf("%d\n", (*f)());
}


int g(int x) { return x; }
void fptrs(void)
{
     void (*v)();
     int (*f)() = g;
     int x = 2;
     int cspec a, cspec b, cspec c, cspec d;
     a = `($f)($x);
     b = `$f(x);
     c = `$g(x);
     d = `($g)(x);
     x = 3;
     v = compile(`{		/* LOOK AT THIS WHEN a AND d ARE PRINTF ARGS */
	  int x = a; int y = d; 
	  printf("%d %d %d %d\n",x,@b,@c,y);
     }, void);
     printf("EXPECT: 2 2 2 3\n");
     (*v)();
}

void ptrs(void)
{
     int *a = (int*)malloc(10), *b[10];
     int cspec z, *cspec y;
     int (*f)();
     a[3] = 1;  z = `$a[3];  a[3] = 100;  a[4] = 1000;
     b[2] = &a[3];  y = `$b[2];  b[2] = &a[4];
     f = compile(`{ return z+*y; }, int);
     printf("EXPECT: 101\n");
     printf("%d\n",(*f)());
}
#if 0
typedef struct {int a; char b; } X;
void aggregates(void)
{
     X x = {1, 65}, y = {2, 66}, *z;
     X cspec x0;
     X (*f)();
     void cspec c;

     /* struct */
     x0 = `$x;  x = y;
     c = `{ X j; y = x; j = @x0; return j; };
     f = compile(c, X);
     printf("EXPECT: 1 A\n");
     printf("%d %c\n",((*f)()).a,((*f)()).b);

     /* ptr to struct */
     x.a = y.a+1;
     z = &y;
     c = `{ X a; a = $*z; return a; };
     z = &x;
     f = compile(c, X);
     printf("EXPECT: 2 B\n");
     printf("%d %c\n",((*f)()).a,((*f)()).b);
}
#endif
main(int argc, char **argv)
{
     doargs(argc, argv);
     easy();
     unquoting();
     fptrs();
     ptrs();
#if 0
     aggregates();
#endif
}

/**/

/*
 * OPTIMIZATIONS BASED ON RUN-TIME CONSTANTS
 */

#include "test.h"

void cond(void)
{
     int x;  int (*f)();
     void cspec c;
     printf("EXPECT: 4 5\n");
     x = 1;
     c = `{ int z = $x ? 4:5; return z; };
     f = compile(c,int);
     dump(f); 
     printf("%d ",(*f)());
     x = 0;
     c = `{ int z = $x ? 4:5; return z; };
     f = compile(c,int);
     dump(f); 
     printf("%d\n",(*f)());
}

void if1(void)
{
     int x=0, y=1;
     int (*f)();
     void cspec cs = `{ 
	  if ($x<$y)
	       return 10;
	  else
	       return 11;
     };
     f = compile(cs,int);
     dump(f); 
     printf("EXPECT: 10\n");
     printf("%d\n",(*f)());
}

void if2(void)
{
     double x=1., y=0.;
     int (*f)();
     void cspec cs = `{ 
	  if ($x<$y)
	       return 10;
	  else
	       return 11;
     };
     f = compile(cs,int);
     dump(f);
     printf("EXPECT: 11\n");
     printf("%d\n",(*f)());
}

void while1(void)
{
     int x=0, y=1;
     void (*f)();
     void cspec cs = `{ 
	  do {
	       printf("woah!\n");
	  } while ($x<$y);
     };
     printf("EXPECT: Warning: infinite loop\n");
     f = compile(cs,void);
     dump(f);
}

void while2(void)
{
     int x=1, y=0;
     void (*f)();
     void cspec cs = `{ 
	  do {
	       printf("woah!\n");
	  } while ($x<$y);
	  printf("woah!\n");
     };
     f = compile(cs,void);
     dump(f); 
     printf("EXPECT: woah! woah!\n");
     (*f)();
}

void while3(void)
{
     int x=0, y=1;
     void (*f)();
     void cspec cs = `{ 
	  while ($x<$y) {
	       printf("woah!\n");
	  }
     };
     printf("EXPECT: Warning: infinite loop\n");
     f = compile(cs,void);
     dump(f);
}

void while4(void)
{
     int x=1, y=0;
     void (*f)();
     void cspec cs = `{ 
	  while ($x<$y) {
	       printf("woah!\n");
	  }
	  printf("woah!\n");
     };
     f = compile(cs,void);
     dump(f); 
     printf("EXPECT: woah!\n");
     (*f)();
}

void for1(void)
{
     int x=0, y=1;
     void (*f)();
     void cspec cs = `{ 
	  int i;
	  for (i=0; $x<$y; i++) {
	       printf("woah!\n");
	  }
     };
     printf("EXPECT: Warning: infinite loop\n");
     f = compile(cs,void);
     dump(f);
}

void for2(void)
{
     int x=1, y=0;
     void (*f)();
     void cspec cs = `{ 
	  int i;
	  for (i=0; $x<$y; i++) {
	       printf("woah!\n");
	  }
	  printf("hello\n");
     };
     f = compile(cs,void);
     dump(f);
     printf("EXPECT: hello\n");
     (*f)();
}

void main(int argc, char **argv)
{
     doargs(argc, argv);
     cond();
     if1(); 
     if2();
     while1(); 
     while2(); 
     while3(); 
     while4();
     for1();
     for2();
}


/**/

/*
 * DYNAMIC FUNCTION CALL CONSTRUCTION
 */

#include "test.h"

typedef double (*df)();
typedef int (*xf)();
double xx(int i, double d) { return (double)i*d;}
int yy(int i, double d, int z) { return z+i*(int)d;}

void x1(void)
{
     double (*f)(df);
     df vspec dp;
     void cspec args;
     int i = 2;
     double d = 2.5;

     dp = param(df, 0);
     args = push_init;
     push(args, `i);
     push(args, `d);

     f = compile(`{ return (@dp)(@args); },double);
     dump(f);
     printf("EXPECT: 5.0\n");
     printf("%e\n",f(xx));
}

void x2(void) {
     int (*f)(xf);
     xf vspec dp;
     void cspec args;
     int i = 2;
     double d = 2.5;

     dp = param(xf, 0);
     args = push_init;
     arg(1, args, `d);
     push(args, `i);
     arg(0, args, `i);

     f = compile(`{ return (@dp)(@args); },int);
     dump(f);
     printf("EXPECT: 6\n");
     printf("%d\n",f(yy));
}

void main(int argc, char **argv) {
     doargs(argc, argv);
     x1();
     x2();
}

/**/

/*
 * DYNAMIC JUMPS/LABELS
 */

#include "test.h"

typedef int (*ip)();
typedef double (*dp)(double);
typedef void (*vp)();

void jump1(void)
{
     void cspec c1, cspec c2, cspec c3;
     int vspec v = local(int);
     ip f;
     c2 = `{ v = v+1; };
     c3 = jump c2;
     c1 = `{ if (v < 10) jump c2; };
     c1 = `{ v = 0; @c3; v = 100; @c2; @c1; return v; };
     f = compile(c1, int);
     printf("EXPECT: 10\n");
     printf("%d\n", (*f)());

}

void jump2(void)
{
     void cspec ca, cspec cb, cspec cc, cspec cd, cspec ce, cspec cf;
     void cspec gb, cspec gd;
     vp f;
     ca = `{ printf("a"); };
     cb = `{ printf("b"); };
     cc = `{ printf("c"); };
     cd = `{ printf("d"); };
     cf = `{ printf("f\n"); };
     ce = `{ printf("e"); jump cf; };
     cb = `{ @cb; jump(cc); };
     cd = `{ @cd; jump ce; };
     gb = jump(cb);
     gd = jump(cd);
     ca = `{ @ca; @gb; };
     cc = `{ @cc; @gd; };
     cf = `{ @ca; @cc; @cb; @ce; @cd; @cf; };
     f = compile(cf, void);
     printf("EXPECT: abcdef\n");
     (*f)();

}

void label1(void)
{
     void cspec ca, cspec cb, cspec cc, cspec cd, cspec ce, cspec cf;
     void cspec gb, cspec gd;
     void cspec L1, cspec L2, cspec L3, cspec L4, cspec L5, cspec L6;
     vp f;
     L1 = label;
     L2 = label();
     L3 = label();
     L4 = (label);
     L5 = (label());
     L6 = label;
     ca = `{ @L1; printf("a"); };
     cb = `{ @L2; printf("b"); };
     cc = `{ @L3; printf("c"); };
     cd = `{ @L4; printf("d"); };
     cf = `{ @L6; printf("f\n"); };
     ce = `{ @L5; printf("e"); jump L6; };
     cb = `{ @cb; label(); jump(L3); };
     cd = `{ @cd; label; jump L5; };
     gb = jump(L2);
     gd = jump(L4);
     ca = `{ @ca; @gb; label(); };
     cc = `{ @cc; @gd; };
     cf = `{ @ca; @cc; @cb; @ce; @cd; @cf; };
     f = compile(cf, void);
     printf("EXPECT: abcdef\n");
     (*f)();

}

void iself(void)
{
     void cspec c1;
     int vspec v = param(int, 0);
     ip f;
     c1 = `{ if (v == 0) return v; else return v+selfi(v-1); };
     f = compile(c1, int);
     printf("EXPECT: 15\n");
     printf("%d\n", (*f)(5));

}

void dself(void)
{
     void cspec c1;
     double vspec v = param(double, 0);
     dp f;
     c1 = `{ if (v == 0.0) return v; else return v+selfd(v-1.); };
     f = compile(c1, double);
     printf("EXPECT: 15.0\n");
     printf("%e\n", (*f)(5.));

}

main(int argc, char **argv)
{
     doargs(argc, argv);
     jump1();
     jump2();
     label1();
     iself();
     dself();
}

/**/

/* 
 * SEMANTICS CHECKS (NON-LOCAL GOTOS, DECOMPILE)
 */

extern int printf();
void a() {
     void cspec c1 = `{ hi: printf("hi"); goto there; };
     void cspec c2 = `{ there: printf(" there\n"); };
     (*compile(`{ goto hi; @c2; @c1; }, void))();
}

void b() {
     int z;
     int (*p)();
     decompile(z);
     decompile(p);
     decompile(printf);
     decompile(compile(`{ return 2; }, int));
}

/**/

/*
 * DECOMPILE
 */

#include "test.h"

extern int printf();
void main(int argc, char **argv) {
     int (*f1)(), (*f2)();
     doargs(argc, argv);
     f1 = compile(`{ return 19; }, int);
     f2 = printf;
     (*f2)("%d\n", (*f1)());
     printf("f1\n");
     decompile(f1);
     printf("f1 again\n");
     decompile(f1);
     printf("f2\n");
     decompile(f2);
}

/**/

/*
 * DYNAMIC FUNCTIONS WITH MANY ARGUMENTS
 */

#include "test.h"

typedef void (*f1t)(int, char, long, float, double, 
		    int, float, double, char, long);
typedef void (*f2t)(int, char, long, int, int,
		    int, int, int, char);

void f1 () {
     int vspec iI = param(int, 0);
     char vspec iC = param(char, 1);
     long vspec iL = param(long, 2);
     float vspec iF = param(float, 3);
     double vspec iD = param(double, 4);
     int vspec jI = param(int, 5);
     float vspec jF = param(float, 6);
     double vspec jD = param(double, 7);
     char vspec jC = param(char, 8);
     long vspec jL = param(long, 9);
     void cspec cs;
     f1t f;
     cs = `{
	  printf("%d ", iI);
	  printf("%c ", iC);
	  printf("%d ", iL);
	  printf("%f ", iF);
	  printf("%e ", iD);
	  printf("%d ", jI);
	  printf("%f ", jF);
	  printf("%e ", jD);
	  printf("%c ", jC);
	  printf("%d\n", jL);
     };
     f = (f1t)compile(cs, void);
     dump(f);
     printf("EXPECT: %d %c %d %f %e %d %f %e %c %d\n", 1, 'a', 3, 4.0, 5.0,
	    6, 7.0, 8.0, 'b', 10);
     (*f)(1, 'a', 3, 4.0, 5.0, 6, 7.0, 8.0, 'b', 10);
}

void f2 () {
     int vspec iI = param(int, 0);
     char vspec iC = param(char, 1);
     long vspec iL = param(long, 2);
     int vspec iI0 = param(int, 3);
     int vspec iI1 = param(int, 4);
     int vspec jI = param(int, 5);
     int vspec jI0 = param(int, 6);
     int vspec jI1 = param(int, 7);
     char vspec jC = param(char, 8);
     void cspec cs;
     f2t f;
     cs = `{
	  printf("%d ", iI);
	  printf("%c ", iC);
	  printf("%d ", iL);
	  printf("%d ", iI0);
	  printf("%d ", iI1);
	  printf("%d ", jI);
	  printf("%d ", jI0);
	  printf("%d ", jI1);
	  printf("%c\n", jC);
     };
     f = (f2t)compile(cs, void);
     dump(f);
     printf("EXPECT: %d %c %d %d %d %d %d %d %c\n", 1, 'a', 3, 4, 5,
	    6, 7, 8, 'b');
     (*f)(1, 'a', 3, 4, 5, 6, 7, 8, 'b');
}

void main(int argc, char **argv) {
     doargs(argc, argv);
     f1();
     f2();
}

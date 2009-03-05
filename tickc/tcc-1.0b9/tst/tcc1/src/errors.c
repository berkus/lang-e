/* t0 */

void main()
{
  `3+4;
}

/**/

/* d0 */

void main()
{
     int x;

     /* error: address of rtc */
     int cspec c = `&$x;

     /* error: not an lvalue */
     void cspec y = `{$x = 4;};

     /* error: arg of $ has spec type */
     int vspec v;
     void cspec z = `{ int a  = $v; };

     /* error: @ operand of $ */
     {
	  int vspec v;
	  int m[6];
	  void cspec z = `{x = $@v;};
	  int cspec y = `$m[@v];
     }
}

/**/

/* a0 */

void main()
{
     {
	  int cspec x = `4;
	  int cspec y = `@(x--);
     }
     {
	  int cspec x = `4;
	  int cspec y = `((@x)--);
     }
     {
	  int cspec x = `4;
	  int cspec y = `@-x;
     }
     {
	  int cspec w = `3;
	  int cspec x = `4;
	  int cspec y = `@(x+w);
     }
     {
	  int cspec x = `4;
	  int cspec y = `@@x;
     }
     {
	  void cspec k = `{return k; };
     }
}


/* rtc */

int cspec xx(int x) { return `x; }

void main()
{
  int cspec (*f)(int) = xx;
  int x = 2;
  /* should be an error */
  int cspec v = `$f(x);
}

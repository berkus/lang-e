#include <stdio.h>
#include <stdarg.h>

int argtypes[] = {1,2,0};
void printargs (int *atp, ...);

void printargs( int *atp, ...)
{
  va_list ap;
  int argtype;

  va_start(ap,atp);
  while ((argtype = *atp++) != 0)
    if (argtype == 1) 
      printf("int %d\n",va_arg(ap,int));
    else
      printf("double %e\n",va_arg(ap,double));
  va_end(ap);
}

int main()
{
  printargs(&argtypes[0], 1, 2.0);
  return 0;
}

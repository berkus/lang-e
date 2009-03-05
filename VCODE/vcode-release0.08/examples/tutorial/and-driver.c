#include "vcode.h"
#include "and.c"
int main(void)  {
	v_uptr up = mk_and();

	printf("1 and 1 = %d, 1 and 0 = %d\n", up(1,1), up(1,0));
	return 0;
}

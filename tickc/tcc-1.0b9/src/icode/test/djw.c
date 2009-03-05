#include <stdio.h>
#include <icode.h>

long foo() { printf("here we are\n"); return 3; }

int main(void) { 
        v_iptr ip;
        i_local_t rd, p0, p;
        int offset;
        i_label_t l;

	i_debug_on();
        i_init(16);
                rd = i_local(0, I_P);
                i_callli(rd, foo);
/*		i_retl(rd);*/
                i_retv();
        i_end();

        ip = i_emit(&offset,0).i;
	v_dump((void*)ip);
        printf("calling ip() %d\n", ip());
        return 0;
}
/*
spec:
callli
retl
retv
*/

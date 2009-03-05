#include "../icode.h"

void a () {
     i_local_t a, b, c,d;
     
     i_init(20);

     a = i_local(0, I_I);
     b = i_local(0, I_I);
     c = i_local(0, I_I);
     d = i_local(0, I_I);

     i_seti(a, 5);
     i_seti(b, 7);
     i_seti(c, 8);
     i_movi(d, c);
     i_addi(c, a, b); 
     i_addi(d, d, c); 
     i_reti(d);
     i_end();

     i_unparse();
     i_emitc();
}

void main() {
     printf("##########\n");
     a();
     i_reset(); printf("##########\n");
}

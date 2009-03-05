#include "stdio.h"
#include "vcode.h"

#define PROCS 100000
#define INSNS 100

/* instructions generated is PROCS * INSNS */

int main () {
  char insn[INSNS * 10];
  v_reg_t r1, r2;
  union v_fp f;
  int v1 = 0;
  int i,j;

  for (j = 1; j < PROCS; j++) {
    v_lambda ("foo", "", NULL, V_LEAF, insn, sizeof (insn));
    if (!v_getreg (&r1, V_I, V_TEMP) || !v_getreg (&r2, V_I, V_TEMP)) {
      exit (0);
    }
    v_seti (r1, 0xff);
    for (i = 1; i < INSNS; i++) {
      v_stusi  (r1, v_zero, &v1);
     }
    f = v_end (NULL);
  }
  /*  f.i ();
  printf ("v1 = %d\n", v1); */
  return 0;
}

/* Power: raise base to n-th power: n > 0; specialized for a runtime constant n. */
v_fptr specialize_power(int n) {
	v_reg_type x, sum;

        v_lambda("power", "%d", &x, V_LEAF, malloc(512), 512);
	{
		int i;

		/* Allocate accumulator */
		v_getreg(&sum, V_D, V_TEMP);
		v_movf(sum, x);		/* initialize sum */

		/* Specialize power to x^n by unrolling the loop to multiply
	 	   x n times: (x * x * ... * x). */
		for(i = 0; i < n - 1; i++)
			v_mulf(sum, sum, x);

		v_retf(sum);	/* return x ^ n */
	}
        return v_end(0).f;	/* return pointer to result. */
}

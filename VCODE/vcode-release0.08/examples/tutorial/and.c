/* generate code to compute bitwise and: unsigned and(unsigned x, unsigned y) { return x & y; } */
v_uptr mk_and(void) {
	v_reg_type arg[2];	/* Two arguments. */

	/* Create and function that returns the result of anding its two unsigned inputs */
        v_lambda("and", "%u%u", arg, V_LEAF, (void *)malloc(512), 512);
		v_andu(arg[0], arg[0], arg[1]); /* And the two arguments */
		v_retu(arg[0]);			/* Return the result. */
        return v_end(0).u;
}

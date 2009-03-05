/* Create a leaf function that will return the next integer from 1 .. 2^32 */
v_uptr mk_stream(void) {
	/* Allocate space for code */
	v_code *code = (void *)malloc(512);

        v_lambda("int-stream", "", 0, V_LEAF, code, 512);
	{
		/* state is initially set to ``0'' */
		void *state = calloc(1, sizeof(unsigned));
		v_reg_type state_r, temp;

		/* allocate register to hold pointer to per-function storage. */
		v_getreg(&state_r, V_P, V_TEMP);	
		/* allocate scratch register */
		v_getreg(&temp, V_P, V_TEMP);		

		/* load pointer to state into register */
		v_setp(state_r, state);		/* SET Pointer */
		/* Load current state value */
		v_ldui(temp, state_r, 0);	/* LoaD Unsigned Immediate */
		/* Add 1 to it. */
		v_addui(temp, temp, 1);		/* ADD Unsigned Immediate */
		/* Store the new value into state */
		v_stui(temp, state_r, 0);	/* STore Unsigned Immediate */
		/* Return new value */
		v_retu(temp);  			/* RETurn Unsigned */
	}
        /* Compile and return an unsigned function pointer. */
        return v_end(0).u;      
}

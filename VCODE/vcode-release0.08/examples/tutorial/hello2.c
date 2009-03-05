	/* Create a function to print standard greeting. */
        v_lambda("hello-world", "", 0, V_NLEAF, malloc(512), 512);
		/* Generate simple call to printf; %P indicates it takes a pointer constant as an argument. */
                v_scallv((v_vptr)printf, "%P", "hello, world\n");
        v_end(0).v(); 	/* Compile & call */

# Note: If the given instruction template is not available, will have
# to be added by hand.

# Binary, infix ALU operations
# TODO:
#	add boolean experessions.
# Format
# 	name		c-equiv operation	types
&alu( 	"add", 		"+", 			"i u ul l f d");
&alu( 	"sub", 		"-", 	 		"i u ul l f d");
&alu(	"mul", 		"*",	 		"i u ul l f d");
&alu(	"div", 		"/",	 		"i u ul l f d");
&alu(	"mod", 		"%",	 		"i u ul l");
&alu(	"xor", 		"-",	 		"i u ul l");
&alu(	"and", 		"&",	 		"i u ul l");
&alu(	"or", 		"|",	 		"i u ul l");

# &alu6("lsh", "<<", @tlist1);
# &alu6("rsh", ">>", @tlist1);

# Simple unary operations
# 	name		c-equiv operation	types
&unary(	"com",		"~",			"i u ul l");
&unary(	"not",		"!",			"i u ul l");
&unary(	"neg",		"-",			"i u ul l f d");
&unary(	"mov",		" ",			"i u ul l p f d");

# Memory operations.
# TODO: 
#	add unaligned
# Format:
# 	name	types 				offset required		ld/st
&mem(	"st",	"c uc s us i u ul p f d", 	"aligned_offset", 	"store");
&mem(	"ld",	"c uc s us i u ul p f d", 	"aligned_offset", 	"load");

# Branch operations
# TODO
#	add conditonal move
# Format:
# 	name		c-equiv operation	types
&branch("beq",		"==",			"i u ul l p f d");
&branch("bne",		"!=",			"i u ul l p f d");
&branch("blt",		"<",			"i u ul l p f d");
&branch("ble",		"<=",			"i u ul l p f d");
&branch("bgt",		">",			"i u ul l p f d");
&branch("bge",		">=",			"i u ul l p f d");

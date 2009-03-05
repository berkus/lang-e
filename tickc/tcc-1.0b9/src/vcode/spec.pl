
#!/usr/uns/bin/perl
#
# Preprocessor used to generate a translator from vcode to machine language.
#
# sophisticated beq (takes anull & predict bits):  sbeq
#
# Non-constant loading unary and floating point do not take constants.  
# everything else does.
#
# If the constant field is not present, then we assume it is a pseudo-immediate operation
# operands are then loaded using the loadt vector (floating point are stuffed in a 
# temporary array and then laid out in the code vector -- need to be careful about the
# at vector). 
#
# There are a number of flags we look for:
# 	synthetic_cmv
#	no_reg_plus_reg

# we encode the knowlege of each type of instruction (i.e., how many operands it
# takes, and of what type

#
# The grammer is as follows:
#	'(' base insn_list? '(' type_list (name|insn_list) (name|insn_list)?)+ ')'
#		| '(' def name instruction? ')'
#		| ';' .*		/* single-line comments */
#		;
#	name: [a-zA-Z_]+ 
#		;
#	base: name 		/* base of vcode instruction */
#		;
#	type_list: type+
#		;
#	type: c | uc | s | us | i | u | ul | l | p | f | d 
#		;
#	insn_list: name
#		| `(' insn+ ')'
#		;
#	insn:	`(' name [ ',' [^,)]+ ]* ')'
#		;
#	
#	Semantics:
#	----------
#		if a name ends with :, then we append the type to it;
#		otherwise, we use registers.
#	
#		+ having the type appended
#		+ substitution of 
#	
#	If an instruction is simply a name, we attempt to replace it.

$_ = $ARGV[0];
$fastp = 0;
if (/fast/) {
    $fastp = 1;
}
&initialize;

# patterns 
$base	   = "^\b(\w+)\b";
$def	   = "^\bdef\b";

$_ = join("",<STDIN>); 	# slurp everything
s/([^%]*)%%// && print $1;	# print out everything before first %%
s/((;.*)|(\n))*//g;             # strip comments (and newlines)

# begin processing
while(/\s\S/) {
	&expect("(");
	# definition
	(&def || &constructor)  || die "unrecognized token <$_>\n";
	&expect(")");
}

#insn_list: `(' insn+ ')'
#	;
sub insn_list {
	local(@list, @i);
	if(s/^\s*\(//) {
		push(@list, $i) while($i = &insn);
		&expect(")");
	}
	@list;
}

sub insn {
	if(!/^\s*\(/) {
		0;
	} else {
		local($name);
		&expect("(");
		($name = &name) || die "must have a name\n";
		s/([^)]*\))//;			# slurp up entire line
		$name . "(" . $1;	
	}
}

# install a macro definition
sub def {
	local($name);
	if(s/^\s*def\b//) {
		($name = &name) || die "need a name\n";
		$macro{$name} = (/^\s*\)/ || join('#', &insn_list));
		1;
	}
}

sub is_mem {
	($base eq "ld" || $base eq "st" || $base eq "uld" || $base eq "ust");
}

sub is_cmv {
	($base eq "cmveq" || $base eq "cmvne" || $base eq "cmvlt" || 
	 $base eq "cmvle" || $base eq "cmvgt" || $base eq "cmvge");
}

sub type_list {
	local(@tlist);
	push(@tlist, $1) while(s/^\s*($types)\b//);
	die "must have a type list" if($#tlist < 0);
	@tlist;
}

sub name {
	s/^\s*((\w+:)|\w+|@)// && $1;
}

# is this token what we expected?
sub expect {
        local($x) = @_;
	(s/^\s*(\W)// && ($1 eq $x)) || 
		die "expecting $x, left <$_> ", join(' ',caller), "\n";
}

sub is_typelist {
	/^\s*\(\s*$types/;
}
# base insn_list? (type_list insn_list insn_list?)+ ')'
sub constructor {
	return unless($base = &name);

	# need to look ahead to see if it is a type-list
	@macro = &insn_list unless &is_typelist;
	do {
		$iname = $rname = "";	# reinitialize
		&expect("(");	
		@type_list = &type_list;
		if(&is_cmv && $macro{"synthetic_cmv"}) {
			/^\s*\)/ || die "Malformed cmove: only allowed typelist\n";
		} else {	
			($rname = &name) || (@ireg_list = &insn_list) || ($#macro >= 0) ||
					die "must have an instruction list\n";
			# optional immediate stuff
			/^\s*\)/ || ($iname = &name) || (@iimm_list = &insn_list);
		}
		&output;
		&expect(")");
	} while(/^\s*\(/);
	@macro = @ireg_list = @iimm_list = ();
	1;	
}


# remap operands
sub redef {
	local($i, $imm, *decls) = @_;

	if($i =~ /^v_/) {
        	LOOP: while($i =~ s/\b($operands)\b/"_v_$1_$base$imm"/e) {
                	next LOOP if(defined $decls{$1});
                	$decls{$1} = "\t$vdecl_type{$1} _v_$1_$base$imm = ($vdecl_type{$1})$1;\t\\\n";
        	}
	} else {
        	LOOP: while($i =~ s/\b($operands)\b/"_v__$1_$base$imm"/e) {
                	next LOOP if(defined $decls{$1});
			$decls{$1} = "\t$decl_type{$1} _v__$1_$base$imm = ($1)$access{$1};\t\\\n";
        	}
	}
	$i;
}

sub pick {
	local($one, $two, $proto) = @_;

	if($proto =~ /\b$one\b/) {
		$one;
	} else {
		($proto =~ /\b$two\b/) || die "neither $one nor $two is present in $proto\n";
		$two;
	}
}

# The fun begins here:
# 	for v_* functions, we move rd -> _v_rd; rs -> _v_rs; etc.
# 	for non v_* functions rd -> __rd
# If a pattern is supplied, then there must be a replacement function.
sub mkbody {
	local($name, $proto, $imm, @ilist) = @_;
	local($decl, $body, %decls) = "";

	if($pseudo_imm) {
		local($tmp) = $proto;
		# replace imm with v_at: this is pretty broken, since we only key
		# off of a single imm (i.e., we ignore `imm1' and `imm2').  Future
		# work.
		$tmp =~ s/\b(imm)\b/v_at/g;
		$body = "(v_at, imm); v_$base:$tmp";
	# synthesize a conditional move.
	} elsif($cmv) {
		$body = $cmv_body;
		$body =~ s/#placeholder#/$cmv_brcs{$base}/eg;
	# we either have a name or an instruction list
	} elsif($name || $#ilist < 0) {
		local(@elist) = split('#', $macro{$name});

		$name = ($macro{$name} || $name);
		# if there is a macro, plug name in for @
		if($#macro >= 0) {
			($#elist < 1) || ($name =~ /[()]/) ||
				die "can only substitute simple names\n";
			$body = join(';', @macro);
			$body =~ s/@/$name/g;
 		# is a vcode insn or a machine insn with an analogous type signature.
		} else {
			# if it is more than a simple name or there are
			# multiple instructions
			if($#elist > 0 || $name =~ /[()]/) {
				$body = join(";\t\\\n\t", @elist);	
			} else {
				$body = $name . $proto;
			}
		}
	# for the instruction list, go through and substitute; 
	} else {
		$#ilist >= 0 || die "ilist must be non-nil\n";
		$body = join(";\t\\\n\t", @ilist);	
	}  

	# we do two transformations:  operands contained within vcode 
	# instructions are are replaced with references to __v_#name;  
	# operands contained within all other instructions are replaced
	# with references to __v__#name.  

	# include whichever one is in the prototype
	$body =~ s/\b($operands)_or_($operands)\b/(&pick($1, $4, $proto))/ge;

	# remap operands
	$body =~ s/\b([\w:]+\([^)]*\))/(&redef($1, $imm, *decls))/ge;
	foreach (keys %decls) {
		$decl .= $decls{$_};
	}
	# apply each type.
	LOOP: foreach (@type_list) {
		# We look for names containing an `:' --- `:' is replaced with
		# the current type.
		$p = $body;	# make a copy
		# XXXXYYYYY
		next LOOP if($imm && !&is_mem && ($_ eq "f" || $_ eq "d"));
		# next LOOP if($imm && !&is_mem);
		# if pseudo-imm, then have to insert the required load constant insn
		if($pseudo_imm) {
			next LOOP if($_ eq "f" || $_ eq "d");
			$p = $loadt{$_} . $p;
		}
		$p =~ s/(v_\w*)(:)(\w*)/$1$_$3/g;
		print<<"EOF";
#define v_$base$_$imm$proto do {		\\
$decl					\\
	$p;	\\
} while(0)
EOF
	}
}

# Apply each type to the macro pattern.
sub output {
	local($proto) = $proto{$class{$base}};
	local($body, $decl, $p);

	if(&is_mem && $macro{"no_reg_plus_reg"} == 1) {
		if($rname) {
			!$iname || die "bogus iname = <$iname>\n";
			$iname = $rname;
			$rname = "";
		} else {
			!$iname || die "bogus iname = <$iname>\n";
			@iimm_list = @ireg_list;
		}
		@ireg_list = ("v_addl(v_at, rs1, rs2)", "v_$base:i(rd, v_at, 0)");
	} 

	# special cased cmv
	if(&is_cmv && $macro{"synthetic_cmv"}) {
		$cmv = 1;
		# do all four combinations
		$cmv_body = $cmv_reg; 
		&mkbody("", $proto{$class{$base}}, "", @ireg_list);

		$cmv_body = $cmv_imm1; 
		&mkbody("", $cmv_imm1_proto, "ri", @ireg_list);

		$cmv_body = $cmv_imm2;
		&mkbody("", $cmv_imm2_proto, "ir", @ireg_list);

		$cmv_body = $cmv_imm3;
		&mkbody("", $cmv_imm3_proto, "ii", @ireg_list);

		$cmv = 0;
	} else {
		local($protoi) = $protoi{$class{$base}};
		# encoded knowlege that v_set*i do not have non-immediate instances
		if($base eq "set") {
			&mkbody($rname, $protoi, "", @ireg_list);
		} else {
			&mkbody($rname, $proto{$class{$base}}, "", @ireg_list);
			# pseudo-imm
			if(!$iname && $#iimm_list < 0) {
				$pseudo_imm = 1;
				&mkbody($iname, $protoi, "i", @iimm_list);
				$pseudo_imm = 0;
			} else {
				&mkbody($iname, $protoi, "i", @iimm_list);
			}
		}
	}
}

sub initialize {
	# initialiation 
	$operands 	= "(label|rs|rs1|rs2|rs3|imm|imm1|fimm|imm2|rd1|rd2|rd|b1|b2)";
	$types  	= "(c|uc|s|us|i|u|ul|l|p|f|d)";

	# Defines how to access a variable of each type.
	if ($fastp) {
	    %access = (
		       "rd", "", 
		       "rd1", "", 
		       "rd2", "", 
		       "rs",  "",
		       "rs1", "",
		       "rs2", "",
		       "rs3", "",
		       "b1",   "",
		       "b2",   "",
		       "label", "", 		# .label",
		       );
	} else {
	    %access = (
		       "rd", ".reg", 
		       "rd1", ".reg", 
		       "rd2", ".reg", 
		       "rs",  ".reg",
		       "rs1", ".reg",
		       "rs2", ".reg",
		       "rs3", ".reg",
		       "b1",   ".reg",
		       "b2",   ".reg",
		       "label", "", 		# .label",
		       );
	}
	%decl_type = (
        	"rd", "unsigned", 
        	"rd1", "unsigned", 
        	"rd2", "unsigned", 
        	"rs",  "unsigned",
        	"rs1", "unsigned",
        	"rs2", "unsigned",
        	"rs3", "unsigned",
        	"b1",   "unsigned",
        	"b2",   "unsigned",
		"uimm", "unsigned long",
		"imm1",	"long",
		"imm2",	"long",
		"imm",	"long",
		"fimm",	"double",
		"uimm1", "unsigned long",
		"uimm2", "unsigned long",
        	"label", "v_label_type", 		# .label",
	);
	%vdecl_type = (
        	"rd", "v_reg_type", 
        	"rd1", "v_reg_type", 
        	"rd2", "v_reg_type", 
        	"rs",  "v_reg_type",
        	"rs1", "v_reg_type",
        	"rs2", "v_reg_type",
        	"rs3", "v_reg_type",
        	"b1",   "v_reg_type",
        	"b2",   "v_reg_type",
		"uimm", "unsigned long",
		"imm1",	"long",
		"imm2",	"long",
		"imm",	"long",
		"fimm",	"double",
		"uimm1", "unsigned long",
		"uimm2", "unsigned long",
        	"label", "v_label_type", 		# .label",
	);

	# Defines how to load each immediate.
	%loadt = (
        	"c",    "v_seti",
        	"uc",   "v_setu",
        	"s",    "v_seti",
        	"us",   "v_setu",
        	"i",    "v_seti",
        	"u",    "v_setu",
        	"l",    "v_setl",
		"p", 	"v_setp",
        	"ul",   "v_setul",
        	"f",   "v_setf",
        	"d",   "v_setd",
	);

	# Defines the prototype information for each class of vcode instruction
	%gens = (
		"nulary", 	"rs",
		"unary",  	"rd rs",
		"binary", 	"rd rs1 rs2",
		"branch",	"rs1 rs2 label",
		"ternaryI",	"rd rd2 rs1 rs2",
		"ternaryII",	"rd rs b1 b2",
		"ternaryIII",	"rd rs1 rs2 rs3",
	);
	%gensi = (
		"nulary", 	"imm",
		"unary",  	"rd imm",
		"binary", 	"rd rs1 imm",
		"branch",	"rs1 imm label",
		"ternaryI",	"rd rd2 rs1 imm",
		"ternaryII",	"DO BY HAND",
		"ternaryIII", 	"BOGUS",
	);

	# Used if synthetic_cmv is defined -- gives the branching instruction used
	# to synthesize the conditional move.
	%cmv_brcs = (
		"cmveq", "v_bne:",
		"cmvne", "v_beq:",
		"cmvlt", "v_bge:",
		"cmvle", "v_bgt:",
		"cmvgt", "v_ble:",
		"cmvge", "v_blt:"
	);
	$cmv_reg = 	"v_label_type lab = v_genlabel();    \\\n" . 
        		"\t#placeholder#(b1, b2, lab);        \\\n" .
        		"\tv_nop();                                \\\n" .
        		"\tv_mov:(rd, rs);                     \\\n" .
        		"\tv_label(lab)";
	# only the branch can take an immediate
	$cmv_imm1_proto = "(rd, rs, b1, imm)";
	$cmv_imm1 = 	"v_label_type lab = v_genlabel();    \\\n" . 
        		"\t#placeholder#i(b1, imm, lab);        \\\n" .
        		"\tv_nop();                                \\\n" .
        		"\tv_mov:(rd, rs);                     \\\n" .
        		"\tv_label(lab)";
	# only the move can take an immediate
	$cmv_imm2_proto = "(rd, imm, b1, b2)";
	$cmv_imm2 = 	"v_label_type lab = v_genlabel();    \\\n" . 
        		"\t#placeholder#(b1, b2, lab);        \\\n" .
        		"\tv_nop();                                \\\n" .
        		"\tv_set:(rd, imm);                     \\\n" .
        		"\tv_label(lab)";
	# both the move and the branch can take immediates
	$cmv_imm3_proto = "(rd, imm1, b1, imm2)";
	$cmv_imm3 = 	"v_label_type lab = v_genlabel();    \\\n" . 
        		"\t#placeholder#i(b1, imm2, lab);        \\\n" .
        		"\tv_nop();                                \\\n" .
        		"\tv_set:(rd, imm1);                     \\\n" .
        		"\tv_label(lab)";

	# all the different types
	&construct("binary", "add sub mul mulhi div mod ld st uld ust and or xor");
	&construct("binary", "andnot ornot xornot nor nand nxor lsh rsh lt le ge gt eq ne");
	&construct("branch", "beq bne blt ble bgt bge");
	&construct("unary",  "com not mov neg cvc2 cvs2 cvus2 cvi2 cvu2 cvl2 cvul2 cvp2 cvf2");
	&construct("nulary", "ret");
	&construct("unary",  "cvd2 set abs neg nabs ceil floor sqrt mov");
	&construct("ternaryI", "mulhilo divmod");
	&construct("ternaryII", "cmveq cmvne cmvlt cmvle cmvgt cmvge");
	&construct("ternaryIII", "muladd mulsub negmuladd negmulsub");
}
	
sub construct {
	local($class, $bases) = @_;
	# make protos
	foreach (keys %gens) {
		$proto{$_} = "(" . join(',', split(' ', $gens{$_})) . ")";
	}
	foreach (keys %gens) {
		$protoi{$_} = "(" . join(',', split(' ', $gensi{$_})) . ")";
	}
	foreach (split(' ', $bases)) {
		$class{$_} = $class;
	}
}

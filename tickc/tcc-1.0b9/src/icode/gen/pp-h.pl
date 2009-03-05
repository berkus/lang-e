#!perl

# $Id: pp-h.pl,v 1.1 1998/05/06 19:04:02 maxp Exp $

# gen-pp: generates elements of a switch statement useful in
# pretty-printing icode.

$opcode = 0;
$opname = $fname = "";

sub printdef {
    local($t, $n) = @_;
    $n =~ s/f_/f_d/g;
    print <<"EOF"
case i_op_$opname$t:
	$n(cp, "$opname$t");
	break;
EOF
    ;
}

while (<STDIN>) {
    /^;/ && next;
    if ( /^\(format(i?) (\w+) (\([\w\s]+\))/ ) {
	$imm = $1;		# match "(format <format name> (<args>))"
	$fname = $2;
    }
    elsif ( /^\((\w+)\s*\(([\w\s\(\)]+)\)\)\s*$/ ) {
				# match "(opcode (types (types-in-paren) ...))"
	!$fname && die("syntax error\n");
	$opname = $1; $_ = $2;
	if ($2) {
	    do {
		s/(.*)(\(.+\))(.+)/\1\3\2/g;
	    } until /^[^\(\)]*(\([^\(\)]*\))*$/;
	}
				# put into canonical form: t1:t2:..:tn|u1:..:un
				# where u* are types-in-paren and t* are not
	s/\s+/:/g; s/[\(\)]/\|/g; s/\|\|/:/g; s/(:\||\|:)/\|/g; s/^://g;
	$phase = 0;		# phase==0: std ops; phase==1: immediate ops
	foreach $xty (split(/\|/,$_)) {
	    $phase > 1 && die("bad phase number\n");
	    foreach $ty (split(/:/,$xty)) {
		&printdef("$ty", "$fname");
		if (!$phase && !$imm) {
				# print immediate ops
		    &printdef("${ty}i", "${fname}i");
		}
	    }
	    ++$phase;
	}
    }
}

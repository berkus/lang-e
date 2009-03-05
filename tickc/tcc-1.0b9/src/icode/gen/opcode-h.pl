#!perl

# $Id: opcode-h.pl,v 1.1 1998/05/06 19:04:01 maxp Exp $

$opcode = 0;
$opname = $fname = "";

sub printdef {
    local($t, $op) = @_;
    print "\ti_op_$opname$t=$op,\n";
}

print "enum {\n";
while (<STDIN>) {
    /^;/ && next;
    if ( /^\(misc (\w+)\)/ ) {
	$opname = $1;
	&printdef("", $opcode++);
    } elsif ( /^\(format(i?) (\w+) (\([\w\s]+\))/ ) {
				# match "(format <format name> (<args>))"
	$imm = $1;
	$fname = $2;
    }
    elsif ( /^\((\w+)\s*\(([\w\s\(\)]+)\)\)\s*$/ ) {
				# match "(opcode (types (types-in-paren) ...))"
				# types-in-paren do not have immediate ops,
				# others do.
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
				# print standard ops
		&printdef("$ty", $opcode);
		if (!$phase && !$imm) {
				# print immediate ops
		    $immop = $opcode+512;
		    &printdef("${ty}i", $immop);
		}
		$opcode++;
	    }
	    ++$phase;
	}
    }
}
print "\ti_op_last\n};\n";

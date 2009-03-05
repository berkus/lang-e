#!perl

# $Id: macros-gen-h.pl,v 1.1 1998/05/06 19:03:55 maxp Exp $

$opcode = 0;
$opname = "";
$fname = $fargs = $margs = "";

sub printdef {
    local($t, $n, $op) = @_;
    $_ = "$opname$t";
    /(set[fd]|callv|retv)/ && return;	# these are special cases; do by hand
    print "#define i_$opname$t$fargs $n$margs\n";
}
while (<STDIN>) {
    /^;/ && next;
    if ( /^\(misc/ ) {
	$opcode++;
    } elsif ( /^\(format(i?) (\w+) (\([\w\s]+\))/ ) {
				# match "(format <format name> (<args>))"
	$imm = $1;
	$fname = $2; $fargs = "";
	if ($3) {
	    $_ = $3; s/\s+/,/g;
	    $fargs = $_;
	}
    } elsif ( /^\((\w+)\s*\(([\w\s\(\)]+)\)\)\s*$/ ) {
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
		$margs = $fargs; $margs =~ s/\(/\($opcode,/g;
		&printdef("$ty", "$fname", $opcode);
		if (!$phase && !$imm) {
				# print immediate ops
		    $immop = $opcode+512;
		    $margs = $fargs; $margs =~ s/\(/\($immop,/g;
		    &printdef("${ty}i", "${fname}i", $immop);
		}
		++$opcode;
	    }
	    ++$phase;
	}
    }
}

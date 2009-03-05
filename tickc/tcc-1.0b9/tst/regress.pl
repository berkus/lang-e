#!/usr/uns/bin/perl

# $Id: regress.pl,v 1.1.1.1 1997/11/10 23:43:21 maxp Exp $


$usage = <<"EOF"

$0: [-d(ebug)] [-h(elp)]

This script runs regression tests by parsing the file given on STDIN.
The file can contain any number of entries having the following syntax:

src: <basename>.{c,tc}
[tcc: [<options>]]
[err: [<name0>]]
[asm: [<name1>]]
[bin: [<name2>]]
[arg: [<arguments>]]
[err0: [<name3>]]
[out0: [<name4>]]
[gprof: ]
[notcc: ]
[nodiff: ]

The only required field is src:.  By default, the script will compile
<basename>.{c,tc} to the executable file <basename>, run it with no 
arguments, and compare the output to the file <basename>.0.out.
src: must appear at the head of its entry.

tcc: can be followed by options to pass to tcc when compiling.

If err: is specified, compiler error/warning messages are dumped to file
<name0> (or <basename>.err by default).  This is compared to whatever
name is specified in err0, or <basename>.0.err by default.

If asm: is specified, <basename> is compiled to the assembly file <name1> 
(or <basename>.s by default).  This is compared to whatever is specified in
out0:, or  <basename>.0.s by default.

If bin: is specified, <basename> is compiled to <name2> (or <basename>
by default).  bin: and asm: are mutually exclusive.

If arg: is specified, the given arguments are passed to the executable
when it is run.  arg: is ignored if asm: is specified.

If err0: is specified and <name3> is given, <name3> is used for comparison
of compiler errors/warnings rather than <basename>.0.err.

If out0: is specified and <name4> is given, <name4> is used for comparison 
of output rather than <basename>.0.out or <basename>.0.s

If gprof: is specified, assumes that a gmon.out file is created when
basename is run, and invokes 'gprof <basename> > <basename>.prof'.

If notcc: is specified, the source is not compiled; 'basename' is simply
executed.

If nodiff: is specified, does not invoke 'diff' on the output of 'basename'. 
EOF
    ;

$tcc = "tcc";
$tcccom = $tccopts = "";
$debug = $verbose = 0;
@filelist = ();

sub parsecommandline {
    while ($_ = $ARGV[0]) {
	shift @ARGV;
	if (/^-d/)      { $debug++; next; }
	if (/^-v/)      { $verbose++; next; }
	if (/^-h/)      { print $usage."\n"; exit(1); }
	push(@filelist, $_);
    }
}

sub docom {
    local($com) = @_;
    $verbose && print "$com\n"; system($com);
}

sub doentry {
    $dothisentry = 0;
    if ($#filelist == -1) { $dothisentry = 1; }
    else {
	foreach $file (@filelist) {
	    if ($file eq $basename) { $dothisentry = 1; last; }
	}	
    }
    if ($dothisentry == 0) { return; }
    $tcccom = "$tcc $tccopts";
    if (!$out0name) {
	if ($asmname) { $out0name = $basename.".0.s"; } 
	else { $out0name = $basename.".0.out"; }
    }
    if ($errname) { 
	$errcom = "> $errname 2>&1";
	if (!$err0name) { $err0name = $basename.".0.err"; }
    } else { 
	if ($err0name) {
	    $errname = $basename.".err"; $errcom = "> $errname 2>&1";
	} elsif ($verbose) {
	    $errcom = "";
	} else {
	    $errcom = "> /dev/null 2>&1";
	}
    }
    if (! ($asmname || $binname)) { $binname = $basename; }
    if ($debug) {
	print <<"EOF"
tcccom:$tcccom, errcom:$errcom
src:$srcname, base:$basename, asm:$asmname, bin:$binname
out0:$out0name, args:$arg, err:$errname, err0:$err0name

EOF
    ;
    } else {
	print "#\n# $basename\n#\n";
	if ($asmname) {
	    &docom("$tcccom -keep -S $srcname $errcom");
	    $_ = $basename; /\/*(\w+\/)(\w+)/; 
	    ($asmname ne "$2.s") && &docom("mv $2.s $asmname");
	    &docom("diff $asmname $out0name");
	} else {
	    if (!$notcc) { 
		&docom("$tcccom -keep -o $binname $srcname $errcom"); 
	    }
	    if (!$errname) {
		&docom("./$binname $arg > $basename.out 2>&1");
		if (!$nodiff) { &docom("diff $out0name $basename.out"); }
		if ($gprof) { &docom("gprof ./$binname > $binname.prof"); }
	    }
	}
	$errname && &docom("diff $errname $err0name");
    }
}

sub error {
    print STDERR @_; print STDERR "\n";
    exit(-1);
}

&parsecommandline();
$inentry = 0;
$line = 0;
while (<>) {
    $line++;
    if (/^src:\s*(.*)\.(\w+)/) {
	if ($inentry) { &doentry(); }
	$inentry = 1;
	$basename = $1;	$srcname = "$1.$2"; $asmname = "";
	$binname = "";	$out0name = "";	    $arg = "";
	$errname = "";   $err0name = "";	    $gprof = "";
	$notcc = "";	$nodiff = "";
	$tccopts = "";
	next;
    }
    if (/^tcc:\s*([\s\S]+)$/) {
	$inentry || &error("Line $line: Bad asm entry");
	$tccopts = $1;
	chop $tccopts;
	next;
    }	
    if (/^err:\s*([\s\S]+)$/) {
	$inentry || &error("Line $line: Bad err entry");
	$_ = $1;
	if (/\S+/) { $errname = $_; chop $basename; } 
	else { $errname = $basename.".err"; }
	next;
    }
    if (/^asm:\s*([\s\S]+)$/) {
	$inentry || &error("Line $line: Bad asm entry");
	$binname && &error("Line $line: Cannot specify asm and bin together");
	$_ = $1;
	if (/\S+/) { $asmname = $_; chop $asmname; } 
	else { $asmname = $basename.".s"; }
	next;
    }
    if (/^bin:\s*([\s\S]+)$/) {
	$inentry || &error("Line $line: Bad bin entry");
	$asmname && &error("Line $line: Cannot specify asm and bin together");
	$_= $1;
	if (/\S+/) { $binname = $_; chop $binname; } 
	else { $binname = $basename; }
	next;
    }
    if (/^err0:\s*([\s\S]+)$/) {
	$inentry || &error("Line $line: Bad err0 entry");
	$_ = $1;
	if (/\S+/) { $err0name = $_; chop $err0name; }
	next;
    }
    if (/^out0:\s*([\s\S]+)$/) {
	$inentry || &error("Line $line: Bad out0 entry");
	$_ = $1;
	if (/\S+/) { $out0name = $_; chop $out0name; }
	next;
    }
    if (/^arg:(.*)$/) {
	$inentry || &error("Line $line: Bad arg entry");
	$arg = $1;
    }
    if (/^gprof:(.*)$/) {
	$inentry || &error("Line $line: Bad gprof entry");
	$gprof = 1;
    }
    if (/^notcc:(.*)$/) {
	$inentry || &error("Line $line: Bad notcc entry");
	$notcc = 1;
    }
    if (/^nodiff:(.*)$/) {
	$inentry || &error("Line $line: Bad nodiff entry");
	$nodiff = 1;
    }
}
if ($inentry) { &doentry(); }

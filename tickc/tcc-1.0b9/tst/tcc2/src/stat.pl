#!/usr/local/bin/perl

# $Id: stat.pl,v 1.1 1998/02/26 03:41:48 maxp Exp $

use Getopt::Long;

#
# DATA
#

$bdata = `cat /home/tickc/xtcc/tst/tcc2/src/data.simplesim`;
eval "%benchmarks = $bdata"; if ($@) { die $@; }

$prefix = `pwd`;
chomp $prefix;
%paths = (
	  icode_gcc	=>	"$prefix/icode-c",
	  icode_lcc	=>	"$prefix/icode",
	  vcode_gcc	=>	"$prefix/vcode-c",
	  vcode_lcc	=>	"$prefix/vcode",
	  );

if (! -e $paths{"icode_gcc"}) {
    print("There is no directory '$paths{icode_gcc}'.\n");
    die("You should run this script in a tcc benchmark directory.\n");
}


#
# CODE
#

sub dumpBenchmarkInfo {
    my $x;
    print OUTFD "Time: ".`date`;
    print OUTFD "Platform: ".`uname -a`;
    print OUTFD "Simulator: $sim\n";
    print OUTFD "Arguments:\n";
    foreach $benchname (@_) {
	print OUTFD "\t$benchname: compiles=$benchmarks{$benchname}->{comp} ".
	    "runs=$benchmarks{$benchname}->{runs} ".
            "data=$benchmarks{$benchname}->{data}\n";
    }
    print OUTFD "Modes: ";
    foreach $x (@modes) { printf OUTFD "$x "; }
    printf OUTFD "\n";
}

#
# MAIN
#

@benchs = (); @modes = (); @nots = ();
$outfile = "";
				# Parse arguments
$Getopt::Long::autoabbrev = 1;
&GetOptions("bench=s@", \@benchs, 
	    "mode=s@", \@modes,
	    "nots=s@", \@nots,
	    "outfile=s", \$outfile, 
	    "sim=s", \$sim,
	    );

$outfile = "$prefix/benchmark-results" unless ($outfile ne "");
$sim = "sim-safe" unless ($sim ne "");
foreach $notelt (@nots) { $notthis{$notelt} = 1; }

				# Decide what to measure
@benchs = sort(keys %benchmarks) unless (@benchs != ());
@modes = ("icode_gcc", "vcode_gcc") 
    unless (@modes != ());

open OUTFD, ">$outfile";
				# Run benchmarks
&dumpBenchmarkInfo(@benchs);
foreach $benchname (@benchs) {
    $b = $benchmarks{$benchname};
    next if ($notthis{$benchname});
    foreach $mode (@modes) {
	$xsim = "$sim -notrace -statsfile $prefix/$benchname.$mode.stats ";
	$prog = "$b->{prog} -data $b->{data} -compiles $b->{comp} ".
	    "-runs $b->{runs}";
	
	chdir $paths{$mode};

	system "$xsim $prog";	# dynamic
	system "$xsim $prog -s";# static
    }
}

close OUTFD;

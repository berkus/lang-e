#!/usr/local/bin/perl

# $Id: sstat.pl,v 1.1 1998/02/26 03:41:47 maxp Exp $

use Getopt::Long;

#
# DATA
#

$bdata = `cat /home/tickc/xtcc/tst/tcc2/src/data.simplesim`;
eval "%benchmarks = $bdata"; if ($@) { die $@; }

$prefix = `pwd`;
chomp $prefix;

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
	print OUTFD "\t$benchname: ".
	    "runs=$benchmarks{$benchname}->{runs} ".
            "data=$benchmarks{$benchname}->{data}\n";
    }
    printf OUTFD "\n";
}

#
# MAIN
#

@benchs = (); @nots = ();
$outfile = "";
				# Parse arguments
$Getopt::Long::autoabbrev = 1;
&GetOptions("bench=s@", \@benchs, 
	    "nots=s@", \@nots,
	    "outfile=s", \$outfile, 
	    "sim=s", \$sim,
	    );

$outfile = "$prefix/stat-info" unless ($outfile ne "");
$sim = "sim-safe" unless ($sim ne "");
foreach $notelt (@nots) { $notthis{$notelt} = 1; }

				# Decide what to measure
@benchs = sort(keys %benchmarks) unless (@benchs != ());

open OUTFD, ">$outfile";
				# Run benchmarks
&dumpBenchmarkInfo(@benchs);
foreach $benchname (@benchs) {
    $b = $benchmarks{$benchname};
    next if ($notthis{$benchname});
    $xsim = "$sim -notrace -statsfile $prefix/$benchname.c.stats ";
    $prog = "$b->{prog} -data $b->{data} -runs $b->{runs}";
    system "$xsim $prog";
}

close OUTFD;

#!/usr/uns/bin/perl

# $Id: sbench-sparc-solaris.pl,v 1.1 1998/02/26 03:46:34 maxp Exp $

use Getopt::Long;

#
# DATA
#

$bdata = `cat /home/tickc/xtcc/tst/tcc2/src/data.sparc-solaris`;
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
    print OUTFD "Arguments:\n";
    foreach $benchname (@_) {
	print OUTFD "\t$benchname: ".
	    "runs=$benchmarks{$benchname}->{runs} ".
            "data=$benchmarks{$benchname}->{data}\n";
    }
    print OUTFD "Reps: $reps\n";
    print OUTFD "\n";
}

sub mt1 {			# measure time for 1 time output
    my ($com) = @_;
    my ($j, @times, $nl, $time);
    for ($j = 0; $j < $reps; $j++) {
	$nl = `$com`;
	if ($debug) { print "\n+\n+\nCommand=$com\nOutput=\n$nl\n"; }
	$nl =~ /.*user time used ([0-9\.]+)\s+system time used ([0-9\.]+).*/;
	push @times, $1+$2;
    }
    foreach $time (@times) { chomp $time; print OUTFD "$time "; }
    print OUTFD "\n";
}

sub getCost {
    my ($b, $sys) = @_;
    print OUTFD "Cost: ";
    &mt1("$b->{prog} -data $b->{data} -runs $b->{runs}");
}

#
# MAIN
#

$reps = 0; @benchs = (); @nots = ();
$outfile = "";
				# Parse arguments
$Getopt::Long::autoabbrev = 1;
&GetOptions("bench=s@", \@benchs, 
	    "debug", \$debug,
	    "nots=s@", \@nots,
	    "outfile=s", \$outfile, 
	    "reps=i", \$reps,
	    );

$reps = 1 unless ($reps != 0);
$outfile = "$prefix/benchmark-results" unless ($outfile ne "");
foreach $notelt (@nots) { $notthis{$notelt} = 1; }

				# Decide what to measure
@benchs = sort(keys %benchmarks) unless (@benchs != ());

open OUTFD, ">$outfile";
				# Run benchmarks
&dumpBenchmarkInfo(@benchs);
foreach $benchname (@benchs) {
    $bench = $benchmarks{$benchname};
    next if ($notthis{$benchname});

	print OUTFD "\nBenchmark: $benchname; Run: $bench->{runs}\n";
	&getCost($bench, $mode);
}

close OUTFD;

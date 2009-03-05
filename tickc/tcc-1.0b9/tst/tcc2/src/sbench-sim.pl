#!/usr/local/bin/perl

# $Id: sbench-sim.pl,v 1.1 1998/02/26 03:46:34 maxp Exp $

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
    print OUTFD "Reps: $reps\n";
    print OUTFD "\n";
}

sub mt1 {			# measure time for 1 time output
    my ($com) = @_;
    my ($j, @times, $nl, $time);
    for ($j = 0; $j < $reps; $j++) {
	$nl = `$sim $com 2>>$logfile`;
	if ($debug) { print "\n+\n+\nCommand=$com\nOutput=\n$nl\n"; }
	$nl =~ /.*Number of cycles: ([\d\.e+]+)/;
	push @times, $1;
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
$outfile = ""; $logfile = "";
				# Parse arguments
$Getopt::Long::autoabbrev = 1;
&GetOptions("bench=s@", \@benchs, 
	    "debug", \$debug,
	    "logfile=s", \$logfile,
	    "nots=s@", \@nots,
	    "outfile=s", \$outfile, 
	    "reps=i", \$reps,
	    "sim=s", \$sim,
	    );

$reps = 1 unless ($reps != 0);
$outfile = "$prefix/benchmark-results" unless ($outfile ne "");
$sim = "sim-outorder" unless ($sim ne "");
$logfile = "$prefix/benchmark-log" unless ($logfile ne "");
foreach $notelt (@nots) { $notthis{$notelt} = 1; }

				# Decide what to measure
@benchs = sort(keys %benchmarks) unless (@benchs != ());

system "touch $logfile";
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

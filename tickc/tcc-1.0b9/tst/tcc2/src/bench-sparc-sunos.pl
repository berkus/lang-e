#!/usr/local/bin/perl

# $Id: bench-sparc-sunos.pl,v 1.3 1998/02/26 03:41:29 maxp Exp $

use Getopt::Long;

#
# DATA
#

$bdata = `cat /home/tickc/xtcc/tst/tcc2/src/data.sparc-sunos`;
eval "%benchmarks = $bdata"; if ($@) { die $@; }

$prefix = `pwd`;
chomp $prefix;
$cyclespersec = 7.e+7;
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

sub countDynInsns {
    my ($b, $sys) = @_;
    chdir $paths{$sys};
    my $nl = `$b->{exec} -data $b->{data} -compiles 1 -runs 1 -count`;
    $nl =~ /.*Number of insns: ([0-9]+).*/; $b->{$sys}->{insns} = $1;
    if ($verbose) { print "\tDynamic code length: $b->{$sys}->{insns}\n"; }
}

sub mean {
    my $cnt = 0;
    foreach (@_) { $cnt += $_; }
    my $mean = $cnt/@_;
    my $sd = 0;
    if ($#_) {
	my $tot = 0;
	foreach (@_) { $tot += ($_ - $mean)**2; }
	$sd = sqrt($tot/$#_);
    }
    my $pc = ($sd/$mean)*100;
    if ($debug || $verbose) { 
	print "\t\tMean of (@_) is $mean\n"; 
	print "\t\tStd dev is $sd ($pc% of mean)\n"; 
    }
    return $mean;
}

sub mt1 {			# measure time for 1 time output
    my ($com) = @_;
    my ($j, @times, $nl);
    for ($j = 0; $j < $reps; $j++) {
	$nl = `$com`;
	if ($debug) { print "++++\n\t$com\n$nl++++\n"; }
	$nl =~ /.*user time used ([0-9\.]+)\s+system time used ([0-9\.]+).*/;
	push @times, ($1+$2);
    }
    return &mean(@times);
}

sub mt2 {			# measure time for 2 time outputs
    my ($com) = @_;
    my ($j, @times, $nl, $cnt);
    for ($j = 0; $j < $reps; $j++) {
	@nl = `$com`;
	if ($debug) { print "++++\n\t$com\n@nl++++\n"; }
	$cnt = 0;
	grep {
	    if (/.*user time used ([0-9\.]+)\s+system time used ([0-9\.]+)/) {
		push @{ $times[$cnt++] }, ($1+$2);
	    }
	} @nl;
    }
    return (&mean(@{$times[0]}), &mean(@{$times[1]}));
}

sub getCostClosures {
    my ($b, $sys) = @_;
    chdir $paths{$sys};
    my $time = 
	&mt1("$b->{exec} -data $b->{data} -compiles $b->{comp} -onlyClosures");
    $b->{$sys}->{timeClosures} = $time/$b->{comp};
    if ($verbose) { print "\tTime to manipulate closures: ".
			"$time ($b->{comp} x $b->{$sys}->{timeClosures})\n"; }
}

sub getCostIcode {
    my ($b, $sys) = @_;
    chdir $paths{$sys};
    my (%t, $nl);
				# Cost of IR
    $t->{Closures} = ($b->{comp}*$b->{$sys}->{timeClosures});
    $t->{IR} = 
	&mt1("$b->{exec} -data $b->{data} -compiles $b->{comp} -onlyIR");
    $t->{IR} -= $t->{Closures};
    $b->{$sys}->{timeIR} = $t->{IR}/$b->{comp};

				# Cost of linear ralloc
    $t->{OptLin} =
	&mt1("$b->{exec} -data $b->{data} -compiles $b->{comp} -onlyOpt -rallocLin");
    $t->{OptLin} = $t->{OptLin} - $t->{IR} - $t->{Closures};
    $b->{$sys}->{timeOptLin} = $t->{OptLin}/$b->{comp};

				# Cost of graph coloring
    $t->{OptGC} =
	&mt1("$b->{exec} -data $b->{data} -compiles $b->{comp} -onlyOpt -rallocGC");
    $t->{OptGC} = $t->{OptGC} - $t->{IR} - $t->{Closures};
    $b->{$sys}->{timeOptGC} = $t->{OptGC}/$b->{comp};

    if ($verbose) { 
	print "\tTime for icode IR: $t->{IR} ($b->{comp} x $b->{$sys}->{timeIR})\n";
	print "\tTime for icode lr2: $t->{OptLin} ($b->{comp} x $b->{$sys}->{timeOptLin})\n";
	print "\tTime for icode gc: $t->{OptGC} ($b->{comp} x $b->{$sys}->{timeOptGC})\n";
    }	
}

sub getCostDynamic {
    my ($b, $sys) = @_;
    chdir $paths{$sys};
    my (%t, $nl);

    if ($sys =~ /icode/) {
	if ($debug) { print "\tFinal costs for icode phase: $sys\n"; }
	$t->{Init} = ($b->{comp} * ($b->{$sys}->{timeClosures}
				    + $b->{$sys}->{timeIR}
				    + $b->{$sys}->{timeOptLin}));
    } else {
	if ($debug) { print "\tFinal costs for icode phase: $sys\n"; }
	$t->{Init} = ($b->{comp} * ($b->{$sys}->{timeClosures}));
    }
    
    ($t->{Comp}, $t->{RunsDyn}) = 
	&mt2("$b->{exec} -data $b->{data} -compiles $b->{comp} -runs $b->{runs}");
    $t->{GenCode} = $t->{Comp} - $t->{Init};
    $b->{$sys}->{timeGenCode} = $t->{GenCode}/$b->{comp};
    $b->{$sys}->{timeComp} = $t->{Comp}/$b->{comp};
    $b->{$sys}->{timeRunsDyn} = $t->{RunsDyn}/$b->{runs};
    my $insnspersec = $b->{$sys}->{insns} / $b->{$sys}->{timeComp};
    $b->{$sys}->{cyclesPerInsn} = $cyclespersec/$insnspersec;

    if ($verbose) {
	print "\tTime to generate code: $t->{GenCode} ($b->{comp} x $b->{$sys}->{timeGenCode})\n";
	print "\tTotal time to compile: $t->{Comp} ($b->{comp} x $b->{$sys}->{timeComp})\n";
	print "\tCycles per instruction: $b->{$sys}->{cyclesPerInsn}\n";
	print "\tTime to run dynamic code: $t->{RunsDyn} ($b->{runs} x $b->{$sys}->{timeRunsDyn})\n";
    }
}

sub getCostStatic {
    my ($b, $sys) = @_;
    chdir $paths{$sys};
    my $time = 
	&mt1("$b->{exec} -data $b->{data} -runs $b->{runs} -s");
    $b->{$sys}->{timeRunsSta} = $time/$b->{runs};
    if ($verbose) { print "\tTime to run static code: ".
			"$time ($b->{runs} x $b->{$sys}->{timeRunsSta})\n"; }
}

sub dumpInfo {
    my ($name, $b, $sys) = @_;
    if ($sys =~ /icode/) {
	printf outfd "Benchmark: $name; Mode: $sys; Length: $b->{$sys}->{insns}; Closures: %e; IR: %e; LR2: %e; GC: %e; Gen: %e; Compile: %e; CPI: %e; DRun: %e; SRun: %e\n", $b->{$sys}->{timeClosures}, $b->{$sys}->{timeIR}, $b->{$sys}->{timeOptLin}, $b->{$sys}->{timeOptGC}, $b->{$sys}->{timeGenCode}, $b->{$sys}->{timeComp}, $b->{$sys}->{cyclesPerInsn}, $b->{$sys}->{timeRunsDyn}, $b->{$sys}->{timeRunsSta};

    } else {
	printf outfd "Benchmark: $name; Mode: $sys; Length: $b->{$sys}->{insns}; Closures: %e; Gen: %e; Compile: %e; CPI: %e; DRun: %e; SRun: %e\n", $b->{$sys}->{timeClosures}, $b->{$sys}->{timeGenCode}, $b->{$sys}->{timeComp}, $b->{$sys}->{cyclesPerInsn}, $b->{$sys}->{timeRunsDyn}, $b->{$sys}->{timeRunsSta};
    }

}

#
# MAIN
#

$reps = 0; @benchs = (); @modes = (); $outfile = "";
				# Parse arguments
$Getopt::Long::autoabbrev = 1;
GetOptions("bench=s@", \@benchs, "mode=s@", \@modes,
	   "verbose", \$verbose, "debug", \$debug,
	   "outfile=s", \$outfile, "reps=i", \$reps);

$reps = 3 unless ($reps != 0);
$outfile = "$prefix/benchmark-results" unless ($outfile ne "");

				# Decide what to measure
@benchs = keys %benchmarks unless (@benchs != ());
@modes = ("icode_lcc", "vcode_lcc", "icode_gcc", "vcode_gcc") 
    unless (@modes != ());

open outfd, ">$outfile";
				# Run benchmarks
foreach $benchname (@benchs) {
    $bench = $benchmarks{$benchname};
    foreach $mode (@modes) {
	if ($verbose) { 
	    print "=\n=\n===== Benchmark: $benchname; Mode: $mode\n=\n=\n"; 
	}
	&countDynInsns($bench, $mode);
	&getCostClosures($bench, $mode);
	if ($mode =~ /icode/) {	&getCostIcode($bench, $mode); }
	&getCostDynamic($bench, $mode);
	&getCostStatic($bench, $mode);
	&dumpInfo($benchname, $bench, $mode);
    }
}

close outfd;

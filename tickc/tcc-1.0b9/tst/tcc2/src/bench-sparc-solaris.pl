#!/usr/uns/bin/perl

# $Id: bench-sparc-solaris.pl,v 1.7 1998/05/17 20:42:16 maxp Exp $

use Getopt::Long;

#
# DATA
#

$bdata = `cat /home/tickc/xtcc/tst/tcc2/src/data.sparc-solaris`;
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
    print OUTFD "Arguments:\n";
    foreach $benchname (@_) {
	print OUTFD "\t$benchname: compiles=$benchmarks{$benchname}->{comp} ".
	    "runs=$benchmarks{$benchname}->{runs} ".
            "data=$benchmarks{$benchname}->{data}\n";
    }
    print OUTFD "Modes: ";
    foreach $x (@modes) { printf OUTFD "$x "; }
    printf OUTFD "\n";
    print OUTFD "Register allocators: ";
    foreach $x (@rallocs) { printf OUTFD "$x "; }
    printf OUTFD "\n";
    print OUTFD "Reps: $reps\n";
    print OUTFD "\n";
}

sub countDynInsns {
    my ($b, $sys) = @_;
    my ($m, $nl);
    chdir $paths{$sys};
    if ($sys =~ /icode/) {
	foreach $m (@rallocs) {
	    $nl = `$b->{prog} -data $b->{data} -compiles 1 -runs 1 -ralloc$m -count`;
	    $nl =~ /.*Number of insns: ([0-9]+).*/; 
	    $len = $1;
	    print OUTFD "Length ($m): $len\n";
	}
    } else {
	$nl = `$b->{prog} -data $b->{data} -compiles 1 -runs 1 -count`;
	$nl =~ /.*Number of insns: ([0-9]+).*/; 
	$len = $1;
	print OUTFD "Length: $len\n";
    }
}

sub mt1 {			# measure time for 1 time output
    my ($com) = @_;
    my ($j, @times, $nl);
    for ($j = 0; $j < $reps; $j++) {
	$nl = `$com`;
	if ($debug) { print "\n+\n+\nCommand=$com\nOutput=\n$nl\n"; }
	$nl =~ /.*user time used ([0-9\.]+)\s+system time used ([0-9\.]+).*/;
	push @times, ($1+$2);
    }
    foreach $time (@times) { chomp $time; print OUTFD "$time "; }
    print OUTFD "\n";
}

sub mt2 {			# measure time for 2 time outputs
    my ($com) = @_;
    my ($j, @times, $nl, $cnt);
    for ($j = 0; $j < $reps; $j++) {
	@nl = `$com`;
	if ($debug) { print "\n+\n+\nCommand=$com\nOutput=\n@nl\n"; }
	$cnt = 0;
	grep {
	    if (/.*user time used ([0-9\.]+)\s+system time used ([0-9\.]+)/) {
		push @{ $times[$cnt++] }, ($1+$2);
	    }
	} @nl;
    }
    $setname[0] = "DCom";
    $setname[1] = "DRun";
    foreach $set (0..1) {
	print OUTFD "$setname[$set]: ";
	foreach $time (@{$times[$set]}) { chomp $time; print OUTFD "$time "; }
	print OUTFD "\n";
    }
}

sub getCostClosures {
    my ($b, $sys) = @_;
    chdir $paths{$sys};
    print OUTFD "Closures: ";
    &mt1("$b->{prog} -data $b->{data} -compiles $b->{comp} -end_closures");
}

sub getCostIcode {
    my ($b, $sys) = @_;
    chdir $paths{$sys};
    my (%t, $nl, $L, $H, $m, $p);
				# Cost of IR
    print OUTFD "IR: ";
    &mt1("$b->{prog} -data $b->{data} -compiles $b->{comp} -end_IR");
				# Cost of register allocation
    foreach $m (@rallocs) {
	foreach $p ("fg", "lv", "ra1", "ra2") {
	    print OUTFD "Ralloc $m ($p): ";
	    &mt1("$b->{prog} -data $b->{data} -compiles $b->{comp} ".
		 "-end_$p -ralloc$m");
	}
    }
}

sub getCostDynamic {
    my ($b, $sys) = @_;
    chdir $paths{$sys};
    my (%t, $nl, $cL, $cH, $rL, $rH, $m, $ips);

    if ($sys =~ /icode/) {
	foreach $m (@rallocs) {
	    print OUTFD "ICODE ($m):\n";
	    &mt2("$b->{prog} -data $b->{data} -compiles $b->{comp} ".
		 "-runs $b->{runs} -ralloc$m");
	}
    } else {
	print OUTFD "VCODE:\n";
	&mt2("$b->{prog} -data $b->{data} -compiles $b->{comp} ".
	     "-runs $b->{runs}");
    }
}

sub getCostStatic {
    my ($b, $sys) = @_;
    chdir $paths{$sys};
    print OUTFD "Static: ";
    &mt1("$b->{prog} -data $b->{data} -runs $b->{runs} -s");
}

#
# MAIN
#

$reps = 0; @benchs = (); @modes = (); @rallocs = (); $outfile = ""; @nots = ();
				# Parse arguments
$Getopt::Long::autoabbrev = 1;
&GetOptions("allocator=s@", \@rallocs,
	    "bench=s@", \@benchs, 
	    "debug", \$debug,
	    "mode=s@", \@modes,
	    "nots=s@", \@nots,
	    "outfile=s", \$outfile, 
	    "reps=i", \$reps,
	    "verbose", \$verbose
	    );

$reps = 1 unless ($reps != 0);
$outfile = "$prefix/benchmark-results" unless ($outfile ne "");

				# Decide what to measure
@benchs = sort(keys %benchmarks) unless (@benchs != ());
@modes = ("icode_lcc", "vcode_lcc", "icode_gcc", "vcode_gcc") 
    unless (@modes != ());
@rallocs = ("FLR", "Lin", "GC", "None") 
    unless (@rallocs != ());
foreach $notelt (@nots) { 
    @x = split / /, $notelt;
    $notthis{$x[0]}->{flag} = 1;
    $notthis{$x[0]}->{mode} = $x[1];
}

open OUTFD, ">$outfile";
				# Run benchmarks
&dumpBenchmarkInfo(@benchs);
foreach $benchname (@benchs) {
    $bench = $benchmarks{$benchname};
    foreach $mode (@modes) {
	next if ($notthis{$benchname}->{flag} 
		 && $notthis{$benchname}->{mode} eq $mode);
	print OUTFD "\nBenchmark: $benchname; Mode: $mode; ".
	    "Comp: $bench->{comp}; Run: $bench->{runs}\n";
	if ($verbose) { 
	    print "=\n=\n===== Benchmark: $benchname; Mode: $mode\n=\n=\n"; 
	}
	&countDynInsns($bench, $mode);
	&getCostClosures($bench, $mode);
	if ($mode =~ /icode/) {	&getCostIcode($bench, $mode); }
	&getCostDynamic($bench, $mode);
	&getCostStatic($bench, $mode);
    }
}

close OUTFD;

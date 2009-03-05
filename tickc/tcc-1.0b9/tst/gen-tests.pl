#! perl

# Breaks up a large file with blocks separated by /**/ into lots of little
# files, one block per file.  Useful for generating lots of test cases
# (ie regression tests for a compiler) from a unified source file.

# for example: g-t xxx.c x ===> xxx.c x1.c x2.c x3.c ..... xn.c
$usage = $0.": <filename> <destination-prefix>\n";
$breakstring = '^/\*\*/';

sub newdstfile {
    local($counter, $HANDLE, $prefix) = @_;
    local($filename) = $prefix.$counter.".c";

    unless (open($HANDLE,">".$filename)) {
	print STDERR "Can't open destination file: ".$filename.".\n";
	return;
    }
    select($HANDLE);
    $HANDLE;
}

sub breakup {
    local($srcfile, $dstprefix) = @_;
    local($counter) = 0;
    local($SRCHANDLE) = "source";
    local($DSTHANDLE) = "destination";

    unless (open($SRCHANDLE,$srcfile)) {
	print STDERR "Can't open source file: ".$srcfile.".\n";
	return;
    }
    
    $DSTHANDLE = &newdstfile($counter, $DSTHANDLE, $dstprefix);
    while (<$SRCHANDLE>) {
# 	if (m[$breakstring]o) { # perl5ism
	if (/^\/\*\*\//) {
 	    $counter++;
 	    $DSTHANDLE = &newdstfile($counter,$DSTHANDLE,$dstprefix);
 	} else {
 	    print $_;
 	}
    }
    close($DSTHANDLE);
    close($SRCHANDLE);
}

sub parsecommandline {
# effects: returns source file to be broken up
    if ($#ARGV != 1) {
	print $usage;
	exit;
    }
    local($srcfile,$dstprefix) = ($ARGV[0], $ARGV[1]);
}

sub printstatus {
    print "woopdeeda\n";
}

#
# main
#

&breakup(&parsecommandline());

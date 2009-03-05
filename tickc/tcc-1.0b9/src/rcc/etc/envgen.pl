
# maxp, 960306
# last modified: $Date: 1997/10/30 16:17:36 $

# This file is used to generate the header which tcc needs to define its
# internal environment for template compilation.

while (<>) {
    /\S/ || next;		# don't print whitespace
    /#.*/ && next;		# cpp comment? skip it
    /printf/ && next;		# no printf prototypes
    print $_;			# valid line: print it
}

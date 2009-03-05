#!/bin/sh

# $Id: build.sh,v 1.1.1.1 1997/12/05 01:25:44 maxp Exp $

# This is a Bourne shell script for building bpo and related tools.
#
# This script uses nothing other than than the C compiler
# ("CC", defined below) and shell intrinsics, so should be portable.
#
# Although it duplicates some work (i.e. builds files when they are
# up to date), I suggest using it rather than a makefile because
# (1) it should work uniformly across unix and dos/windows/nt platforms
# (2) it is guaranteed to correctly deal with the recursive nature of
#     the bpo build process (i.e. rules being used both as source code
#     and data, and files being "logically" out of date despite
#     up-to-date time stamps because different phases of compilation
#     require different rewrite element (BTYPE) sizes).


#
# The following should be set depending on the build environment
#

CC="cl"						# c compiler
DEF="/D"					# flag to define a cpp symbol
CONLY="/c"					# flag to compile but not link
CFLAGS="/TC /O2 /Yd /Zi /nologo /W3 /WX"	# c front end flags
LFLAGS="/O2 /Yd /Zi /nologo /Fe"		# linker flags
OBJ=.obj                                        # obj suffix (.o, .obj, etc)
EXE=.exe                                        # exe suffix ('', '.exe', etc)

#
# Support routines
#

docommand() { 
    echo [ $1 ]
    eval $1
}

compile() {
    f=$1
    otherflags=$2
    docommand "$CC $CFLAGS $CONLY $otherflags $f"
}

usage() {
    echo
    echo "Usage: $0: { [tpo|bpo <size>] <name> | [bog <size>|tog]"
    echo "         | tfb <size> <asm> <rules> }"
    echo
    echo "The argument <size> should be 'LONG' or 'BYTE'"
    echo
    echo "'tog' builds the text optimizer generator."
    echo "'bog' builds the binary optimizer generator for data of size <size>."
    echo "'tpo' builds a text peephole optimizer."
    echo "  $0 assumes that you have 2 files: <name>.tog should contain"
    echo "  the tog rules, and <name>.c should contain the tpo driver."
    echo "  There is also a default driver for tpo.  It is in tpot.c."
    echo "  If you do not have a file <name>.c, $0 will automatically"
    echo "  build a tpo project using tpot.c."
    echo "'bpo' builds a binary peephole optimizer."
    echo "  Again, $0 assumes that you have 2 files: <name>.bog should"
    echo "  contain the bog rules, and <name>.c the bpo driver."
    echo
    echo "The tfb rule is for convenience.  <asm> should be a set of"
    echo "assembler rules.  <rules> should correspond to a file"
    echo "<rules>.tfb, that specifies binary rewrite rules in assembly"
    echo "format.  $0 will first build 'tpo <asm>', and then use the"
    echo "resulting assembler to process the .tfb file and create a file"
    echo "<rules>.bog.  It will then build 'bpo <size> <rules>'."
    echo
}

bail() {
    echo "$1"
    exit 1
}

checkbtype() {
    case $1 in
	LONG)
	    BTYPE=LONG
	    ;;
	BYTE)
	    BTYPE=BYTE
	    ;;
	*) 
	    bail "Incorrect build command.  Must specify <size>."
	    ;;
    esac
}

checkfiles() {
    name=$1
    og=$2
    po=$3
    if [ x$name = x ]; then
	echo "Must specify <name>, the name of your $po project."
	echo "$0 assumes that you have 2 files: <name>.$og should contain"
	echo "the $og rules, and <name>.c should contain the $po driver."
	if [ $po = tpo ]; then
	    echo "There is also a default driver for tpo.  It is in tpot.c."
	    echo "If you do not have a file <name>.c, $0 will automatically"
	    echo "build a tpo project using tpot.c."
	fi
	exit 1
    fi
    if [ ! -f $name.c -a $po != tpo -o ! -f $name.$og ]; then
	echo "Could not find the required $po files."
	echo "$0 assumes that you have 2 files: <name>.$og should contain"
	echo "the $og rules, and <name>.c should contain the $po driver."
	exit 1
    fi
}

#
# Main (entry point)
#

if [ $# = 0 ]; then
    usage
    exit 1
fi

case $1 in
    bog)
	checkbtype $2
	echo "Building bog:"
	src="bog.c list.c mem.c string.c sym.c"
	    # name obj files manually; ms-dog environment may lack sed/perl
	obj="bog${OBJ} list$OBJ mem$OBJ string$OBJ sym$OBJ" 
	for p in $src; do
	    compile $p ${DEF}__${BTYPE}__
	done
	docommand "$CC ${LFLAGS}bog $obj"
	;;
    tog)
	echo "Building tog:"
	src="list.c mem.c string.c tog.c"
	obj="list$OBJ mem$OBJ string$OBJ tog$OBJ"
	for p in $src; do
	    compile $p ""
	done
	docommand "$CC ${LFLAGS}tog $obj"
	;;
    tpo)
	checkfiles "$2" tog tpo
	echo "Building tpo:"
	name=$2
	if [ -f ${name}.c ]; then
	    cname=$name
	else
	    cname="tpot"
	    echo "Could not find driver ${name}.c.  Using driver tpot.c."
	fi
	if [ ! -f tog$EXE ]; then
	    echo "Could not find tog$EXE: attempting to build it."
	    docommand "$0 tog"
	fi
	echo "Creating bog$EXE with BTYPE=LONG"
	docommand "$0 bog LONG"
	docommand "./tog -t ${name}-str.c -r ${name}-tmp.bog ${name}.tog"
	docommand "./bog ${name}-tmp.bog > ${name}-tab.c"
	src="${name}-str.c ${name}-tab.c bpo.c list.c mem.c string.c \
	    tpo.c ${cname}.c"
	obj="${name}-str${OBJ} ${name}-tab${OBJ} bpo${OBJ} list${OBJ} \
	    mem${OBJ} string${OBJ} tpo${OBJ} ${cname}${OBJ}"
	for p in $src; do
	    compile $p ${DEF}__LONG__
	done
	docommand "$CC ${LFLAGS}${name} $obj"
	;;
    bpo)
	checkbtype $2
	checkfiles "$3" bog bpo
	name=$3
	echo "Building bpo:"
	echo "Creating bog$EXE with BTYPE=$BTYPE"
	docommand "$0 bog $BTYPE"
	docommand "./bog ${name}.bog > ${name}-tab.c"
	src="${name}.c ${name}-tab.c bpo.c"
	obj="${name}${OBJ} ${name}-tab${OBJ} bpo${OBJ}"
	for p in $src; do
	    compile $p ${DEF}__${BTYPE}__
	done
	docommand "$CC ${LFLAGS}${name} $obj"
	;;
    tfb)
	checkbtype $2
	asm=$3
	rules=$4
	if [ x$asm = x -o x$rules = x ]; then
	    bail "Insufficient arguments to tfb option."
	fi
	docommand "$0 tpo $asm"
	docommand "./$asm -r ${rules}.tfb > ${rules}.bog"
	docommand "$0 bpo $BTYPE $rules"
	;;
    *)
	bail "Unknown build command."
	;;
esac

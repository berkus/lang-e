CC=gcc
VDIR=../../vcode-src
CFLAGS= -I- -I. -I$(VDIR) -Wall -g
# CFLAGS= -g -O2 -I- -I. -I$(VDIR) -Wall 
LIBS= $(VDIR)/vcode.lib.a  
# /usr/lib/libmld.a # include for mips
OBJS= 
SRCS= marshal.c

all: marshal

marshal: marshal.o $(LIBS) $(VDIR)/vcode.h
	$(CC) $(CFLAGS) -o marshal marshal.o $(LIBS) -lm
	marshal > tmp
	diff tmp marshal.out
	rm -f tmp

depend:
	makedepend $(CFLAGS) $(SRCS)

clean:
	rm -f *.o *.Addrs *.pixie *.Counts marshal

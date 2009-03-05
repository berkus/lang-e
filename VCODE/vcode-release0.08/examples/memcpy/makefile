CC=gcc
VDIR=../../vcode-src
CFLAGS= -I- -I. -I$(VDIR) -Wall -g
# CFLAGS= -g -O2 -I- -I. -I$(VDIR) -Wall 
LIBS= $(VDIR)/vcode.lib.a  
# /usr/lib/libmld.a # include for mips
OBJS= 
SRCS=memcpy.c blt.c

all: memcpy blt

memcpy: memcpy.o $(LIBS) $(VDIR)/vcode.h
	$(CC) $(CFLAGS) -o memcpy memcpy.o $(LIBS) -lm
	memcpy

blt: blt.o $(LIBS) $(VDIR)/vcode.h
	$(CC) $(CFLAGS) -o blt blt.o $(LIBS) -lm
	blt
depend:
	makedepend $(CFLAGS) $(SRCS)

clean:
	rm -f *.o *.Addrs *.pixie *.Counts memcpy blt

CC=gcc
VDIR=../../vcode-src
CFLAGS= -I- -I. -I$(VDIR) -Wall -g
# CFLAGS= -g -O2 -I- -I. -I$(VDIR) -Wall 
LIBS= $(VDIR)/vcode.lib.a  
# /usr/lib/libmld.a # include for mips
OBJS= 
SRCS=strcmp.c str.c

all: strcmp str

strcmp: strcmp.o $(LIBS) $(VDIR)/vcode.h
	$(CC) $(CFLAGS) -o strcmp strcmp.o $(LIBS) -lm
	# strcmp

str: str.o $(LIBS) $(VDIR)/vcode.h
	$(CC) $(CFLAGS) -o str str.o $(LIBS) -lm
	# str

depend:
	makedepend $(CFLAGS) $(SRCS)

clean:
	rm -f *.o *.Addrs *.pixie *.Counts strcmp str
# DO NOT DELETE

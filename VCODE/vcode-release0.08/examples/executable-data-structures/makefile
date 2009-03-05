CC=gcc
VDIR=../../vcode-src
CFLAGS= -I- -I. -I$(VDIR) -Wall -g
# CFLAGS= -g -O2 -I- -I. -I$(VDIR) -Wall 
LIBS= $(VDIR)/vcode.lib.a  
# /usr/lib/libmld.a # include for mips
OBJS= 
SRCS=ll.c binary.c

all: ll binary

ll: ll.o $(LIBS) $(VDIR)/vcode.h
	$(CC) $(CFLAGS) -o ll ll.o $(LIBS) -lm
	ll

binary: binary.o $(LIBS) $(VDIR)/vcode.h
	$(CC) $(CFLAGS) -o binary binary.o $(LIBS) -lm
	binary

depend:
	makedepend $(CFLAGS) $(SRCS)

clean:
	rm -f *.o *.Addrs *.pixie *.Counts ll binary
# DO NOT DELETE

ll.o: /usr/include/stdio.h /usr/include/malloc.h ../../vcode-src/vcode.h
ll.o: ../../vcode-src/vcode-config.h ../../vcode-src/vcode-macros.h
ll.o: ../../vcode-src/demand.h ../../vcode-src/binary.h /usr/include/assert.h
binary.o: /usr/include/assert.h ../../vcode-src/vcode.h
binary.o: ../../vcode-src/vcode-config.h ../../vcode-src/vcode-macros.h
binary.o: ../../vcode-src/demand.h /usr/include/stdio.h
binary.o: ../../vcode-src/binary.h

CC=gcc
VDIR=../../vcode-src
CFLAGS= -O2 -I- -I. -I$(VDIR) -Wall 
# CFLAGS= -g -O2 -I- -I. -I$(VDIR) -Wall 
LIBS= $(VDIR)/vcode.lib.a 
# /usr/lib/libmld.a # include for mips
OBJS= dcg.o mult.o

all: static dcg sdcg

dcg: $(OBJS) $(LIBS) $(VDIR)/vcode.h
	$(CC) $(CFLAGS) -o dcg $(OBJS) $(LIBS) -lm
	dcg -n 64 -v > dcg.out
	diff static.out dcg.out
	rm dcg.out

sdcg: mult.o sdcg.o $(LIBS) $(VDIR)/vcode.h
	$(CC) $(CFLAGS) -o sdcg sdcg.o mult.o $(LIBS) -lm
	sdcg -n 64 -v > sdcg.out
	diff static.out sdcg.out
	rm sdcg.out


static: static.c
	$(CC) $(CFLAGS) -o static static.c 

mult.h:	booth-gen
	booth-gen	4096	> mult.h

dcg.o: $(VDIR)/vcode.h
sdcg.o: $(VDIR)/vcode.h
mult.o: $(VDIR)/vcode.h mult.h

booth-gen: booth-gen.c
	$(CC) $(CFLAGS) -o booth-gen booth-gen.c

clean:
	rm -f *.o dcg static *.Addrs *.pixie *.Counts booth-gen
# DO NOT DELETE THIS LINE -- make depend depends on it.

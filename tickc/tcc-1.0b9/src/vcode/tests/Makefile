CC	=	ssgcc
VDIR	=	..
CFLAGS	=	-g -I- -I. -I$(VDIR) -Wall

ifeq ($(CC),ssgcc)
SSDIR	=	/home/butt/maxp/simplescalar/sslib
LIBS	=	$(VDIR)/vcode.lib.a $(SSDIR)/libopcodes.a $(SSDIR)/libbfd.a
else
LIBS	=	$(VDIR)/vcode.lib.a 
endif

all: call-test ctest jal-test reg-test regress stest

call-test: call-test.o $(VDIR)/vcode.lib.a call-gen
	$(CC) $(CFLAGS) -o call-test call-test.o $(LIBS)
	call-test > tmp.out
	diff call-test.out tmp.out
	rm tmp.out

call-test.c: call-gen
	perl -w call-gen > call-test.c


ctest: $(VDIR)/vcode.lib.a ctest.o 
	$(CC) $(CFLAGS) -o ctest ctest.o $(LIBS)
	ctest >  tmp.out
	diff tmp.out ctest.out
	rm -f tmp.out

ctest.o: $(VDIR)/vcode.h $(VDIR)/vcode-macros.h $(VDIR)/binary.h


jal-test: $(VDIR)/vcode.lib.a jal-test.o 
	$(CC) $(CFLAGS) -o jal-test jal-test.o $(LIBS)
	jal-test >  tmp.out
	diff tmp.out jal-test.out
	rm -f tmp.out

jal-test.o: $(VDIR)/vcode.h $(VDIR)/vcode-macros.h $(VDIR)/binary.h


reg-test: reg-test.o $(VDIR)/vcode.lib.a reg-test.o 
	$(CC) $(CFLAGS) -o reg-test reg-test.o $(LIBS)
	reg-test >  tmp.out
	diff tmp.out reg-test.out
	rm -f tmp.out


regress: regress.o $(VDIR)/vcode.lib.a test-gen 
	$(CC) $(CFLAGS) -o regress regress.o $(LIBS)
	regress > tmp.out
	diff regress.out tmp.out
	rm tmp.out

regress.c: test-gen
	perl -w test-gen > regress.c

regress.o: $(VDIR)/vcode.h $(VDIR)/vcode-macros.h $(VDIR)/binary.h


stest: $(VDIR)/vcode.lib.a stest.o 
	$(CC) $(CFLAGS) -o stest stest.o $(LIBS)
	stest >  tmp.out
	diff tmp.out stest.out
	rm -f tmp.out

stest.o: $(VDIR)/vcode.h $(VDIR)/vcode-macros.h $(VDIR)/binary.h

clean:
	rm -f *.o call-test.c regress.c \
	call-test ctest jal-test reg-test regress stest

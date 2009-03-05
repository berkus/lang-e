CC=gcc
VDIR=../vcode-src
# CFLAGS=  -O2 -DNDEBUG -D__NDEBUG__ -I- -I. -I$(VDIR) -Wall  
CFLAGS=  -g -I- -I. -I$(VDIR) -Wall  
LIBS= $(VDIR)/vcode.lib.a 
# /usr/lib/libmld.a  # include for mips

all: stest ctest reg-test jal-test call-test regress # call

regress: regress.o $(VDIR)/vcode.lib.a regress.o 
	$(CC) $(CFLAGS) -o regress regress.o $(LIBS) -lm
	regress > tmp.out
	diff regress.out tmp.out
	rm tmp.out

call-test: call-test.o $(VDIR)/vcode.lib.a call-gen
	$(CC) $(CFLAGS) -o call-test call-test.o $(LIBS) -lm
	call-test > tmp.out
	diff call-test.out tmp.out
	rm tmp.out

reg-test: reg-test.o $(VDIR)/vcode.lib.a reg-test.o 
	$(CC) $(CFLAGS) -o reg-test reg-test.o $(LIBS) -lm
	reg-test >  temp
	diff temp reg-test.out
	rm -f temp


regress.c: test-gen
	perl -w test-gen > regress.c
call-test.c: call-gen
	perl -w call-gen > call-test.c

vm: $(VDIR)/vcode.lib.a vm.o 
	$(CC) $(CFLAGS) -o vm vm.o $(LIBS) -lm

stest: $(VDIR)/vcode.lib.a stest.o 
	$(CC) $(CFLAGS) -o stest stest.o $(LIBS) -lm
	stest >  temp
	diff temp stest.out
	rm -f temp

jal-test: $(VDIR)/vcode.lib.a jal-test.o 
	$(CC) $(CFLAGS) -o jal-test jal-test.o $(LIBS) -lm
	jal-test >  temp
	diff temp jal-test.out
	rm -f temp

ctest: $(VDIR)/vcode.lib.a ctest.o 
	$(CC) $(CFLAGS) -o ctest ctest.o $(LIBS) -lm
	ctest >  temp
	diff temp ctest.out
	rm -f temp

call: $(VDIR)/vcode.lib.a call.o 
	$(CC) $(CFLAGS) -o call call.o $(LIBS) -lm

vcode.lib.a: $(VOBJS) vcode.h
	ar ruv vcode.lib.a $(VOBJS) 

vm.o: $(VDIR)/vcode.h $(VDIR)/vcode-macros.h
stest.o: $(VDIR)/vcode.h $(VDIR)/vcode-macros.h
ctest.o: $(VDIR)/vcode.h $(VDIR)/vcode-macros.h
regress.o: $(VDIR)/vcode.h $(VDIR)/vcode-macros.h
jal-test.o: $(VDIR)/vcode.h $(VDIR)/vcode-macros.h 

clean:
	rm -f *.o regress call-test vm reg-test call stest ctest jal-test regress.c

CC=gcc
VDIR=../../vcode-src
CFLAGS= -I- -I. -I$(VDIR) -Wall -g
# CFLAGS= -g -O2 -I- -I. -I$(VDIR) -Wall 
LIBS= $(VDIR)/vcode.lib.a  
# /usr/lib/libmld.a # include for mips
OBJS= 

all: hello zero inf-stream and pow jump branch curry hello2 hello-push

hello: hello.o $(LIBS) $(VDIR)/vcode.h
	$(CC) $(CFLAGS) -o hello hello.o $(LIBS) -lm
	hello > tmp
	diff hello.out tmp
	rm -f tmp

hello-push: hello-push.o $(LIBS) $(VDIR)/vcode.h
	$(CC) $(CFLAGS) -o hello-push hello-push.o $(LIBS) -lm
	hello-push > tmp
	diff hello-push.out tmp
	rm -f tmp

zero: zero.o $(LIBS) $(VDIR)/vcode.h
	$(CC) $(CFLAGS) -o zero zero.o $(LIBS) -lm
	zero > tmp
	diff zero.out tmp
	rm -f tmp

inf-stream: inf-mkstream.c inf-driver.c inf-stream.o $(LIBS) $(VDIR)/vcode.h
	$(CC) $(CFLAGS) -o inf-stream inf-stream.o $(LIBS) -lm
	inf-stream > tmp
	diff inf-stream.out tmp
	rm -f tmp

and: and-driver.c and.c and-driver.o $(LIBS) $(VDIR)/vcode.h
	$(CC) $(CFLAGS) -o and and-driver.o $(LIBS) -lm
	and > tmp
	diff and.out tmp
	rm -f tmp

pow: power-driver.c power.c pow.o $(LIBS) $(VDIR)/vcode.h
	$(CC) $(CFLAGS) -o pow pow.o $(LIBS) -lm
	pow > tmp
	diff pow.out tmp
	rm -f tmp

jump: jump-and-link.o $(LIBS) $(VDIR)/vcode.h
	$(CC) $(CFLAGS) -o jump jump-and-link.o $(LIBS) -lm
	jump > tmp
	diff jump.out tmp
	rm -f tmp

branch: branch.o $(LIBS) $(VDIR)/vcode.h
	$(CC) $(CFLAGS) -o branch branch.o $(LIBS) -lm
	branch > tmp
	diff branch.out tmp
	rm -f tmp

curry: curry.o $(LIBS) $(VDIR)/vcode.h
	$(CC) $(CFLAGS) -o curry curry.o $(LIBS) -lm
	curry > tmp
	diff curry.out tmp
	rm -f tmp

hello2: hello2-driver.o $(LIBS) $(VDIR)/vcode.h
	$(CC) $(CFLAGS) -o hello2 hello2-driver.o $(LIBS) -lm
	hello2 > tmp
	diff hello2.out tmp
	rm -f tmp

clean:
	rm -f *.o *.Addrs *.pixie *.Counts inf-stream zero hello and pow jump branch curry hello2 hello-push

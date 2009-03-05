CC=gcc
VDIR=../../vcode-src
CFLAGS=  -g -I- -I. -I$(VDIR) 
LIBS= $(VDIR)/vcode.lib.a 

all: calc dcg_calc

calc: lex.yy.o calc.tab.o $(LIBS) $(VDIR)/vcode.h
	$(CC) $(CFLAGS) -o calc lex.yy.o calc.tab.o $(LIBS) -lm

dcg_calc: lex.yy.o dcg_calc.tab.o $(LIBS) $(VDIR)/vcode.h
	$(CC) $(CFLAGS) -o dcg_calc lex.yy.o dcg_calc.tab.o $(LIBS) -lm
	dcg_calc < test > tmp
	diff tmp calc.out
	rm -f tmp

lex.yy.o: 
calc.tab.o: 
dcg_calc.tab.o: 

lex.yy.c: calc.l
	flex -I calc.l

dcg_calc.tab.c: dcg_calc.y
	bison -d dcg_calc.y

calc.tab.c: calc.y
	bison -d calc.y

clean:
	rm -f *.o calc static *.Addrs *.pixie *.Counts dcg_calc

CC=gcc
CFLAGS= -g
LIBS= #-ls5fn
OBJS= bin.tab.o lex.yy.o

all: bpp

bpp: $(OBJS)
	$(CC) $(CFLAGS) -o bpp $(OBJS) $(LIBS)  -lfl

lex.yy.o: 
bin.tab.o: 

lex.yy.c: bin.l
	flex bin.l

bin.tab.c: bin.y
	bison -d bin.y

clean:
	rm -f *.o bpp 

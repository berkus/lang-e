CC=gcc
YACC=bison -d
LEX=flex
LEXLIB=-L/usr/uns/lib -lfl
CFLAGS= -g
OBJS= bin.tab.o lex.yy.o

all: bpp

bpp: $(OBJS)
	$(CC) $(CFLAGS) -o bpp $(OBJS) $(LIBS) $(LEXLIB)

lex.yy.c: bin.l
	$(LEX) bin.l

bin.tab.c: bin.y
	$(YACC) bin.y

clean:
	rm -f *.o bpp 

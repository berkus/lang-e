CC	=	gcc
CFLAGS	=	-I/ac3/tickc/xtcc/include/sparc/solaris/tickc -g -O0
LDFLAGS	=	-L/ac3/tickc/xtcc/lib -licode -lvcode -ltcsup -lopcodes -lbfd

tst: tst.o xtst.o
	$(CC) -o $@ $^ $(LDFLAGS)

xtst.c: tst.ops
	perl /ac3/tickc/xtcc/lib/xlate.pl -s $< $@

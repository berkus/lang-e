CFLAGS = $(DEFS) -I. -ggdb

vcode.lib.a: vcode.o dis.o
	ar -r vcode.lib.a vcode.o dis.o
	ranlib vcode.lib.a

vcode.o: vcode.h x86-codegen.h vcode.c dis.o
	gcc -c $(CFLAGS) -O2 vcode.c
	ld -r -o out.o vcode.o dis.o
	mv out.o vcode.o

regres.o: regres.c vcode.h x86-codegen.h
	gcc -c $(CFLAGS) regres.c

regres: regres.o vcode.lib.a vcode.h x86-codegen.h
	gcc -o regres $(CFLAGS) regres.o vcode.lib.a

test: regres
	echo "Running regression tests. This should print 'No errors'"
	./regres

time-test: time.c vcode.h x86-codegen.h vcode.lib.a
	gcc -o time-test $(CFLAGS) -O2 time.c vcode.lib.a

time: time-test
	echo Check time.c for count of insns generated in this amount of time.
	time ./time-test

dis.o: dis.c
	gcc -c $(CFLAGS) -O2 dis.c

clean:
	rm -f *.o *.a regres time-test core *~

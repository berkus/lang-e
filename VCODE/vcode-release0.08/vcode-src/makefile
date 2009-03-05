CC=gcc
# LCCFLAGS= -g -v -pg -I/home/am2/maxp/tickc/include/sparc/sun
GCCFLAGS= -O2 -fpcc-struct-return -ansi -pedantic -Wall 
CFLAGS= $(GCCFLAGS)
LIBS=
ARCH=sparc
TST=tst
HDRS=vcode.h vcode-internal.h vcode-macros.h
# VOBJS= link.o $(ARCH)-gen.o gen.o flushcache.o
# VSRC= link.c $(ARCH)-gen.c gen.c
VOBJS= flushcache.o link.o $(ARCH)-gen.o gen.o register.o xlate.o call.o poly-insns.o fp-imm.o sparc-pseudo.o sparc-dis.o portable-insts.o
VSRC= link.c $(ARCH)-gen.c gen.c register.c xlate.c call.c poly-insns.c fp-imm.c sparc-pseudo.S sparc-dis.c portable-insts.c


DISDIR=sparc-dis

all: binary.h vcode-macros.h vcode.lib.a

vcode.lib.a: $(VOBJS) vcode.h
	ar ruv vcode.lib.a $(VOBJS) $(DISDIR)/*.o
	ranlib vcode.lib.a

binary.h: $(ARCH)-bin
	bin/bpp/bpp < $(ARCH)-bin > binary.h

sparc-pseudo.o: sparc-pseudo.S
	gcc -c sparc-pseudo.S

vcode-macros.h: binary.h $(ARCH).md spec.pl
	perl spec.pl < $(ARCH).md > vcode-macros.h

flushcache.o: flushcache.c vcode.h vcode-config.h vcode-macros.h demand.h
	gcc -O -c $<

$(ARCH)-gen.c: vcode-internal.h 
gen.c: vcode-internal.h 

depend:
	makedepend $(VSRC)

clean:
	rm -f *.o dump.f core vcode.lib.a *.Counts *.pixie *.Addrs vcode-macros.h binary.h

cleanx:
	rm -f vcode-config.h makefile
# DO NOT DELETE

link.o: vcode-internal.h vcode.h vcode-config.h /usr/include/stdio.h
link.o: vcode-macros.h demand.h binary.h /usr/include/assert.h
link.o: vcode-portable-insts.h
sparc-gen.o: /usr/include/assert.h /usr/include/stdio.h /usr/include/memory.h
sparc-gen.o: vcode-internal.h vcode.h vcode-config.h vcode-macros.h demand.h
sparc-gen.o: binary.h vcode-portable-insts.h
gen.o: /usr/include/assert.h /usr/include/stdio.h vcode-internal.h vcode.h
gen.o: vcode-config.h vcode-macros.h demand.h binary.h vcode-portable-insts.h
register.o: vcode-internal.h vcode.h vcode-config.h /usr/include/stdio.h
register.o: vcode-macros.h demand.h binary.h /usr/include/assert.h
register.o: vcode-portable-insts.h /usr/include/string.h
register.o: /usr/include/sys/stdtypes.h
xlate.o: vcode-internal.h vcode.h vcode-config.h /usr/include/stdio.h
xlate.o: vcode-macros.h demand.h binary.h /usr/include/assert.h
xlate.o: vcode-portable-insts.h
call.o: /usr/include/stdio.h vcode-internal.h vcode.h vcode-config.h
call.o: vcode-macros.h demand.h binary.h /usr/include/assert.h
call.o: vcode-portable-insts.h
poly-insns.o: vcode.h vcode-config.h /usr/include/stdio.h vcode-macros.h
poly-insns.o: demand.h binary.h /usr/include/assert.h vcode-portable-insts.h
fp-imm.o: vcode-internal.h vcode.h vcode-config.h /usr/include/stdio.h
fp-imm.o: vcode-macros.h demand.h binary.h /usr/include/assert.h
fp-imm.o: vcode-portable-insts.h
sparc-dis.o: vcode-internal.h vcode.h vcode-config.h /usr/include/stdio.h
sparc-dis.o: vcode-macros.h demand.h binary.h /usr/include/assert.h
sparc-dis.o: vcode-portable-insts.h
portable-insts.o: vcode-internal.h vcode.h vcode-config.h
portable-insts.o: /usr/include/stdio.h vcode-macros.h demand.h binary.h
portable-insts.o: /usr/include/assert.h vcode-portable-insts.h

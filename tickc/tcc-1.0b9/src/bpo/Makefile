#
# This makefile is intended to work with GNU make.
#

.EXPORT_ALL_VARIABLES:
.PHONY: clean clean-all
.PRECIOUS: %.c %.o

RM	=	rm -f
CC	=	gcc

#CFLAGS0	=	-Wall -O3 -DNDEBUG -ggdb
CFLAGS0	=	-Wall -O0 -ggdb
BTYPE	=	-D__BYTE__
CFLAGS	=	$(CFLAGS0) $(BTYPE)

# Suffixes:
#	.tog	patterns for tog; text patterns to specify text-to-text opts
#	.bog	patterns for bog; bit patterns to specify binary opts
#	.t2b	patterns for tog; text to binary ("assembler") translation
#	.tfb	data for tpo; "text-format-binary", ie. asm-like specs of
#		 binary opts; must be used together with .t2b to create a .bog

# Optimizers

%-tfb: %.t2b %.tfb bog tog
	./tog -t $*-str1.c -r $*.tmp1 $*.t2b
	./bog $*.tmp1 > $*-tab1.c
	$(MAKE) $(addsuffix .o, $*-str1 $*-tab1 bpo tpo)
	$(CC) -o $*-gen $(addsuffix .o, $*-str1 $*-tab1 \
		bpo list mem string tpo)
	./$*-gen $*.tfb > $*.tmp2
	rm bog bog.obj; $(MAKE) BTYPE=-D__BYTE__ bog	
	./bog $*.tmp2 > $*-tab2.c
	$(CC) $(CFLAGS) BTYPE=-D__BYTE__ bpo.c $*.c $*-tab2.c
	$(CC) -o $*-gen $(addsuffix .o, bpo $* $*-tab2)

%-bpolib: % %.tfb bog
	./$* -r $*.tfb > $*.bog
	./bog $*.bog > bpo-$*.c
	$(CC) $(CFLAGS) -c bpo-$*.c

%-bpo: %.bog
	$(MAKE) bog
	./bog $< > $*-tab.c
	$(MAKE) $(addsuffix .o, $* $*-tab bpo)
	$(CC) -o $* $(addsuffix .o, $* $*-tab bpo)

%-tpo: %.tog tog bog
	./tog -t $*-str.c -r $*.tmp $<
	./bog $*.tmp > $*-tab.c
	$(MAKE) BTYPE=-D__LONG__ \
		$(addsuffix .o, tpot $*-str $*-tab bpo list mem string tpo)
	$(CC) -o $* \
		$(addsuffix .o, tpot $*-str $*-tab bpo list mem string tpo)

# Optimizer Generators

bpo.o: btype.h bpo.c bpo.h
bog.o: btype.h bog.c list.h mem.h string.h sym.h
bog2.o: btype.h bog1.c bog2.c cwmisc.c cwmisc.h list.h mem.h set.h string.h sym.h

bog: $(addsuffix .o, bog list mem string sym)
	$(CC) -o $@ $^

bog2: $(addsuffix .o, bog1 bog2 cwmisc list mem set string sym)
	$(CC) -o $@ $^

tog: $(addsuffix .o, list mem string tog)
	$(CC) -o $@ $^

# Other

%: %.c
	$(CC) $(CFLAGS) -o $@ $<

%.o: %.c
	$(CC) $(CFLAGS) -c $<

%.s: %.c
	$(CC) $(CFLAGS) -c -S $<

clean:
	$(RM) *.obj *.o *.tmp *.tmp1 *.tmp2 *~

clean-all: clean
	$(RM) bog tog *-str.c *-tab.c *.pdb *.ilk

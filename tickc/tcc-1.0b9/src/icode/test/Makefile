CFLAGS	=	-ggdb -I/home/tickc/xtcc/include/sparc/sun/tickc
LDFLAGS	=	-L/home/tickc/xtcc/lib/sparc-sun-sunos4.1.3_U1 -licode -lvcode

%: %.c
	gcc $(CFLAGS) $< -o $@ $(LDFLAGS)

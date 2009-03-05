CC	=	cl
CFLAGS	=	/c /I. /TC
CPPFLAGS=	/c /I. /Imsdis/inc /GX /TP

%: %.obj dis.obj
	$(CC) $^ /link /debug msdis/lib/x86/msdisd.lib

%.obj: %.cpp
	$(CC) $(CPPFLAGS) $<

%.obj: %.c
	$(CC) $(CFLAGS) $<

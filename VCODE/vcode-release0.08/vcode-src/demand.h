#ifndef __DEMAND__
#define __DEMAND__

#ifdef __NDEBUG__
#       define demand(bool, msg) (void)0
#else
#	include <stdio.h>
#       define demand(bool, msg) \
            do { if(!(bool)) __demand(#bool, #msg, __FILE__, __LINE__); } while(0)

#	if defined(SPARC) && defined(__GNUC__) && !defined(_V_SOLARIS_)
		extern int printf(char *, ...);
#	endif

#	define __demand(_bool, _msg, _file, line) do {				\
        	printf("file %s, line %d: %s\n",_file,line,_bool);		\
        	printf("%s\n",_msg);						\
        	exit(1);							\
	} while(0)

#endif /* __NDEBUG__ */
#endif /* __DEMAND__ */

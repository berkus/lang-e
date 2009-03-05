#include <string.h>

char *cpp[] = { _TICKC_CPP, "-undef", "-nostdinc",
		"-Di386", "-Dlinux", "-Dunix",
		"-D__i386__", "-D__linux__", "-D__unix__", 
		"-D__i386", "-D__linux", "-D__unix",
		"-D__TCC__", "-D__ELF__",
		"$1", "$2", "$3", 0 };

char *include[] = { "-I"_TICKC_INC"/tickc", 
		    "-I"_TICKC_INC, 
		    "-I/usr/include", 0 };

char *com[] = { _TICKC_COM, "-target=i386-linux", "$1", "$2", "$3", 0 };

char *as[] = { _TICKC_AS, "-o", "$3", "$1", "$2", 0 };

char *ld[] = { _TICKC_LD, "-o", "$3",
	       "$1", 
	       "/usr/lib/crt1.o", "/usr/lib/crti.o", "/usr/lib/crtbegin.o",
	       "-L"_TICKC_LIB, "-L"_TICKC_GLP, 
	       "$2", "", /* 10 */
	       "-ltickc-rts", "-licode", "-lvcode", "", /* 14 */
	       "-lgcc", "-lc", /* 16 */ "-lgcc",
	       "/usr/lib/crtend.o", "/usr/lib/crtn.o", "", /* 20 */
	       0};

char *cc2[] = { _TICKC_CC2, "-g", "-O2", "-fno-builtin", "-mv8",
		"-D__TCC__", "-D__CC2__",
		"-I"_TICKC_INC"/tickc",
		"-I"_TICKC_INC,
		"-include", _TICKC_INC"/tickc/tickc-env.h",
		"-include", _TICKC_GLP"/include/stdarg.h",
		"-imacros", _TICKC_INC"/tickc/tickc-macros.h",
		"-S", "$1", "$2", "-o", "$3", 0 };

int option(arg) char *arg; {
     if (strcmp(arg, "-g") == 0)
	  ld[20] = "-lg";
     else if (strcmp(arg, "-p") == 0) {
	  ld[4] = "/usr/lib/gcrt1.o";
	  ld[14] = "-lgmon";
	  ld[16] = "-lc_p";
     } else if (strcmp(arg, "-b") == 0 
		&& access(_TICKC_BBX, 4) == 0)
	  ld[10] = _TICKC_BBX;
     else
	  return 0;
     return 1;
}

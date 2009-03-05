#include <string.h>

char *cpp[] = { _TICKC_CPP,
		"-DLANGUAGE_C", "-D__LANGUAGE_C",  "-D__LANGUAGE_C__", 
		"-Dunix", "-D__unix", "-D__unix__", 
		"-Dultrix", "-D__ultrix", "-D__ultrix__", 
		"-Dbsd4_2", "-D__bsd4_2", "-D__bsd4_2__", 
		"-Dmips=1", "-D__mips=1", "-D__mips__=1", 
		"-Dhost_mips", "-D__host_mips", "-D__host_mips__",
		"-DMIPSEL", "-D__MIPSEL", "-D__MIPSEL__",
		"-D__TCC__",
		"$1", "$2", "$3", 0 };

char *include[] = { "-I"_TICKC_INC"/tickc", 
		    "-I"_TICKC_INC, 
		    "-I/usr/include", 0 };

char *com[] = { _TICKC_COM, "-target=mips-ultrix", "$1", "$2", "$3", 0 };

char *as[] =  { _TICKC_AS, "-o", "$3", "", "$1", "-nocpp", "-EL", "$2", 0 };

char *ld[] = { _TICKC_LD, "-o", "$3", "/usr/lib/crt0.o", "$1", 
	       "-L"_TICKC_LIB, "-L"_TICKC_GLP, "$2",
	       "-ltickc-rts", "-licode", "-lvcode", /* 10 */
	       "-ltcsup",
	       "-lc", "-lgcc", "-lc", "", /* 15 is for -lg */
	       "-lgcc", 0};

char *cc2[] = { _TICKC_CC2, "-g", "-w", "-O2", "-fno-builtin",
		"-D__TCC__", "-D__CC2__",
		"-I"_TICKC_INC"/tickc",
		"-I"_TICKC_INC,
		"-include", _TICKC_INC"/tickc/tickc-env.h",
		"-include", _TICKC_GLP"/include/stdarg.h",
		"-imacros", _TICKC_INC"/tickc/tickc-macros.h",
		"-S", "$1", "$2", "-o", "$3", 0 };

int option(arg) char *arg; {
     if (strcmp(arg, "-g") == 0)
	  ld[15] = "-lg";
     else if (strcmp(arg, "-p") == 0) {
	  ld[3]  = "/usr/lib/mcrt0.o";
	  ld[11] = ld[13] = "-lc_p";
     } else if (strcmp(arg, "-pg") == 0) {
	  ld[3]  = "/usr/lib/gcrt0.o";
	  ld[11] = ld[13] = "-lc_p";
     } else if (strcmp(arg, "-b") == 0
		&& access(_TICKC_BBX, 4) == 0)
	  ld[13] = _TICKC_BBX;
     else
	  return 0;
     return 1;
}

#include <string.h>

char *cpp[] = { _TICKC_CPP, "-undef", "-nostdinc",
		"-Dsun", "-Dsparc", "-Dunix", 
		"-D__sparc__", "-D__sun__", "-D__unix__", 
		"-D__sparc", "-D__sun", "-D__unix",
		"-D__TCC__", 
		"$1", "$2", "$3", 0 };

char *include[] = { "-I"_TICKC_INC"/tickc", 
		    "-I"_TICKC_INC, 
		    "-I/usr/include", 0 };

char *com[] = { _TICKC_COM, "-target=sparc-solaris", "$1", "$2", "$3", 0 };

char *as[] = { _TICKC_AS, "-Qy", "-s", "-o", "$3", "$1", "$2", 0 };

char *ld[] = { _TICKC_LD,
	       "-Qy",
	       "-o", "$3", 
	       _TICKC_GLP"/crt1.o",
	       _TICKC_GLP"/crti.o",
	       "/usr/ccs/lib/values-Xa.o",
	       _TICKC_GLP"/crtbegin.o",
	       "$1", "", /* 11 */
	       "-L"_TICKC_LIB, "-L"_TICKC_GLP, "$2",
	       "-L/usr/ccs/lib", "-L/usr/lib",
	       "-ltickc-rts", "-licode", "-lvcode", "-ltcsup",
	       "-lopcodes", "-lbfd",
	       "-lc", "-lgcc", "-lc", "-lgcc", "", /* 27 */
	       _TICKC_GLP"/crtend.o",
	       _TICKC_GLP"/crtn.o",
	       0};

char *cc2[] = { _TICKC_CC2, "-g", "-w", "-O2", "-fno-builtin", "-mv8",
		"-D__TCC__", "-D__CC2__",
		"-I"_TICKC_INC"/tickc",
		"-I"_TICKC_INC,
		"-include", _TICKC_INC"/tickc/tickc-env.h",
		"-include", _TICKC_GLP"/include/stdarg.h",
		"-imacros", _TICKC_INC"/tickc/tickc-macros.h",
		"-S", "$1", "$2", "-o", "$3", 0 };

int option(arg) char *arg; {
     if (strcmp(arg, "-g") == 0)
	  ld[27] = "-lg";
     else if (strcmp(arg, "-p") == 0) {
#if 0
	  ld[9]  = "/usr/lib/mcrt0.o";
	  ld[16] = ld[18] = "-lc_p";
#endif
	  return 0;
     } else if (strcmp(arg, "-pg") == 0) {
#if 0
	  ld[9]  = "/usr/lib/gcrt0.o";
	  ld[16] = ld[18] = "-lc_p";
#endif
	  return 0;
     } else if (strcmp(arg, "-b") == 0 
		&& access(_TICKC_BBX, 4) == 0)
	  ld[11] = _TICKC_BBX;
     else
	  return 0;
     return 1;
}

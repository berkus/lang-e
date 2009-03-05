char *cpp[] = { _TICKC_CPP, "-undef", "-nostdinc",
		"-D__STRICT_ANSI__",
		"-Dmips", "-Dultrix", "-Dunix", 
		"-DSSEL", "-DMIPSEL", "-Dsimplescalar",
		"-DSIMPLE_SCALAR",
		"-D__mips", "-D__ultrix", "-D__unix", 
		"-D__SSEL", "-D__MIPSEL", "-D__simplescalar",
		"-D__SIMPLE_SCALAR",
		"-D__mips__", "-D__ultrix__", "-D__unix__",
		"-D__SSEL__", "-D__MIPSEL__", "-D__simplescalar__",
		"-D__SIMPLE_SCALAR__", "-D__sslittle256__",
		"-D__TCC__", 
		"$1", "$2", "$3", 0 };

char *include[] = { "-I"_TICKC_INC"/tickc", 
		    "-I"_TICKC_INC, 
		    "-I"_SSHOME"/sslittle256-na-sstrix/include",
		    "-I"_TICKC_GLP"/include",
		    0 };

char *com[] = { _TICKC_COM, "-target=sslittle-sstrix", "$1", "$2", "$3", 0 };

char *as[] = { _TICKC_AS, "-nocpp", "-EL", "-o", "$3", "$1", "$2", 0 };

char *ld[] = { _TICKC_LD,
	       "-EL", "-o", "$3",
	       _SSHOME"/sslittle256-na-sstrix/lib/crt0.o",
	       "-L"_TICKC_GLP,
	       "-L"_SSHOME"/sslittle256-na-sstrix/lib",
	       "-L"_TICKC_LIB,
	       "$1", "$2",
	       "-ltickc-rts", "-licode", "-lvcode", "-ltcsup",
	       "-lopcodes", "-lbfd",
	       "-lgcc", "-lc", "-lgcc",
	       0};

char *cc2[] = { _TICKC_CC2, "-g", "-w", "-O2", "-fno-builtin",
		"-D__sslittle256__",
		"-D__TCC__", "-D__CC2__",
		"-I"_TICKC_INC"/tickc",
		"-I"_TICKC_INC,
		"-include", _TICKC_INC"/tickc/tickc-env.h",
		"-include", _TICKC_GLP"/include/stdarg.h",
		"-imacros", _TICKC_INC"/tickc/tickc-macros.h",
		"-S", "$1", "$2", "-o", "$3", 0 };

int option(arg) char *arg; {
     return 0;
}

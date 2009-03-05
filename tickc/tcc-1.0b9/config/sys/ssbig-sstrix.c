char *cpp[] = { _TICKC_CPP, "-undef", "-nostdinc",
		"-D__STRICT_ANSI__",
		"-Dmips", "-Dirix", "-Dunix", 
		"-DSSEB", "-DMIPSEB", "-Dsimplescalar",
		"-DSIMPLE_SCALAR",
		"-D__mips", "-D__irix", "-D__unix", 
		"-D__SSEB", "-D__MIPSEB", "-D__simplescalar",
		"-D__SIMPLE_SCALAR",
		"-D__mips__", "-D__irix__", "-D__unix__",
		"-D__SSEB__", "-D__MIPSEB__", "-D__simplescalar__",
		"-D__SIMPLE_SCALAR__",
		"-D__TCC__", 
		"$1", "$2", "$3", 0 };

char *include[] = { "-I"_TICKC_INC"/tickc", 
		    "-I"_TICKC_INC, 
		    "-I"_SSHOME"/ssbig-na-sstrix/include",
		    "-I"_TICKC_GLP"/include",
		    0 };

char *com[] = { _TICKC_COM, "-target=ssbig-sstrix", "$1", "$2", "$3", 0 };

char *as[] = { _TICKC_AS, "-nocpp", "-EB", "-o", "$3", "$1", "$2", 0 };

char *ld[] = { _TICKC_LD,
	       "-EB", "-o", "$3",
	       _SSHOME"/ssbig-na-sstrix/lib/crt0.o",
	       "-L"_TICKC_GLP,
	       "-L"_SSHOME"/ssbig-na-sstrix/lib",
	       "-L"_TICKC_LIB,
	       "$1", "$2",
	       "-ltickc-rts", "-licode", "-lvcode", "-ltcsup",
	       "-lopcodes", "-lbfd",
	       "-lgcc", "-lc", "-lgcc",
	       0};

char *cc2[] = { _TICKC_CC2, "-g", "-w", "-O2", "-fno-builtin",
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

/* linux.c -- lcc driver definitions for Linux.
 *
 * original version from Thorsten.Ohl@Physik.TH-Darmstadt.de
 */

#include <string.h>
char *cpp[] = {"/lib/cpp",
	"-undef", "-nostdinc", "-lang-c", "-U__GNUC__",
	"-D_POSIX_SOURCE", "-D__STDC__", "-D__STRICT_ANSI__",
	"-Dunix", "-Di386", "-Dlinux",
	"-D__unix__", "-D__i386__", "-D__linux__",
	"$1", "$2", "$3", 0};

char *include[] = {"-I/usr/local/include/ansi", "-I/usr/include", 0};

char *com[] = {"/usr/local/lib/rcc", "-target=x86-linux",
	"$1", "$2", "$3", 0};

char *as[] = {"/usr/bin/as", "-o", "$3", "$1", "$2", 0};

char *ld[] = {"/usr/bin/ld", "-d", "-o", "$3",
	"/usr/lib/crt0.o", "$1", "$2", "",
	"-lm", "", "-lc", "", "",
/*	"-L/usr/lib/gcc-lib/i486-linux/2.6.2", "-lgcc", */
	0};

int 
option(arg)
	char *arg;
{
	if (strcmp(arg, "-g") == 0)
		ld[12] = "-lg";
	else if (strcmp(arg, "-p") == 0) {
		ld[4] = "/usr/lib/gcrt0.o";
		ld[11] = "-lgmon";
		ld[12] = "-lc_p";
	} else if (strcmp(arg, "-b") == 0 && access("/usr/local/lib/bbexit.o", 4) == 0)
		ld[7] = "/usr/local/lib/bbexit.o";
	else
		return 0;
	return 1;
}

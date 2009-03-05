#include "c.h"

static char rcsid[] = "$Id: main.c,v 1.3 1998/07/09 22:43:20 maxp Exp $";

static void compilestring ARGS((char *));
static int doargs ARGS((int, char **));
static void emitYYnull ARGS((void));
static void typestab ARGS((Symbol, void *));

static void closureInit ARGS((void));
static void envInit ARGS((int, char **));
extern int system ARGS((char *));

static char envfile[] = _TICKC_INC"/tickc/tickc-env.h";
static char templatermcmd[] = "rm %s";
static char templatemvcmd[] = "mv %s %s";
static char templatecatcmd[] = "cat ";

static char templatecppcmd[] = _TICKC_CPP" -undef -D"_TARGET
  " -I"_TICKC_INC"/tickc -imacros "_TICKC_INC"/tickc/tickc-macros.h %s > %s";

Interface *IR = NULL;
Interface *sIR = NULL;
Interface *dIR = NULL;

static char *infile, *outfile, *dcgfile;

int Aflag;		/* >= 0 if -A specified */
int Pflag;		/* != 0 if -P specified */
int Xflag;		/* != 0 if -X specified to permit extensions */
int glevel;		/* == [0-9] if -g[0-9] specified */
int xref;		/* != 0 for cross-reference data */
Symbol YYnull;		/* _YYnull  symbol if -n or -nvalidate specified */
Symbol YYcheck;		/* _YYcheck symbol if -nvalidate,check specified */

static int verbose;

struct tcnamelist tcname;

static char hhhfile[80], cccfile[80];
struct cbestate cbe = { 0, 0, 1};

int main(argc, argv) int argc; char *argv[]; {
     int i, j;
     for (i = argc - 1; i > 0; i--)
	  if (strncmp(argv[i], "-target=", 8) == 0)
	       break;
     if (i > 0) {
	  for (j = 0; bindings[j].name; j++)
	       if (strcmp(&argv[i][8], bindings[j].name) == 0)
		    break;
	  if (bindings[j].ir) {
	       binding = &bindings[j];
	       sIR = bindings[j].ir;
	       dIR = bindings[j].Iir;
	       IR = sIR;
	       cbe.have = (sIR == &cIR);
	  } else {
	       fprint(2, "%s: unknown target `%s'\n", argv[0], &argv[i][8]);
	       exit(1);
	  }
     }
     if (!IR) {
	  int i;
	  fprint(2, "%s: must specify one of\n", argv[0]);
	  for (i = 0; bindings[i].name; i++)
	       fprint(2, "\t-target=%s\n", bindings[i].name);
	  exit(1);
     }
     typeInit();
     closureInit();
     argc = doargs(argc, argv);
     if (outfile && strcmp(outfile, "-") != 0)
	  if ((outfd = creat(outfile, 0666)) < 0) {
	       fprint(2, "%s: can't write `%s'\n", argv[0], outfile);
	       exit(1);
	  }
				/* Initialize the itable */
     itabinit();
				/* `C: open the dcg C file descriptor */
     if (dcgfile && strcmp(dcgfile, "-") != 0)
	  if ((dcgfd = creat(dcgfile, 0666)) < 0) {
	       fprint(2, "%s: can't write `%s'\n", argv[0], dcgfile);
	       exit(1);
	  }
     if (cbe.have) {
	  sprintf(hhhfile, "%s.h", dcgfile);
	  sprintf(cccfile, "%s.c", dcgfile);
	  if ((hhhfd = creat(hhhfile, 0666)) < 0) {
	       fprint(2, "%s: can't write `%s'\n", argv[0], hhhfile);
	       exit(1);
	  }
	  if ((cccfd = creat(cccfile, 0666)) < 0) {
	       fprint(2, "%s: can't write `%s'\n", argv[0], cccfile);
	       exit(1);
	  }
     }
     envInit(argc,argv);
     if (infile && strcmp(infile, "-") != 0)
	  if ((infd = open(infile, 0)) < 0) {
	       fprint(2, "%s: can't read `%s'\n", argv[0], infile);
	       exit(1);
	  }
     inputInit();
     if (cbe.have)
	  cbe.emit = 1;
     t = gettok();
     if (glevel && IR->stabinit)
	  (*IR->stabinit)(firstfile, argc, argv);
     program();
     if (!eval.fc)		/* Unparse the itable */
	  itabdump();
     close(infd);
     close(dcgfd);
     if (eval.tExists && !cbe.have) {
	  if (errcnt)
	       fprint(2, "Not compiling CGFs because of errors.\n");
	  else {
	       eval.tExists = 0;
	       eval.tCompiling = 1;
	       if (dflag) {
		    fprint(2, "Emitted CGFs.\n");
	       } else {
		    char *tmpfile = stringf("%s.tmp", dcgfile);

		    if (verbose)
			 fprint(2, "Preprocessing CGFs.\n");
		    system(stringf(templatecppcmd, dcgfile, tmpfile));

		    if ((infd = open(tmpfile, 0)) < 0) {
			 fprint(2, "%s: can't read `%s'\n", argv[0], tmpfile);
			 system(stringf(templatermcmd, tmpfile));
			 exit(1);
		    }
		    inputInit();
		    t = gettok();
		    if (verbose)
			 fprint(2, "Compiling CGFs.  ");
		    program();
		    eval.tCompiling = 0;
		    close(infd);
		    system(stringf(templatermcmd, tmpfile));
		    if (verbose)
			 fprint(2, "Done.\n");
	       }
	  }
     } else if (cbe.have) {
	  char *tmpfile = stringf("%s.tmp", dcgfile);
	  if (verbose)
	       fprint(2, "Finalizing C output.  ");

	  outflushfdx(cccfdIdx); close(cccfd);
	  outflushfdx(hhhfdIdx); close(hhhfd);

	  system(stringf("%s %s %s %s > %s", templatecatcmd,
			 hhhfile, cccfile, dcgfile, tmpfile));
	  system(stringf(templatermcmd, hhhfile));
	  system(stringf(templatermcmd, cccfile));
	  system(stringf(templatemvcmd, tmpfile, dcgfile));
	  if (verbose)
	       fprint(2, "Done.\n");
     }
     if (events.end)
	  apply(events.end, NULL, NULL);
     memset(&events, 0, sizeof events);
     emitYYnull();
     if (glevel || xref) {
	  Symbol symroot = NULL;
	  Coordinate src;
	  foreach(types,       GLOBAL, typestab, &symroot);
	  foreach(identifiers, GLOBAL, typestab, &symroot);
	  src.file = firstfile;
	  src.x = 0;
	  src.y = lineno;
	  if ((glevel > 2 || xref) && IR->stabend)
	       (*IR->stabend)(&src, symroot,
			      ltov(&loci,    PERM),
			      ltov(&symbols, PERM), NULL);
	  else if (IR->stabend)
	       (*IR->stabend)(&src, NULL, NULL, NULL, NULL);
     }
     finalize();
     (*IR->progend)();
     outflush();
     close(outfd);
     close(errfd);
     deallocate(PERM);
     return errcnt > 0;
}
/* compilestring - compile str */
static void compilestring(str) char *str; {
     inputstring(str);
     t = gettok();
     program();
}

/* doargs - process program arguments, removing top-half arguments from argv */
static int doargs(argc, argv) int argc; char *argv[]; {
     int i, j;

     for (i = j = 1; i < argc; i++)
	  if (strcmp(argv[i], "-g") == 0)
	       glevel = 2;
	  else if (strncmp(argv[i], "-g", 2) == 0
		   && argv[i][2] && argv[i][2] >= '0' && argv[i][2] <= '9') {
	       glevel = argv[i][2] - '0';
#ifdef STABINIT
	       {
		    extern void STABINIT ARGS((char *, int, char *[]));
		    IR->stabinit = STABINIT;
	       }
#endif
	  } else if (strcmp(argv[i], "-x") == 0)
	       xref++;
	  else if (strcmp(argv[i], "-A") == 0) {
	       if (++Aflag >= 2)
		    Xflag = 0;
	  } else if (strcmp(argv[i], "-X") == 0)
	       Xflag++;
	  else if (strcmp(argv[i], "-P") == 0)
	       Pflag++;
	  else if (strcmp(argv[i], "-w") == 0)
	       wflag++;
	  else if (strcmp (argv[i], "-b")    == 0
		   ||       strcmp (argv[i], "-C")    == 0
		   ||       strncmp(argv[i], "-a", 2) == 0)
	       profInit(argv[i]);
	  else if (strcmp(argv[i], "-n") == 0) {
	       if (!YYnull) {
		    YYnull = install(string("_YYnull"), &globals, 
				     GLOBAL, PERM);
		    YYnull->type = ftype(voidtype, inttype);
		    YYnull->sclass = STATIC;
		    (*IR->defsymbol)(YYnull);
	       }
	  } else if (strncmp(argv[i], "-n", 2) == 0) { /* -nvalid[,check] */
	       char *p = strchr(argv[i], ',');
	       if (p) {
		    YYcheck = install(string(p+1), &globals, GLOBAL, PERM);
		    YYcheck->type = func(voidptype, NULL, NULL, 1);
		    YYcheck->sclass = EXTERN;
		    (*IR->defsymbol)(YYcheck);
		    p = stringn(argv[i]+2, p - (argv[i]+2));
	       } else
		    p = string(argv[i]+2);
	       YYnull = install(p, &globals, GLOBAL, PERM);
	       YYnull->type = func(voidptype, NULL, NULL, 1);
	       YYnull->sclass = EXTERN;
	       (*IR->defsymbol)(YYnull);
	  } else if (strncmp(argv[i], "-target=", 8) == 0)
	       ;
#if !defined(NDEBUG)
	  else if (strcmp(argv[i], "-trees") == 0)
	       tflag = 1;
#endif
	  else if (strncmp(argv[i], "-t", 2) == 0)
	       traceInit(&argv[i][2]);
	  else if (strncmp(argv[i], "-u", 2) == 0)
	       eval.autounroll = 1;
	  else if (strcmp(argv[i], "-V") == 0) 
	       eval.fc = 1;
	  else if (strcmp(argv[i], "-v") == 0) {
	       int i;
	       verbose = 1;
	       fprint(2, "%s %s targets:\n", argv[0], rcsid);
	       for (i = 0; bindings[i].name; i++)
		    fprint(2, "\t%s%s\n", bindings[i].name,
			   (sIR == bindings[i].ir) 
			   && (dIR == bindings[i].Iir) ? "*" : "");
	  } else if (strncmp(argv[i], "-s", 2) == 0)
	       density = strtod(&argv[i][2], NULL);
	  else if (strncmp(argv[i], "-errout=", 8) == 0) {
	       char *errfile = argv[i] + 8;
	       {
		    errfd = creat(errfile, 0666);
		    if (errfd < 0) {
			 errfd = 2;
			 fprint(2, "%s: can't write errors to `%s'\n", 
				argv[0], errfile);
			 exit(1);
		    }
	       }

	  } else if (strncmp(argv[i], "-e", 2) == 0) {
	       int x;
	       if ((x = strtol(&argv[i][2], NULL, 0)) > 0)
		    errlimit = x;
	  } else if (strcmp(argv[i], "-nodag") == 0)
	       IR->wants_dag = !IR->wants_dag;
	  else if (strcmp(argv[i], "-") == 0 || *argv[i] != '-') {
	       if (infile == 0)
		    infile = argv[i];
	       else if (outfile == 0)
		    outfile = argv[i];
				/* `C: 3rd non-flag argument is dcgfile */
	       else if (dcgfile == 0)
		    dcgfile = argv[i];
	       else
		    argv[j++] = argv[i];
	  } else {
	       if (strcmp(argv[i], "-XP") == 0)
		    argv[i] = "-p";
	       else if (strncmp(argv[i], "-X", 2) == 0)
		    *++argv[i] = '-';
	       argv[j++] = argv[i];
	  }
     argv[j] = 0;
     return j;
}
/* emitYYnull - compile definition for _YYnull, if it's referenced and named "_YYnull" */
static void emitYYnull() {
     if (YYnull && YYnull->ref > 0.0
	 && strcmp(YYnull->name, "_YYnull") == 0) {
	  Aflag = 0;
	  YYnull->defined = 0;
	  YYnull = NULL;
	  compilestring(stringf("static char *_YYfile = \"%s\";\n", file));
	  compilestring("static void _YYnull(int line,...) "
			"{\nchar buf[200];\nsprintf(buf, \""
			"null pointer dereferenced @%s:%d\\n"
			"\", _YYfile, line);\nwrite(2, buf,"
			"strlen(buf));\nabort();\n}\n");
     }
}

/* typestab - emit stab entries for p */
static void typestab (Symbol p, void *cl) {
     if (*(Symbol *)cl == 0 && p->sclass && p->sclass != TYPEDEF)
	  *(Symbol *)cl = p;
     if ((p->sclass == TYPEDEF || p->sclass == 0) && IR->stabtype)
	  (*IR->stabtype)(p);
}

/* envInit: read standard `C environment, and decide which back end to use */
static void envInit (int argc, char *argv[]) {
     if ((infd = open(envfile, 0)) < 0) {
	  fprint(2, "%s: can't read `%s'\n", argv[0], envfile);
	  exit(1);
     }
     inputInit();
     outputInit();
     t = gettok();
     (*IR->progbeg)(argc, argv);
     program();
     close(infd);
     infd = 0;
				/* What dynamic back end to use? */
     dIR = eval.fc ? binding->Vir : binding->Iir;
}

/* closureInit: initialize stuff needed to compile closures */
static void closureInit (void) {
     tcname.Cgfroot = string("_tc_cvi_");
     tcname.Cgfsuffix = string("code");
     tcname.Cgffield = string("code");
     tcname.Labfield = string("lab");
     tcname.Gencspec = string("_tc_cspec");

     tcname.Malloc = string("tc_aalloc");
     tcname.Leafp = string("_tc_leafp");

     tcname.Closure_t = string("_tc_closure_t");
     tcname.Local_t = string("_tc_vspec_t");
     tcname.Cspec_t = string("_tc_cspec_t");
     tcname.Label_t = string("i_label_t");
     tcname.Code_t = string("_tc_code_t");
     tcname.Dcall_t = string("_tc_dcall_t");

     tcname.Local = string("_tc_local");
     tcname.Localf = string("_tc_localf");
     tcname.Localb = string("_tc_localb");
     tcname.Localbf = string("_tc_localbf");
     tcname.Param = string("_tc_param");
     tcname.Paramf = string("_tc_paramf");

     tcname.Pushi = string("_tc_push_init");
     tcname.Push = string("_tc_push");
     tcname.Arg = string("_tc_arg");

     tcname.Compile = string("_tc_compile");
     tcname.Compilef = string("_tc_compilef");
     tcname.Decompile = string("_tc_decompile");

     tcname.Jump = string("_tc_jump");
     tcname.Jumpf = string("_tc_jumpf");
     tcname.Label = string("_tc_label");
     tcname.Labelf = string("_tc_labelf");
     tcname.Mktarget = string("_tc_mktarget");
}

%{
#include <stdio.h>
#include "bin.tab.h"

int verbose=1,yylineno=1;
FILE *tf=NULL;
int debug = 0;
%}
%x rules
%x text
/* note: using these rules, "()" and "{}" pairs *cannot* nest */
args		"("[^)]*")"
text 		"{"[^}]*"}"
space		[ \t]*
%%
<rules>0x[0-9a-f]+			{ return HEX; }
<rules>[01]+			{ return BINST; }
<rules>[a-z_][0-9_a-z]*			{ return INST; }
<rules>{args}			{ BEGIN(text); return ARGS; } 
<text>[^{]* 			{ BEGIN(rules); return TEXT; }
<rules>"{"|"}"			{ return yytext[0]; }
<rules>{space}			/* eat up space */
<rules>\n			{ yylineno++; }
<rules><<EOF>>			{ return 0; }
<rules>"/*"     {       /* eat comments (they can nest) */
                        int c, nest = 1;

                        for ( ; nest ; )
                        {
                                switch(input())
                                {
                                        case '\n': yylineno++; break;
                                        case '*' :
                                                while((c = input()) == '*') ;
                                                if (c == '/') --nest;
                                                break;
                                        case '/':
                                                while ( (c = input()) == '/' ) ;
                                                if(c == '*') nest++;
                                                break;
                                        case EOF:
                                                fputs("EOF in comment",stderr);
                                                break;
                                }
                        }
                }

<rules>.			{ 
                        	fprintf(stderr,"line %d: illegal token `%s'\n",
					yylineno,yytext);
                        	exit(1);
                        	}

"%%"\n		{ BEGIN(rules); yylineno++; }
[^\n]*\n	{ printf("%s", yytext); yylineno++; }
%%	
/* sparc specific */
debug_prologue()
{
	fputs("#include \"binary.h\"\n",tf);
	fputs("static unsigned long code[1024], *dcg_p = code, *cb;\n\n",tf);
	fputs("main()\n{\n",tf);
	fputs("\tunsigned long pc, dst=8, src1=14, src2=64,simm13=64;\n",tf);
	fputs("\tchar buf[1024];\n",tf);
}

debug_epilogue()
{
	fputs("\treturn 0;\n}\n",tf);
}

main(int argc, char *argv[]) 
{ 
	int i;
	
  	for(i=1;i<argc;i++)
    		if(strcmp(argv[i],"-d") == 0)  tf = fopen(argv[++i],"w");
    		else if(strcmp(argv[i],"-g") == 0)  debug = 1;
		else if(strcmp(argv[i],"-h") == 0)  verbose = 0;
    		else yyerror("incorrect cmd line option");
	
	if(tf) debug_prologue();
	yyparse(); 
	if(tf) debug_epilogue();
	return 0;
}

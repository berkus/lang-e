%{
#include "calc.tab.h"
int yylineno = 1;
%}
%%
[0-9]+		{ sscanf(yytext,"%d",&yylval.val); return CNST; }

"-"|"+"|"/"|"*"|"("|")"	{ return yytext[0]; }
[ \t]+			/* eat up whitespace */
\n			{ return '\n'; }
<<EOF>>			{ return 0; }

.	{ 
		fprintf( stderr, "line %d: illegal token <%s>\n",yylineno,yytext);
		exit(1);
	}
%%	

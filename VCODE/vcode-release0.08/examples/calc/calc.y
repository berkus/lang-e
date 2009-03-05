%{
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>

extern char *yytext;
%}
%union {
        int val;
}
%type <val> '+' '-' '*' '/' expr CNST
%token CNST
%left '+' '-'           /* left assoc., same precedence */
%left '*' '/'           /* left assoc., higher precedence */
%right uminus
%%
list: 	/* nothing */
	| list '\n'
	| list expr '\n' 	{ printf("%d\n", $2); }
	;

expr: 	CNST			{ $$ = $1; }
	| expr '+' expr		{ $$ = $1 + $3; }
	| expr '-' expr		{ $$ = $1 - $3; }
	| expr '*' expr		{ $$ = $1 * $3; }
	| expr '/' expr 	{ $$ = $1 / $3; }
	| '(' expr ')'  	{ $$ = $2; }
	| '-' expr %prec uminus { $$ = - $1; }
	;
%%
yyerror(char *s) {
	extern int yylineno;
        fprintf(stderr,"line %d: %s\n",yylineno,s);
}

main() {
        yyparse();
}

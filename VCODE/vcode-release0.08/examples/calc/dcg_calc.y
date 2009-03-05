%{
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <vcode.h>

static unsigned i[1000];

v_iptr ip;

extern char *yytext;
%}
%union {
        int val;
	v_reg_type r;
}
%type <r> '+' '-' '*' '/' expr 
%type <val> CNST
%token CNST
%left '+' '-'           /* left assoc., same precedence */
%left '*' '/'           /* left assoc., higher precedence */
%right uminus
%%
list: 	/* nothing */
	| list '\n'
	| list 	{ v_lambda("foo", "", NULL, V_LEAF, i, sizeof i); } 
	  expr '\n' 	
		{ 
			v_reti($3); 
			ip = v_end(0).i; 
			/* v_dump(ip); */
			printf("%d\n", ip());
		}
	;
expr: 	CNST			{ 
			v_reg_type r;
			if(!v_getreg(&r, V_I, V_TEMP))
				v_fatal("out of registers");
			 $$ = r; 
			v_seti($$, $1); 
		}
	| expr '+' expr		{ v_putreg($1, V_I); v_addi($3, $1, $3); $$ = $3; }
	| expr '-' expr		{ v_putreg($1, V_I); v_subi($3, $1, $3); $$ = $3; }
	| expr '*' expr		{ v_putreg($1, V_I); v_muli($3, $1, $3); $$ = $3; }
	| expr '/' expr		{ v_putreg($1, V_I); v_divi($3, $1, $3); $$ = $3; }
	| '(' expr ')'  	{ $$ = $2; }
	| '-' expr %prec uminus { v_negi($2, $2); $$ = $2; }
	;
%%
yyerror(char *s) {
	extern int yylineno;
        fprintf(stderr,"line %d: %s\n",yylineno,s);
}

int main() {
        yyparse();
	return 0;
}

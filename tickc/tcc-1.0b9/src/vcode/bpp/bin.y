%{
#include <stdio.h>
#include <string.h>
#include <assert.h>

#define YYSTYPE int  /* data type of yacc stack */

static int nbin, nisn;
static char name[100][BUFSIZ],bin[100][BUFSIZ],text[BUFSIZ],args[BUFSIZ];
extern char *yytext;
extern int debug,verbose;
extern char *htob(char *p) ;
extern FILE *tf; /* test file */
char *expand(char *inst, char *binst);
void btoh(char *b);
%}
%token TEXT   	/* macro text */
%token HEX
%token BINST  	/* binary instruction 010101 */
%token INST   	/* assembler tag (addi, ...) */
%token ARGS
%%
list:   /* nothing */
        | list def
	| list act  { 
		int i;
		assert(nbin==nisn);

		for(i=0; i < nbin; i++) 
		{
			btoh(bin[i]);
                        if(verbose)
                        printf("#define %s%s do {%s} while(0)\n",
                                name[i],args,expand(name[i],bin[i]));
                        else
                                printf("#define %s%s\n", name[i],args);
			if(tf) debug_dump(name[i]);
		}
		nbin = nisn = 0;
		}
        ;
def:   args text
	;
act:   '{' specs  '}'
	;
specs: /* nothing */
	| inst specs
	| binst specs
	;
inst: INST 	{ strcpy(name[nisn++],yytext); }
binst: BINST 	{ strcpy(bin[nbin++], yytext); }
	| HEX	{ strcpy(bin[nbin++], htob(yytext)); }
text: TEXT 	{ strcpy(text, yytext); }
args: ARGS	{ strcpy(args, yytext); }

%%
yyerror(char *s)
{
        extern int yylineno;
        fprintf(stderr,"line %d: %s\n",yylineno,s);
}

char *htob(char *p) {
	static char buf[1024], *q;
	unsigned x;
	int i;

	sscanf(p, "0x%x", &x);
	for(q = &buf[0], i = 31; i >= 0; i--)
		*q++ = ((1 << i) & x) ? '1' : '0';
	buf[32] = 0;
	return &buf[0];
}


strcnt(char c, char *str)
{
	int cnt = 0;

	while(*str != ')') 
		if(c == *str++) cnt++;

	return cnt;
}

debug_dump(char *inst)
{
	fputs("\tcb=dcg_p;\n",tf);
        if(args[0] != '(' || args[1] != ')')
                switch(strcnt(',', args)) {
                        case 2: fprintf(tf,"\t%s(dst, src1, src2);\n", inst);
                                break;
                        case 1: fprintf(tf,"\t%s(dst, src1);\n", inst);
                                break;
                        case 0: fprintf(tf,"\t%s(dst);\n", inst);
                                break;
                        default: return; /* don't emit rule */
                        }
/*
	fputs("\twhile(cb < dcg_p) {\n\t\tdi(*cb++, pc, buf);",tf);
*/
	fputs("\twhile(cb < dcg_p) {\n\t\tdecode_instr(*cb++, pc, buf);",tf);
	fputs("\t\tdcg_print(\"%s\\n\",buf); pc+=4;\n\t}\n",tf);
	fprintf(tf,"\tdcg_print(\"<%s>\\n\");\n",inst);
}

/* binary -> hex */
void btoh(char *p)
{
	int h = 0;
	char *b = p;

	while(*b) h = (h << 1) + *b++ - '0';
	sprintf(p,"0x%x",h);
} 

char * min(char *a, char *b)
{
	if(!a) return b;
	if(!b) return a;
	return (a < b) ? a : b;
}
	
/* macro expansion */
char *expand(char *inst, char *binst)
{
	static char expanded[BUFSIZ];
	char *e = expanded, *t = text, *b;
	int ilen = strlen(inst), blen = strlen(binst);

	/* find all instances of @bin */
	while(b = strstr(t, "@bin"))
	{
		int step = b - t;

		/* copy upto match */
		bcopy(t, e, step * sizeof(char));
		e += step;
		t += step;
		bcopy(binst,e, blen); 
		e += blen; t += sizeof("@bin")-1;
	}

	/* copy remainder of string */
	strcpy(e,t);

	if(debug) 
	{
		char d1[] = "unsigned long *cb = dcg_p;",
		     d2[] = "char buf[1024];",
		     d3[] = "while(cb < dcg_p) {",
		     d4[] = "decode_instr(*cb++, 0, buf);dcg_print(\"%s\\n\",buf); }",
		     d5[1024], tmp[1024], *t = tmp, *d=expanded;

		bzero(tmp,1024);
		strcat(t, d1);
		strcat(t, d2);
		t += strlen(t);
		while(*d) *t++ = *d++;
		sprintf(d5,"\t\tdcg_print(\"expected %s\\n\");",inst); 

		strcat(t, d3);
		strcat(t, d4);
		/* strcat(t, d5); */
		t += strlen(t);
		strcpy(expanded, tmp);
	}

	return expanded;
}	

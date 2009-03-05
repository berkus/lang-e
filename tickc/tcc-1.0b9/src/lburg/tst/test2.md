%{
#define NULL 0
#define P1 0D0111
#define P2 0D0222
#define P3 0D0333
#define P4 0D0444
#define P5 0D0555
%}
%start s
%term A=1 B=2 C=3 D=4 E=5
%%
s: r1
s: r2
r1: A	P1
r1: B	P2
r1: C	P3
r2: D	P4
r2: E	P5
%%

void empty (void) {}

%{
#define NULL 0
#define P1 0x01
#define P2 0x02
#define P3 0x03
#define P4 0x04
#define P5 0x05
%}
%start s
%term A=1 B=2 C=3 D=4 E=5
%%
s: r1
s: r2
r1: A	"P1"
r1: B	"P2"
r1: C	"P3"
r2: D	"P4"
r2: E	"P5"
%%

void empty (void) {}

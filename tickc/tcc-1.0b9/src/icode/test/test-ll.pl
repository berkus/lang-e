#!perl

$lengthl1 = 6;
				# HEADER
print <<"EOF"
#include "ll.h"

void ef(i_llt l, i_llet e, void *v1) {
    printf("\t\t\t** \%d\\n", e->lr->n);
}
unsigned int sf(i_llt l, i_llet e, void *v1, void *v2) {
    printf("\t\t\t** \%d\\n", e->lr->n);
    if (e->lr->n == 3)
	return 1;
    return 0;
}

void main(void) {
   i_llt ll1 = i_llcreate($lengthl1);
EOF
    ;
				# DECLARATIONS
for($i=0;$i<$lengthl1;$i++) {
    print <<"EOF"
   i_lrt lr$i;	
   i_llet le$i;
EOF
    ;
}
				# INITIALIZATIONS
for($i=0;$i<$lengthl1;$i++) {
    print <<"EOF"
   NEW0(lr$i, 1);
   lr$i->n = $i;
EOF
    ;
}
				# PRINT EMPTY
print <<"EOF"
   i_llunparse(ll1);
EOF
    ;
				# INSERT FORWARD
for($i=0;$i<$lengthl1;$i++) {
    print <<"EOF"
   le$i = i_llifwd(ll1, NULL);
   i_llset(le$i, lr$i);
EOF
    ;
}

print <<"EOF"
   printf("INSERTING FORWARD\\n");
   i_llunparse(ll1);
   printf("REMOVING FORWARD\\n");
EOF
    ;
				# REMOVE FORWARD
for($i=0;$i<$lengthl1;$i++) {
    print <<"EOF"
   i_llrem(ll1, le$i);
   i_llunparse(ll1);
EOF
    ;
}
				# INSERT FORWARD
for($i=0;$i<$lengthl1;$i++) {
    print <<"EOF"
   le$i = i_llifwd(ll1, NULL);
   i_llset(le$i, lr$i);
EOF
    ;
}

print <<"EOF"
   printf("INSERTING FORWARD\\n");
   i_llunparse(ll1);
   printf("SEARCHING FWD\\n");
   i_llsfwd(ll1, sf, NULL, NULL);
   printf("SEARCHING BWD\\n");
   i_llsbwd(ll1, sf, NULL, NULL);
   printf("ITER FWD\\n");
   i_lleachfwd(ll1, ef, NULL);
   printf("ITER BWD\\n");
   i_lleachbwd(ll1, ef, NULL);
   printf("REMOVING BACKWARD\\n");
EOF
    ;
				# REMOVE BACKWARD
for($i=$lengthl1-1;$i>=0;$i--) {
    print <<"EOF"
   i_llrem(ll1, le$i);
   i_llunparse(ll1);
EOF
    ;
}
				# INSERT BACKWARD
for($i=0;$i<$lengthl1;$i++) {
    print <<"EOF"
   le$i = i_llibwd(ll1, NULL);
   i_llset(le$i, lr$i);
EOF
    ;
}
print <<"EOF"
   printf("INSERTING BACKWARD\\n");
   i_llunparse(ll1);
   printf("REMOVING FORWARD\\n");
EOF
    ;
				# REMOVE FORWARD
for($i=0;$i<$lengthl1;$i++) {
    print <<"EOF"
    i_llrem(ll1, le$i);
    i_llunparse(ll1);
EOF
    ;
}
				# INSERT BACKWARD
for($i=0;$i<$lengthl1;$i++) {
    print <<"EOF"
   le$i = i_llibwd(ll1, NULL);
   i_llset(le$i, lr$i);
EOF
    ;
}
print <<"EOF"
   printf("INSERTING BACKWARD\\n");
   i_llunparse(ll1);
   printf("SEARCHING FWD\\n");
   i_llsfwd(ll1, sf, NULL, NULL);
   printf("SEARCHING BWD\\n");
   i_llsbwd(ll1, sf, NULL, NULL);
   printf("ITER FWD\\n");
   i_lleachfwd(ll1, ef, NULL);
   printf("ITER BWD\\n");
   i_lleachbwd(ll1, ef, NULL);
   printf("REMOVING BACKWARD\\n");
EOF
    ;
				# REMOVE BACKWARD
for($i=$lengthl1-1;$i>=0;$i--) {
    print <<"EOF"
   i_llrem(ll1, le$i);
   i_llunparse(ll1);
EOF
    ;
}
				# RANDOM INSERTIONS/DELETIONS
print <<"EOF"
   printf("STRANGE PATTERN INSERTIONS AND DELETIONS\n");
   le0 = i_llifwd(ll1, NULL);
   i_llset(le0, lr0);
   i_llunparse(ll1);
   le1 = i_llibwd(ll1, le0);
   i_llset(le1, lr1);
   i_llunparse(ll1);
EOF
    ;
for($i=2;$i<$lengthl1;$i++) {
    $j = $i-2;
    print <<"EOF"
   le$i = i_llifwd(ll1, le$j);
   i_llset(le$i, lr$i);
   i_llunparse(ll1);
EOF
    ;
}
print <<"EOF"
}
EOF
    ;

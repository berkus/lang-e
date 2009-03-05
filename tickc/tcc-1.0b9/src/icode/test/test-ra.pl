#!perl 
$csize = 500;			# Code size
$ni = 5;			# Numbers of ints, chars, floats, doubles
$nc = 5;
$np = 2;
$nf = 5;
$nd = 5;
				# HEADER
print <<"EOF"
#include \"icode.h\"

char cformat[] = "%c\\n";
char iformat[] = "%d\\n";
char fformat[] = "%f\\n";
char dformat[] = "%e\\n";
void gen() {
EOF
    ;
				# DECLARE & INIT LOCALS
for ($i = 0; $i < $np; $i++) {
    print <<"EOF"
   i_local_t lp$i = i_local(0, I_P);
EOF
    ;
}
for ($i = 0; $i < $ni; $i++) {
    print <<"EOF"
   i_local_t li$i = i_local(0, I_I);
EOF
    ;
}
for ($i = 0; $i < $nc; $i++) {
    print <<"EOF"
   i_local_t lc$i = i_local(0, I_C);
EOF
    ;
}
for ($i = 0; $i < $nf; $i++) {
    print <<"EOF"
   i_local_t lf$i = i_local(0, I_F);
EOF
    ;
}
for ($i = 0; $i < $nd; $i++) {
    print <<"EOF"
   i_local_t ld$i = i_local(0, I_D);
EOF
    ;
}
				# ASSIGNMENTS
for ($i = 0; $i < $ni; $i++) {
    $_li[$i] = $i;
    print <<"EOF"
   i_seti(li$i, $_li[$i]);
EOF
    ;
}
for ($i = 0; $i < $nc; $i++) {
    $_lc[$i] = $i+65;
    print <<"EOF"
   i_setc(lc$i, $_lc[$i]);
EOF
    ;
}
for ($i = 0; $i < $nf; $i++) {
    $_lf[$i] = $i+0.1;
    print <<"EOF"
   i_setf(lf$i, $_lf[$i]);
EOF
    ;
}
for ($i = 0; $i < $nd; $i++) {
    $_ld[$i] = $i+0.1;
    print <<"EOF"
   i_setd(ld$i, $_ld[$i]);
EOF
    ;
}

sub add  {
    local($t, $d, $s1, $s2) = @_;
    print "   i_add$t(l$t$d,l$t$s1,l$t$s2);\n";
    if ($t eq 'c') { $_lc[$d] = $_lc[$s1]+$_lc[$s2]; }
    elsif ($t eq 'i') { $_li[$d] = $_li[$s1]+$_li[$s2]; }
    elsif ($t eq 'f') { $_lf[$d] = $_lf[$s1]+$_lf[$s2]; }
    elsif ($t eq 'd') { $_ld[$d] = $_ld[$s1]+$_ld[$s2]; }
}
sub sub  {
    local($t, $d, $s1, $s2) = @_;
    print "   i_sub$t(l$t$d,l$t$s1,l$t$s2);\n";
    if ($t eq 'c') { $_lc[$d] = $_lc[$s1]-$_lc[$s2]; }
    elsif ($t eq 'i') { $_li[$d] = $_li[$s1]-$_li[$s2]; }
    elsif ($t eq 'f') { $_lf[$d] = $_lf[$s1]-$_lf[$s2]; }
    elsif ($t eq 'd') { $_ld[$d] = $_ld[$s1]-$_ld[$s2]; }
}
sub mul  {
    local($t, $d, $s1, $s2) = @_;
    print "   i_mul$t(l$t$d,l$t$s1,l$t$s2);\n";
    if ($t eq 'c') { $_lc[$d] = $_lc[$s1]*$_lc[$s2]; }
    elsif ($t eq 'i') { $_li[$d] = $_li[$s1]*$_li[$s2]; }
    elsif ($t eq 'f') { $_lf[$d] = $_lf[$s1]*$_lf[$s2]; }
    elsif ($t eq 'd') { $_ld[$d] = $_ld[$s1]*$_ld[$s2]; }
}
sub div  {
    local($t, $d, $s1, $s2) = @_;
    print "   i_div$t(l$t$d,l$t$s1,l$t$s2);\n";
    if ($t eq 'c') { $_lc[$d] = $_lc[$s1]/$_lc[$s2]; }
    elsif ($t eq 'i') { $_li[$d] = $_li[$s1]/$_li[$s2]; }
    elsif ($t eq 'f') { $_lf[$d] = $_lf[$s1]/$_lf[$s2]; }
    elsif ($t eq 'd') { $_ld[$d] = $_ld[$s1]/$_ld[$s2]; }
}
sub put {
    local($s) = @_;
    print "   $s\n";
}
				# RANDOM CODE

put "i_seti(li0, 5);";
put "i_mulii(li0, li0, 12);";

# add('i', 4, 1, 0);
# add('i', 3, 2, 1);
# add('i', 0, 4, 3);

put "i_setp(lp0, iformat);";
put "i_argp(lp0);";
put "i_argi(li0);";
put "i_callvi(printf);";

#put "printf(\"EXPECT: $_li[0]\\n\");";

# &add('f', 4, 1, 0);
# &add('f', 3, 2, 1);
# &add('f', 0, 4, 3);

# &add('d', 4, 1, 0);
# &add('d', 3, 2, 1);
# &add('d', 0, 4, 3);

				# END
print <<"EOF"
}

void main() {
    unsigned int offset;
    union v_fp vfp;
    void (*vf)();
    i_init($csize);
    gen();
    i_end();
    i_debugon();
    vfp = i_emit(&offset);
    vf = vfp.v;
    v_dump((void*)vf);
    (*vf)();
}
EOF
    ;
put "static int _tciu_i_setc;";
put "static int _tciu_i_seti;";
put "static int _tciu_i_setp;";
put "static int _tciu_i_setf;";
put "static int _tciu_i_setd;";
put "static int _tciu_i_addi;";
put "static int _tciu_i_addf;";
put "static int _tciu_i_addd;";
put "static int _tciu_i_subi;";
put "static int _tciu_i_subf;";
put "static int _tciu_i_subd;";
put "static int _tciu_i_muli;";
put "static int _tciu_i_mulii;";
put "static int _tciu_i_mulf;";
 put "static int _tciu_i_muld;";
 put "static int _tciu_i_divi;";
 put "static int _tciu_i_divf;";
 put "static int _tciu_i_divd;";
 put "static int _tciu_i_cvc2i;";
put "static int _tciu_i_argi;";
put "static int _tciu_i_argp;";
 put "static int _tciu_i_argf;";
 put "static int _tciu_i_argd;";
put "static int _tciu_i_callvi;";
put "static int _tciu_i_callv;";

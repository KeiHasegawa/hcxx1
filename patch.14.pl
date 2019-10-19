#State XXX
#
#  AAA relational_expression: relational_expression . '<' shift_expression
#  BBB                      | relational_expression . '>' shift_expression
#  CCC                      | relational_expression . LESSEQ_MK shift_expression
#  DDD                      | relational_expression . GREATEREQ_MK shift_expression
#  EEE equality_expression: relational_expression .
#
#  Example:
#
#  template<int N> class C { int a[N]; };
#  C<5> c;
#


while ( <> ){
    chop;
    next if ( !/^[Ss]tate (.*)/ );
    $xxx = $1;
    $_ = <>;
    $_ = <>; chop;
    next if (!/([0-9]+) relational_expression: relational_expression \. '<' shift_expression/);
    $aaa = $1;
    $_ = <>; chop;
    next if (!/([0-9]+) +\| relational_expression . '>' shift_expression/);
    $bbb = $1;
    $_ = <>; chop;
    next if (!/([0-9]+) +\| relational_expression \. LESSEQ_MK shift_expression/);
    $ccc = $1;
    $_ = <>; chop;
    next if (!/([0-9]+) +\| relational_expression \. GREATEREQ_MK shift_expression/);
    $ddd = $1;
    $_ = <>; chop;
    next if (!/([0-9]+) equality_expression: relational_expression \./);
    $eee = $1;
    goto label;
}

print STDERR "Error detected at $0\n";
exit 1;

label:
print<<EOF
  if (yystate == $xxx) {
    if (yychar == '>') {
      using namespace cxx_compiler;
      if (parse::templ::param || parse::templ::arg > 0) {
        YYDPRINTF((stderr, "patch.14 is applied\\n"));
        yyn = $eee + 1;
        goto yyreduce;
      }
    }
  }
EOF


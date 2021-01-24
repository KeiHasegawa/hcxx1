# See also patch.21.pl
# patch.21.p : not reference `parse::templ::arg'
# patch.26.p : refenrece it 
#
#State XXX
#
#  AAA postfix_expression: postfix_expression . '[' expression ']'
#  BBB                   | postfix_expression . '(' expression_list ')'
#  CCC                   | postfix_expression . '(' ')'
#  DDD                   | postfix_expression . PLUSPLUS_MK
#  EEE                   | postfix_expression . MINUSMINUS_MK
#  FFF member_access_begin: postfix_expression . '.'
#  GGG                    | postfix_expression . ARROW_MK
#  HHH unary_expression: postfix_expression .
#
# Example:
#  template<class C1> struct S1 { /* ... */ };
#  template<bool B> struct S2 { /* ... */ };
#  ...
#  S1<S2<true>> x;

while (<>) {
    chop;
    next if ( !/^[Ss]tate (.*)/ );
    $xxx = $1;
    $_ = <>;
    $_ = <>; chop;
    next if ( !/([0-9]+) postfix_expression: postfix_expression \. '\['/ );
    goto label;
}

print STDERR "Error detected at $0\n";
exit 1;

label:
print<<EOF
  if (yystate == $xxx) {
    if (yychar == RSH_MK) {
      using namespace cxx_compiler;
      if (parse::templ::arg > 0 && !parse::templ::parenthesis) {
        YYDPRINTF((stderr, "patch.26 is applied\\n"));
        parse::g_read.m_token.push_front(make_pair('>', parse::position));
        yychar = '>';
      }
    }
  }
EOF


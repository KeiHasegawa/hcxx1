#State XXX
#
#   YYY type_specifier: simple_type_specifier .
#   ZZZ postfix_expression: simple_type_specifier . '(' expression_list ')'
#   WWW                   | simple_type_specifier . '(' ')'
#

while ( <> ){
    chop;
    next if ( !/^[Ss]tate (.*)/ );
    $xxx = $1;
    $_ = <>;
    $_ = <>; chop;
    next if ( !/type_specifier: simple_type_specifier/ );
    $_ = <>; chop;
    next if ( !/postfix_expression: simple_type_specifier/ );
    $_ = <>; chop;
    next if ( !/| simple_type_specifier/ );
    goto label;
}

label:
print <<EOF
  if ( yystate == $xxx ) {
    YYDPRINTF((stderr, "rule.03 is applied\\n"));
    if ( cxx_compiler::parse::backtrack::g_stack.empty() ) {
      cxx_compiler::parse::backtrack::g_stack.push(yyssp-1);
      goto yydefault;
    }
    else {
      cxx_compiler::parse::backtrack& x = cxx_compiler::parse::backtrack::g_stack.top();
      ++x.m_way;
      if ( x.m_way == 1 ) {
        cxx_compiler::parse::identifier::flag = cxx_compiler::parse::identifier::look;
      }
      else {
        cxx_compiler::parse::backtrack::g_stack.pop();
      }
    }
  }
EOF

#State XXX
#
#   YYY postfix_expression: simple_type_specifier '(' . expression_list ')'
#   ZZZ                   | simple_type_specifier '(' . ')'

while ( <> ){
    chop;
    next if ( !/^[Ss]tate (.*)/ );
    $xxx = $1;
    $_ = <>;
    $_ = <>; chop;
    next if ( !/postfix_expression: simple_type_specifier \'\(\' \. expression_list \'\)\'/ );
    $_ = <>; chop;
    next if ( !/| simple_type_specifier \'\(\' . \'\)\'/ );
    goto label;
}

label:
print <<EOF
  if ( yystate == $xxx )
    cxx_compiler::parse::identifier::flag = cxx_compiler::parse::identifier::look;
EOF

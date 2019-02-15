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
    next if ( !/([0-9]+) type_specifier: simple_type_specifier/ );
    $yyy = $1;
    $_ = <>; chop;
    next if ( !/postfix_expression: simple_type_specifier/ );
    $_ = <>; chop;
    next if ( !/| simple_type_specifier/ );
    goto label;
}

label:
print <<EOF
  if (yystate == $xxx && yychar == '(') {
    YYDPRINTF((stderr, "rule.03 is applied\\n"));
    yyn = $yyy + 1;
    goto yyreduce;
  }
EOF

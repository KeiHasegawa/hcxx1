#State XXX
#
# AAA elaborated_type_specifier: typenaming nested_name_specifier TYPEDEF_NAME_LEX .
# BBB postfix_expression: typenaming nested_name_specifier TYPEDEF_NAME_LEX . '(' fcast_prev2 expression_list ')'
# CCC                   | typenaming nested_name_specifier TYPEDEF_NAME_LEX . '(' fcast_prev2 ')'
#

while (<>) {
    chop;
    next if ( !/^[Ss]tate (.*)/ );
    $xxx = $1;
    $_ = <>;
    $_ = <>; chop;
    next if (!/([0-9]+) elaborated_type_specifier: typenaming nested_name_specifier TYPEDEF_NAME_LEX \./);
    $aaa = $1;
    $_ = <>; chop;
    next if (!/([0-9]+) postfix_expression: typenaming nested_name_specifier TYPEDEF_NAME_LEX \./);
    $bbb = $1;
    goto label;
}

print STDERR "Error detected at $0\n";
exit 1;

label:
print<<EOF
  if (yystate == $xxx) {
    using namespace cxx_compiler::parse;
    if (peek() != '(') {
      YYDPRINTF((stderr, "patch.17 is applied\\n"));
      identifier::mode = identifier::new_obj;
    }
  }
EOF

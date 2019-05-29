#State XXX
#
# R1 declarator_id: type_name .
#
#State YYY
#
# R2 direct_declarator: '(' . declarator ')'
# R3 direct_abstract_declarator: '(' . enter_parameter parameter_declaration_clause leave_parameter ')' cvr_qualifier_seq exception_specification
# R4                           | '(' . enter_parameter parameter_declaration_clause leave_parameter ')' exception_specification
# R5                           | '(' . enter_parameter parameter_declaration_clause leave_parameter ')' cvr_qualifier_seq
# R6                           | '(' . enter_parameter parameter_declaration_clause leave_parameter ')'
# R7                           | '(' . abstract_declarator ')'
#
# Example:
#
# typedef int T;
# void f(void (T));

while (<>) {
    chop;
    if ( /^[Ss]tate ([0-9]+)/ ) {
	$state = $1;
	$_ = <>;
	$_ = <>; chop;
	if ( /[0-9]+ declarator_id: type_name \./ ) {
	    $xxx = $state;
            next;
	}
	next if ( !/[0-9]+ direct_declarator: '\(' \. declarator '\)'/ );
	$_ = <>; chop;
	next if ( !/[0-9]+ direct_abstract_declarator: '\(' \. enter_parameter parameter_declaration_clause leave_parameter '\)' cvr_qualifier_seq exception_specification/ );
	$_ = <>; chop;
	next if ( !/[0-9]+ +| '\(' \. enter_parameter parameter_declaration_clause leave_parameter '\)' exception_specification/);
	$_ = <>; chop;
	next if ( !/[0-9]+ +| '\(' \. enter_parameter parameter_declaration_clause leave_parameter '\)' cvr_qualifier_seq/);
	$_ = <>; chop;
	next if ( !/[0-9]+ +| '\(' \. enter_parameter parameter_declaration_clause leave_parameter '\)'/);
	$_ = <>; chop;
	if ( /[0-9]+ +| '\(' \. abstract_declarator '\)'/) {
	    $yyy = $state;
	}

    }
}

if ($xxx == 0 || $yyy == 0) {
  print STDERR "Error detected at $0\n";
  exit 1;
}

print<<EOF
  if (yystate == $xxx) {
    if (*yyssp == $yyy) {
      using namespace cxx_compiler::parse;
      YYDPRINTF((stderr, "patch.05.2 is applied\\n"));
      restore(&yystate, &yyss, &yyssp, yyssa, &yyvs, &yyvsp, yyvsa);
      ++context_t::retry[$yyy];
      YY_STACK_PRINT(yyss, yyssp);
      goto yynewstate;
    }
  }
EOF


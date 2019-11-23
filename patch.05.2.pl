#State AAA
#
# R1 declarator_id: type_name .
#
#State BBB
#
# R2 declarator_id: nested_name_specifier type_name .
#
#State YYY
#
# R3 ptr_operator: nested_name_specifier . '*' cvr_qualifier_seq
# R4             | nested_name_specifier . '*'
# R5 declarator_id: nested_name_specifier . type_name
# R6 qualified_id: nested_name_specifier . TEMPLATE_KW unqualified_id
# R7             | nested_name_specifier . unqualified_id
#
#State CCC
#
# R8 declarator_id: COLONCOLON_MK move_to_root type_name .
#
#state ZZZ
#
# R1 ptr_operator: COLONCOLON_MK . move_to_root nested_name_specifier '*' cvr_qualifier_seq
# R2             | COLONCOLON_MK . move_to_root nested_name_specifier '*'
# R3 declarator_id: COLONCOLON_MK . move_to_root nested_name_specifier type_name
# R4              | COLONCOLON_MK . move_to_root type_name
# R5 qualified_id: COLONCOLON_MK . move_to_root nested_name_specifier TEMPLATE_KW unqualified_id
# R6             | COLONCOLON_MK . move_to_root nested_name_specifier unqualified_id
# R7             | COLONCOLON_MK . move_to_root IDENTIFIER_LEX
# R8             | COLONCOLON_MK . move_to_root operator_function_id
# R9             | COLONCOLON_MK . move_to_root template_id
#
#State UUU
#
# R1 ptr_operator: COLONCOLON_MK move_to_root . nested_name_specifier '*' cvr_qualifier_seq
# R2             | COLONCOLON_MK move_to_root . nested_name_specifier '*'
# R3 declarator_id: COLONCOLON_MK move_to_root . nested_name_specifier type_name
# R4              | COLONCOLON_MK move_to_root . type_name
# R5 qualified_id: COLONCOLON_MK move_to_root . nested_name_specifier TEMPLATE_KW unqualified_id
# R6             | COLONCOLON_MK move_to_root . nested_name_specifier unqualified_id
# R7             | COLONCOLON_MK move_to_root . IDENTIFIER_LEX
# R9             | COLONCOLON_MK move_to_root . operator_function_id
# R10            | COLONCOLON_MK move_to_root . template_id
#
#State DDD
#
# R9 declarator_id: COLONCOLON_MK move_to_root nested_name_specifier type_name .
#
#State VVV
#
# R1 ptr_operator: COLONCOLON_MK move_to_root nested_name_specifier . '*' cvr_qualifier_seq
# R2             | COLONCOLON_MK move_to_root nested_name_specifier . '*'
# R3 declarator_id: COLONCOLON_MK move_to_root nested_name_specifier . type_name
# R4 qualified_id: COLONCOLON_MK move_to_root nested_name_specifier . TEMPLATE_KW unqualified_id
# R5             | COLONCOLON_MK move_to_root nested_name_specifier . unqualified_id
#
#
#State XXX
#
# R10 direct_declarator: '(' . declarator ')'
# R11 direct_abstract_declarator: '(' . enter_parameter parameter_declaration_clause leave_parameter ')' cvr_qualifier_seq exception_specification
# R12                           | '(' . enter_parameter parameter_declaration_clause leave_parameter ')' exception_specification
# R13                           | '(' . enter_parameter parameter_declaration_clause leave_parameter ')' cvr_qualifier_seq
# R14                           | '(' . enter_parameter parameter_declaration_clause leave_parameter ')'
# R15                           | '(' . abstract_declarator ')'
#
# Example:
#
# typedef int T;
# void f(void (T));
#
# struct outer { struct inner {}; };
# void f(void (outer::inner));

while (<>) {
    chop;
    if ( /^[Ss]tate ([0-9]+)/ ) {
	$state = $1;
	$_ = <>;
	$_ = <>; chop;
	if ( /[0-9]+ declarator_id: type_name \./ ) {
	    $aaa = $state;
            next;
	}
	if ( /[0-9]+ declarator_id: nested_name_specifier type_name \./ ) {
	    $bbb = $state;
            next;
	}
	if (/[0-9]+ declarator_id: COLONCOLON_MK move_to_root type_name \./) {
	    $ccc = $state;
            next;
	}
	if (/[0-9]+ declarator_id: COLONCOLON_MK move_to_root nested_name_specifier type_name \./) {
	    $ddd = $state;
            next;
	}
	if (/ptr_operator: nested_name_specifier \. '\*' cvr_qualifier_seq/) {
	    $_ = <>; chop;
	    if (/| nested_name_specifier \. '\*'/) {
		$_ = <>; chop;
		if (/declarator_id: nested_name_specifier \. type_name/) {
		    $yyy = $state;
		    next;
		}
	    }
	}
	if (/ptr_operator: COLONCOLON_MK \. move_to_root nested_name_specifier '\*' cvr_qualifier_seq/) {
	    $_ = <>; chop;
	    if (/| COLONCOLON_MK \. move_to_root nested_name_specifier '\*'/) {
		$_ = <>; chop;
		if (/declarator_id: COLONCOLON_MK \. move_to_root nested_name_specifier type_name/) {
		    $zzz = $state;
		    next;
		}
	    }
	}
	if (/ptr_operator: COLONCOLON_MK move_to_root \. nested_name_specifier '\*' cvr_qualifier_seq/) {
	    $_ = <>; chop;
	    if (/| COLONCOLON_MK move_to_root \. nested_name_specifier '\*'/) {
		$_ = <>; chop;
		if (/declarator_id: COLONCOLON_MK move_to_root \. nested_name_specifier type_name/) {
		    $uuu = $state;
		    next;
		}
	    }
	}
	if (/ptr_operator: COLONCOLON_MK move_to_root nested_name_specifier \. '\*' cvr_qualifier_seq/) {
	    $_ = <>; chop;
	    if (/| COLONCOLON_MK move_to_root nested_name_specifier \. '\*'/) {
		$_ = <>; chop;
		if (/declarator_id: COLONCOLON_MK move_to_root nested_name_specifier \. type_name/) {
		    $vvv = $state;
		    next;
		}
	    }
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
	    $xxx = $state;
	}

    }
}

if ($aaa == 0 || $bbb == 0 || $ccc == 0 || $ddd == 0 ||
    $xxx == 0 || $yyy == 0 || $zzz == 0 || $uuu == 0 || $vvv == 0) {
  print STDERR "Error detected at $0\n";
  exit 1;
}

print<<EOF
  if (yystate == $aaa) {
    if (*yyssp == $xxx) {
      using namespace cxx_compiler::parse;
      YYDPRINTF((stderr, "patch.05.2 is applied\\n"));
      restore(&yystate, &yyss, &yyssp, yyssa, &yyvs, &yyvsp, yyvsa, false);
      ++context_t::retry[$xxx];
      YY_STACK_PRINT(yyss, yyssp);
      goto yynewstate;
    }
  }
  if (yystate == $bbb) {
    if (*yyssp == $yyy && *(yyssp-1) == $xxx) {
      using namespace cxx_compiler;
      using namespace parse;
      class_or_namespace_name::after(false);
      YYDPRINTF((stderr, "patch.05.2 is applied\\n"));
      restore(&yystate, &yyss, &yyssp, yyssa, &yyvs, &yyvsp, yyvsa, false);
      ++context_t::retry[$xxx];
      YY_STACK_PRINT(yyss, yyssp);
      goto yynewstate;
    }
  }
  if (yystate == $ccc) {
      if (*yyssp == $uuu && *(yyssp-1) == $zzz && *(yyssp-2) == $xxx) {
	  using namespace cxx_compiler;
	  using namespace parse;
	  class_or_namespace_name::after(false);
	  YYDPRINTF((stderr, "patch.05.2 is applied\\n"));
	  restore(&yystate, &yyss, &yyssp, yyssa, &yyvs, &yyvsp, yyvsa, false);
	  ++context_t::retry[$xxx];
	  YY_STACK_PRINT(yyss, yyssp);
	  goto yynewstate;
      }
  }
  if (yystate == $ddd) {
      if (*yyssp == $vvv && *(yyssp-1) == $uuu && *(yyssp-2) == $zzz && *(yyssp-3) == $xxx) {
	  using namespace cxx_compiler;
	  using namespace parse;
	  class_or_namespace_name::after(false);
	  YYDPRINTF((stderr, "patch.05.2 is applied\\n"));
	  restore(&yystate, &yyss, &yyssp, yyssa, &yyvs, &yyvsp, yyvsa, false);
	  ++context_t::retry[$xxx];
	  YY_STACK_PRINT(yyss, yyssp);
	  goto yynewstate;
      }
  }
EOF


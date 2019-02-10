#State XXX
#
#  AAA declarator: direct_declarator .
#  BBB direct_declarator: direct_declarator . '(' enter_parameter parameter_declaration_clause leave_parameter ')' cvr_qualifier_seq exception_specification
#  CCC                  | direct_declarator . '(' enter_parameter parameter_declaration_clause leave_parameter ')' exception_specification
#  DDD                  | direct_declarator . '(' enter_parameter parameter_declaration_clause leave_parameter ')' cvr_qualifier_seq
#  EEE                  | direct_declarator . '(' enter_parameter parameter_declaration_clause leave_parameter ')'
#  FFF                  | direct_declarator . begin_array assignment_expression end_array
#  GGG                  | direct_declarator . begin_array end_array
#  HHH                  | direct_declarator . begin_array '*' end_array
#

while ( <> ){
    chop;
    next if ( !/^[Ss]tate (.*)/ );
    $xxx = $1;
    $_ = <>;
    $_ = <>; chop;
    if ( !/([0-9]+) declarator: direct_declarator \./ ){
	next;
    }
    $aaa = $1;
    $_ = <>; chop;
    if ( !/direct_declarator: direct_declarator \. \'\(\' enter_parameter parameter_declaration_clause leave_parameter \'\)\' cvr_qualifier_seq exception_specification/ ){
	next;
    }
    $_ = <>; chop;
    if ( !/| direct_declarator \. \'\(\' enter_parameter parameter_declaration_clause leave_parameter \'\)\' exception_specification/ ){
	next;
    }
    $_ = <>; chop;
    if ( !/| direct_declarator \. \'\(\' enter_parameter parameter_declaration_clause leave_parameter \'\)\' cvr_qualifier_seq/ ){
	next;
    }
    $_ = <>; chop;
    if ( !/| direct_declarator \. \'\(\' enter_parameter parameter_declaration_clause leave_parameter \'\)\'/ ){
	next;
    }
    $_ = <>; chop;
    if ( !/| direct_declarator \. begin_array assignment_expression end_array/ ){
	next;
    }
    $_ = <>; chop;
    if ( !/| direct_declarator \. begin_array end_array/ ){
	next;
    }
    $_ = <>; chop;
    if ( !/| direct_declarator \. begin_array '*' end_array/ ){
	next;
    }
    goto label;
}

label:
print <<EOF
  if ( yystate == $xxx ){
    using namespace std;
    int n = (cxx_compiler_char == YYEMPTY) ? cxx_compiler::parse::peek() : cxx_compiler_char;
    if ( n == '(' ){
      YYDPRINTF((stderr, "rule.10 is applied\\n"));
      cxx_compiler::parse::identifier::flag = cxx_compiler::parse::identifier::look;
      n = cxx_compiler::parse::get_token();
      if ( n == INTEGER_LITERAL_LEX ){
        cxx_compiler::parse::g_read.m_token.push_front(make_pair(n,cxx_compiler::parse::position));
		cxx_compiler::parse::g_read.m_lval.push_front(cxx_compiler_lval.m_usr);
        yyn = 133 + 1;
        goto yyreduce;
      }
      else {
        cxx_compiler::parse::g_read.m_token.push_front(make_pair(n,cxx_compiler::parse::position));
		switch ( n ) {
		case IDENTIFIER_LEX:
		case INTEGER_LITERAL_LEX:
		case CHARACTER_LITERAL_LEX:
		case FLOATING_LITERAL_LEX:
		case TYPEDEF_NAME_LEX:
		case STRING_LITERAL_LEX:
		case CLASS_NAME_LEX:
		case ENUM_NAME_LEX:
		case DEFAULT_KW:
		case ORIGINAL_NAMESPACE_NAME_LEX:
		case NAMESPACE_ALIAS_LEX:
			cxx_compiler::parse::g_read.m_lval.push_front(cxx_compiler_lval.m_usr);
			break;
		}
      }
    }
  }
EOF

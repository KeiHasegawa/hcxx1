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
#  Example:
#
#  struct T { T(int); };
#
#  void f(int a)
#  {
#    T t1();   // function declaration
#    T t2(a);  // variable definition
#  }
#
#  Example 2 of '-2' option
#
#  int b();
#  void f(int a)
#  {
#     int(b())+a;  // expression statement
#                  // at reading '+',  int(b()) is not a declaration.
#  }
#
use Getopt::Long;

$opt_header = 0;

GetOptions('header' => \$opt_header, '2' => \$opt_2);

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
    if ($opt_header) {
	goto label_header;
    }
    if ($opt_2) {
	goto label_2;
    }
    goto label;
}

print STDERR "Error detected at $0\n";
exit 1;

label:
print <<EOF
  if (yystate == $xxx) {
    using namespace cxx_compiler::parse;
    switch (yychar) {
    case '(': case '[':
      if (!context_t::retry[$xxx])
        save(yystate, yyss, yyssp, yyvs, yyvsp);
      else {
        YYDPRINTF((stderr, "rule.10 is applied\\n"));
        yyn = $aaa + 1;
        goto yyreduce;
      }
    }
  }
EOF
    ;
exit;

label_headr;
print <<EOF_header
const int DECLARATOR_ID_CONFLICT_STATE = $xxx ;
EOF_header
    ;
exit; 

label_2:
print <<EOF_2
  if (yystate == $xxx) {
    using namespace cxx_compiler::parse;
    switch (yychar) {
    case '(': case '[': case '=': case ';': case ')': case '{':
    case ',': case ':': case THROW_KW:
      break; 
    default:
      if (!context_t::all.empty()) {
	context_t& x = context_t::all.back();
	if (x.m_state == $xxx) {
          YYDPRINTF((stderr, "rule.10.2 is applied\\n"));
          context_t::all.pop_back();
          goto yyerrlab;
	}
      }
    }
  }
EOF_2
    ;
exit; 

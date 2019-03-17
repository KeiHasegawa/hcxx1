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
use Getopt::Long;

$opt_header = 0;

GetOptions('header' => \$opt_header);

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
	goto label2;
    }
    goto label;
}

print STDERR "Error detected at $0\n";
exit 1;

label:
print <<EOF
  if (yystate == $xxx && yychar == '(') {
    using namespace cxx_compiler::parse;
    if (!context_t::retry[$xxx])
      save(yystate, yyss, yyssp, yyvs, yyvsp);
    else {
      YYDPRINTF((stderr, "rule.10 is applied\\n"));
      yyn = $aaa + 1;
      goto yyreduce;
    }
  }
EOF
    ;
exit;

label2:
print <<EOF2
const int DECLARATOR_ID_CONFLICT_STATE = $xxx ;
EOF2

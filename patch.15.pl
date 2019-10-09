# See also patch.04.pl
#
#State XXX
#
#  AAA simple_type_specifier: nested_name_specifier type_name .
#  BBB declarator_id: nested_name_specifier type_name .
#
# Example:
#
# template<class T> struct S { struct X { /* ... */ }; S(); /* ... */ };
# S<int>::X (a);  // use AAA. variable declaration.
# template<class T> S<T>::S() { /* ... */ };    // use BBB
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
    if ( !/([0-9]+) simple_type_specifier: nested_name_specifier type_name \./ ){
	next;
    }
    $aaa = $1;
    $_ = <>; chop;
    if ( !/([0-9]+) declarator_id: nested_name_specifier type_name \./ ){
	next;
    }
    $bbb = $1;
    if ($opt_header) {
	goto label2;
    }
    if ($opt_2) {
	goto label3;
    }
    goto label;
}

print STDERR "Error detected at $0\n";
exit 1;

label:
print <<EOF
  if (yystate == $xxx && yychar == '(') {
    using namespace cxx_compiler;
    if (scope::current->m_id == scope::TAG) {
      if (!parse::context_t::retry[$xxx]) {
        parse::identifier::mode = parse::identifier::canbe_ctor;
        parse::save(yystate, yyss, yyssp, yyvs, yyvsp);
      }
      else {
        YYDPRINTF((stderr, "patch.15 is applied\\n"));
        yyn = $bbb + 1;
        goto yyreduce;
      }
    }
  }
EOF
    ;
exit;

label2:
print <<EOF2
const int NESTED_TYPE_NAME_CONFLICT_STATE = $xxx ;
EOF2
    ;
exit;

label3:
print <<EOF3
    {
      using namespace cxx_compiler;
      if (parse::identifier::mode == parse::identifier::canbe_ctor) {
        switch (yychar) {
        case ORIGINAL_NAMESPACE_NAME_LEX:
        case NAMESPACE_ALIAS_LEX:
        case TYPEDEF_NAME_LEX:
        case CLASS_NAME_LEX:
        case TEMPLATE_NAME_LEX:
        case ENUM_NAME_LEX:
          assert(!parse::context_t::all.empty());
          parse::restore(&yystate, &yyss, &yyssp, yyssa, &yyvs, &yyvsp, yyvsa);
          parse::context_t::retry[$xxx] = true;
          YYDPRINTF((stderr, "retry!\\n"));
          YY_STACK_PRINT(yyss, yyssp);
          goto yynewstate;
	}
      }
    }
EOF3

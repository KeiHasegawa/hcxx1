#State XXX
#
#  AAA simple_type_specifier: type_name .
#  BBB declarator_id: type_name .
#
#  Example:
#
#  struct T {
#    T();     // constructor declaration
#             // use declarator_id -> type_name
#  };
#
#  T (a);     // variable declaration
#             // use simple_type_specifier -> type_name
#
#  T();       // temporary object
#             // use simple_type_specifier -> type_name

while ( <> ){
    chop;
    next if ( !/^[Ss]tate (.*)/ );
    $xxx = $1;
    $_ = <>;
    $_ = <>; chop;
    if ( !/([0-9]+) simple_type_specifier: type_name \./ ){
	next;
    }
    $aaa = $1;
    $_ = <>; chop;
    if ( !/([0-9]+) declarator_id: type_name \./ ){
	next;
    }
    $bbb = $1;
    goto label;
}

print STDERR "Error detected at $0\n";
exit 1;

label:
print <<EOF
  if (yystate == $xxx && yychar == '(') {
    using namespace cxx_compiler;
    if (scope::current->m_id == scope::TAG) {
      if (!cxx_compiler::parse::context_t::retry[$xxx])
        cxx_compiler::parse::save(yystate, yyss, yyssp, yyvs, yyvsp);
      else {
        YYDPRINTF((stderr, "rule.04 is applied\\n"));
        yyn = $bbb + 1;
        goto yyreduce;
      }
    }
  }
EOF
    ;
exit;

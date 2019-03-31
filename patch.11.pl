#state XXX
#
#  AAA simple_type_specifier: nested_name_specifier type_name .
#  BBB declarator_id: nested_name_specifier type_name .
#
#  struct outer {
#    struct inner {
#    };
#    outer();
#  };
#
#  outer::inner x;     // outer::inner is reduced using rule AAA
#  outer::outer(){}    // outer::outer is reduced using BBB
#
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
    goto label;
}

print STDERR "Error detected at $0\n";
exit 1;

label:
print <<EOF
  if (yystate == $xxx) {
    using namespace cxx_compiler;
    using namespace declarations;
    if (scope::current->m_id == scope::TAG) {
      tag* x = static_cast<tag*>(scope::current);
      type_specifier* spec = yyvsp[0].m_type_specifier;
      const type* T = spec->m_type;
      if (T && T->m_id == type::RECORD) {
        typedef const record_type REC;
        REC* rec = static_cast<REC*>(T);
        tag* y = rec->get_tag();
        if (x == y) {
          YYDPRINTF((stderr, "rule.11 is applied\\n"));
          yyn = $bbb + 1;
        }
      }
    }
  }
EOF
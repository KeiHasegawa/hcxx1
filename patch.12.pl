#state XXX
#
#  XX1 ptr_operator: nested_name_specifier . '*' cvr_qualifier_seq
#  XX2             | nested_name_specifier . '*'
#  XX3 declarator_id: nested_name_specifier . type_name
#  XX4 qualified_id: nested_name_specifier . TEMPLATE_KW unqualified_id
#  XX5             | nested_name_specifier . unqualified_id
#
#state YYY
#
#  YY1 simple_type_specifier: nested_name_specifier . type_name
#  YY2                      | nested_name_specifier . TEMPLATE_KW template_id
#  YY3 qualified_id: nested_name_specifier . TEMPLATE_KW unqualified_id
#  YY4             | nested_name_specifier . unqualified_id
#
#  Example:
#
#  struct B { void f(); };
#  void g(void (B::*pf)()){ }
#

while ( <> ) {
    chop;
    if ( /^[Ss]tate (.*)/ ){
        $state = $1;
	$_ = <>;
	$_ = <>;
	chop;
	if (/ptr_operator: nested_name_specifier \. '\*' cvr_qualifier_seq/) {
	    $_ = <>;
	    chop;
	    if (!/| nested_name_specifier \. '\*'/) {
		next;
	    }
	    $_ = <>;
	    chop;
	    if (!/declarator_id: nested_name_specifier \. type_name/){
		next;
	    }
	    $xxx = $state;
	}
	elsif (/simple_type_specifier: nested_name_specifier \. type_name/) {
	    $yyy = $state;
	}
    }
}

if ($xxx == 0 || $yyy == 0) {
  print STDERR "Error detected at $0\n";
  exit 1;
}

print<<EOF
  if (yystate == $yyy) {
    if (yychar == '*') {
      YYDPRINTF((stderr, "rule.12 is applied\\n"));
      using namespace cxx_compiler::parse;
      identifier::mode = identifier::new_obj;
      yystate = $xxx;
    }
  }
EOF


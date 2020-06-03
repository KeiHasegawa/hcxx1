#State XXX
#
#  OOO init_declarator: declarator . $@6 initializer
#  AAA                | declarator .
#  PPP function_definition_begin1: declarator .
#  QQQ function_definition_begin2: declarator .
#
#State YYY
#
#  RRR init_declarator: declarator . $@6 initializer
#  AAA               | declarator .
#  SSS function_definition_begin1: decl_specifier_seq declarator .
#  TTT function_definition_begin2: decl_specifier_seq declarator .
#
#  Example:
#
#  struct S {
#    template<class C> S(C x){ /* ... */ }
#    template<class C> void f(C x){ /* ... */ }
#  };

while ( <> ){
    chop;
    next if ( !/^[Ss]tate (.*)/ );
    $state = $1;
    $_ = <>;
    $_ = <>; chop;
    next if (!/([0-9]+) init_declarator: declarator \./);
    $_ = <>; chop;
    next if (!/([0-9]+) + | declarator \./);
    $aaa = $1;
    $_ = <>; chop;
    if (/function_definition_begin1: decl_specifier_seq declarator \./) {
      $_ = <>; chop;
      if (/function_definition_begin2: decl_specifier_seq declarator \./) {
        $xxx = $state;
        $aax = $aaa;
        next;
      }
    }

    if (/function_definition_begin1: declarator \./) {
      $_ = <>; chop;
      if (/function_definition_begin2: declarator \./) {
        $yyy = $state;
        $aay = $aaa;
        next;
      }
    }
}

if ($xxx == 0 || $yyy == 0 || $aaa == 0 || $aax != $aay) {
  print STDERR "Error detected at $0\n";
  exit 1;
}

print <<EOF
  if (yystate == $xxx || yystate == $yyy) {
    if (yychar == TRY_KW || yychar == ':' || yychar == '{') {
      using namespace cxx_compiler;
      if (scope::current->m_id == scope::TAG) {
        tag* ptr = static_cast<tag*>(scope::current);
        const type* T = ptr->m_types.second;
        if (!T) {
          using namespace parse;
          if (!templ::save_t::nest.empty()) {
            YYDPRINTF((stderr, "patch.13.2 is applied\\n"));
	    templ::save_t* p = templ::save_t::nest.back();
	    p->m_patch_13_2 = true;
	    identifier::mode = identifier::peeking;
	    member_function_body::save_brace(&p->m_read);
	    identifier::mode = identifier::look;
            yychar = ';';
            yyn = $aaa + 1;
            goto yyreduce;
          }
	}
      }
    }
  }
EOF

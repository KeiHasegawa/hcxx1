#State XXX
#
#  AAA member_declarator: declarator .
#  BBB                  | declarator . $@8 constant_initializer
#  CCC function_definition_begin1: decl_specifier_seq declarator .
#  DDD function_definition_begin2: decl_specifier_seq declarator .
#
#State YYY
#
#  AAA member_declarator: declarator .
#  BBB                  | declarator . $@8 constant_initializer
#  EEE function_definition_begin1: declarator .
#  FFF function_definition_begin2: declarator .
#
#  Example:
#
#  struct S {
#    S(int ii, double dd) : i(ii), d(dd) {}
#    int i;
#    double d;
#  };

while ( <> ){
    chop;
    next if ( !/^[Ss]tate (.*)/ );
    $state = $1;
    $_ = <>;
    $_ = <>; chop;
    next if (!/([0-9]+) member_declarator: declarator \./);
    $aaa = $1;
    $_ = <>; chop;
    next if (!/| declarator \. .* constant_initializer/);
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
      YYDPRINTF((stderr, "patch.13 is applied\\n"));
      using namespace cxx_compiler::parse;
      member_function_body::save(yyvsp[0].m_usr);
      yychar = ';';
      yyn = $aaa + 1;
      goto yyreduce;
    }
  }
EOF

#State XXX
#
#  AAA init_declarator: declarator . $@7 initializer
#  BBB                | declarator .
#  CCC function_definition_begin1: decl_specifier_seq declarator .
#  DDD function_definition_begin2: decl_specifier_seq declarator .
#
#    TRY_KW    reduce using rule DDD (function_definition_begin2)
#    ';'       reduce using rule BBB (init_declarator)
#    ','       reduce using rule BBB (init_declarator)
#    '{'       reduce using rule AAA ($@7)
#    '{'       [reduce using rule CCC (function_definition_begin1)]
#    ':'       reduce using rule CCC (function_definition_begin1)
#    $default  reduce using rule AAA ($@7)
#
while (<>) {
    chop;
    next if ( !/^[Ss]tate (.*)/ );
    $xxx = $1;
    $_ = <>;
    $_ = <>; chop;
    next if ( !/([0-9]+) init_declarator: declarator \. .* initializer/ );
    $aaa = $1;
    $_ = <>; chop; 
    next if ( !/([0-9]+) .* declarator \./ );
    $bbb = $1;
    $_ = <>; chop; 
    next if ( !/([0-9]+) function_definition_begin1: decl_specifier_seq declarator \./ );
    $ccc = $1;
    $_ = <>; chop; 
    next if ( !/([0-9]+) function_definition_begin2: decl_specifier_seq declarator \./ );
    $ddd = $1;
    goto label;
}

print STDERR "Error detected at $0\n";
exit 1;

label:
print<<EOF
  if (yystate == $xxx) {
    if (yychar == '{') {
      using namespace cxx_compiler;
      usr* u = yyvsp[0].m_usr;
      const type* T = u->m_type;
      type::id_t id = T->m_id;
      if (id == type::FUNC) {
        YYDPRINTF((stderr, "patch.24 is applied\\n"));
        yyn = $ccc + 1;
      }
    }
  }
EOF


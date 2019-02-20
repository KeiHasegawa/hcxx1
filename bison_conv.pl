print "#pragma warning ( disable : 4065 )\n";
print "#pragma warning ( disable : 4060 )\n";
print "#include \"stdafx.h\"\n";
print "#include \"yy.h\"\n";

$yychar_converted = 0;
$rule08_inserted = 0;

while ( <> ){
  chop;
  s/	/        /;
  s/cxx\.tab\.c/cxx_y.cpp/;
  if ( /yychar = yylex \(\);/ ){
    print "      yychar = cxx_compiler::parse::get_token();\n";
    ++$yychar_converted;
    next;
  }
  if ( /yychar = YYLEX;/ ){
    print "      yychar = cxx_compiler::parse::get_token();\n";
    ++$yychar_converted;
    next;
  }
  print $_,"\n";
  if ( /yystate = yydefgoto/ ){
    print "#include \"rule.00\"\n";
  }
  if ( /yystate = yyn/ ){
    print "#include \"rule.01\"\n";
    print "#include \"rule.02\"\n";
  }
  if ( /goto yydefault;/ ){
    print "#include \"rule.03\"\n";
    print "#include \"rule.09\"\n";
    print "#include \"rule.10\"\n";
  }
  if ( /yyn.*=.*yydefact\[yystate\];/ ){
    print "#include \"rule.06\"\n";
  }
  if ( /yystate = 0/ ){
    print "#include \"rule.07\"\n";
  }
  if ( /YY_SYMBOL_PRINT \(\"-> \$\$ =\", yyr1\[yyn\], &yyval, &yyloc\);/ ){
    print "#include \"rule.08\"\n";
    ++$rule08_inserted;
  }
  if (/^yyerrlab:/) {
      print<<EOF
  if (!cxx_compiler::parse::context_t::all.empty()) {
    cxx_compiler::parse::restore(&yystate, &yyss, &yyssp, yyssa, &yyvs, &yyvsp, yyvsa);
    ++cxx_compiler::parse::context_t::retry[yystate];
    YYDPRINTF((stderr, "retry!\\n"));
    YY_STACK_PRINT(yyss, yyssp);
    goto yynewstate;
  }
EOF
  }
}

if ($yychar_converted != 1) {
    print STDERR "Error detected at $0\n";
    print STDERR '$yychar_converted = ', $yychar_converted, "\n";
    exit 1;
}

if ($rule08_inserted != 1) {
    print STDERR "Error detected at $0\n";
    print STDERR '$rule08_inserted = ', $rule08_inserted, "\n";
    exit 1;
}

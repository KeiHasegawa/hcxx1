print "#pragma warning ( disable : 4065 )\n";
print "#pragma warning ( disable : 4060 )\n";
print "#include \"stdafx.h\"\n";
print "#include \"yy.h\"\n";

$flag = 0;

while ( <> ){
  chop;
  s/	/        /;
  s/cxx\.tab\.c/cxx_y.cpp/;
  if ( /yychar = yylex \(\);/ ){
    print "      yychar = cxx_compiler::parse::get_token();\n";
    ++$flag;
    next;
  }
  if ( /yychar = YYLEX;/ ){
    print "      yychar = cxx_compiler::parse::get_token();\n";
    ++$flag;
    next;
  }
  if ( /  yyvsp -= yylen;/ && !$rule08_done ){
    print " #include \"rule.08\"\n";
    $rule08_done = 1;
  }
  if ( /^yydestruct \(const char \*yymsg, int yytype, YYSTYPE \*yyvaluep\)$/ ){
      $yydestruct = 1;
  }
  print $_,"\n";
  if ( /yystate = 0/ ){
    print "#include \"rule.07\"\n";
  }
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

if ( $flag != 1 ) {
    print STDERR "Error detected at $0\n";
    print STDERR '$flag = ', $flag, "\n";
    exit 1;
}

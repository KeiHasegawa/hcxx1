print <<EOF
  if ( cxx_compiler::parse::backtrack::g_stack.top().m_point == yyssp ) {
      YYDPRINTF((stderr, "rule.12 is applied\\n"));
      yyn = yypact[yystate];
      goto hasegawa_magic;
  }
EOF

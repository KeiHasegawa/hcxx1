print <<EOF
  if ( cxx_compiler::parse::backtrack::g_stack.top().m_point == yyssp ) {
      yyn = yypact[yystate];
      goto hasegawa_magic;
  }
EOF

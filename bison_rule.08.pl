#State XXX
# 
#     YYY declaration: function_definition .

while ( <> ){
    chop;
    next if ( !/^[Ss]tate (.*)/ );
    $xxx = $1;
    $_ = <>;
    $_ = <>; chop;
    next if ( !/declaration: function_definition \./ );
    goto label;
}

label:
print <<EOF
  if ( cxx_compiler::parse::member_function_body::g_restore.m_saved &&
       yystate == $xxx ) {
    YYDPRINTF((stderr "rule.08 is applied\n"));
    return 0;
  }
EOF

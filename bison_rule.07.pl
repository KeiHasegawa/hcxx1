#State XXX
#
#   YYY function_definition: function_definition_begin1 . function_body

while ( <> ){
    chop;
    next if ( !/^[Ss]tate (.*)/ );
    $xxx = $1;
    $_ = <>;
    $_ = <>; chop;
    next if ( !/function_definition: function_definition_begin1 \. function_body/ );
    goto label;
}

label:
print <<EOF
  if ( cxx_compiler::parse::member_function_body::g_restore.m_saved )
    yystate = $xxx;
EOF

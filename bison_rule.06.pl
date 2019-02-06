#State XXX
# 
#   YYY type_name: class_name .
#   ZZZ class_or_namespace_name: class_name .

while ( <> ){
    chop;
    next if ( !/^[Ss]tate (.*)/ );
    $xxx = $1;
    $_ = <>;
    $_ = <>; chop;
    next if ( !/type_name: class_name \./ );
    $_ = <>; chop;
    next if ( !/(.*) class_or_namespace_name: class_name \./ );
    $zzz = $1;
    goto label;
}

label:
print <<EOF
  if ( yystate == $xxx && cxx_compiler::parse::identifier::g_peek_coloncolon )
    yyn = $zzz + 1;
EOF

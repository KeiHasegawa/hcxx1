#State XXX
# 
#   YYY type_name: class_name .
#   ZZZ class_or_namespace_name: class_name .
#
#   Example:
#
#   struct T { void f(); };
#
#   void T::f()
#   {
#     // ...
#   }
#

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
  if (yystate == $xxx && cxx_compiler::parse::peek() == COLONCOLON_MK) {
    YYDPRINTF((stderr, "rule.06 is applied\\n"));
    yyn = $zzz + 1;
  }
EOF

#State XXX
#
#   YYY function_definition: function_definition_begin1 . function_body
#
#   Example:
#
#   struct S {
#     int f(){ A x; x = 1.0; return x + a; }
#     typedef double A;
#     int a;
#   };

while ( <> ){
    chop;
    next if ( !/^[Ss]tate (.*)/ );
    $xxx = $1;
    $_ = <>;
    $_ = <>; chop;
    next if ( !/function_definition: function_definition_begin1 \. function_body/ );
    goto label;
}

print STDERR "Error detected at $0\n";
exit 1;

label:
print <<EOF
  if (cxx_compiler::parse::member_function_body::saved) {
    YYDPRINTF((stderr, "patch.07 is applied\\n"));
    yystate = $xxx;
  }
EOF

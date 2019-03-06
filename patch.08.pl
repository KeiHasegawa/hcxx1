#State XXX
# 
#     YYY declaration: function_definition .
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
    next if ( !/declaration: function_definition \./ );
    goto label;
}

print STDERR "Error detected at $0\n";
exit 1;

label:
print <<EOF
  if (cxx_compiler::parse::member_function_body::saved && yystate == $xxx) {
    YYDPRINTF((stderr, "rule.08 is applied\\n"));
    return 0;
  }
EOF

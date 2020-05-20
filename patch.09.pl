#State XXX
#
#  AAA nested_name_specifier: class_or_namespace_name COLONCOLON_MK . nested_name_specifier
#  BBB                      | class_or_namespace_name COLONCOLON_MK .
#  CCC                      | class_or_namespace_name COLONCOLON_MK . TEMPLATE_KW nested_name_specifier
#
#  Example:
#
#  namespace N {
#    struct outer {
#      struct inner {
#        // ...
#      };
#      // ...
#    };
#  }
#
#  N::outer::inner x;  // outer : shift
#                      // inner : recuded using BBB

while ( <> ){
    chop;
    next if ( !/^[Ss]tate (.*)/ );
    $xxx = $1;
    $_ = <>;
    $_ = <>; chop;
    next if ( !/([0-9]+) nested_name_specifier: class_or_namespace_name COLONCOLON_MK \. nested_name_specifier/ );
    $aaa = $1;
    $_ = <>;    
    next if ( !/([0-9]+) .* class_or_namespace_name COLONCOLON_MK \./ );
    $bbb = $1;
    $_ = <>;    
    next if ( !/([0-9]+) .* class_or_namespace_name COLONCOLON_MK \. TEMPLATE_KW nested_name_specifier/ );
    $ccc = $1;
    goto label;
}

print STDERR "Error detected at $0\n";
print STDERR "aaa = $aaa", "\n";
print STDERR "bbb = $bbb", "\n";
print STDERR "ccc = $ccc", "\n";
exit 1;

label:
print <<EOF
  if (yystate == $xxx) {
    using namespace cxx_compiler::parse;
    if (last_token == TEMPLATE_KW) {
      identifier::mode = identifier::templ_name;
    }
    else if (last_token == TEMPLATE_NAME_LEX) {
      if (!templ_arg_and_coloncolon()) {
        YYDPRINTF((stderr, "patch.09 is applied\\n"));
        yyn = $bbb + 1;
        goto yyreduce;
      }
    }
    else {
      if (peek() != COLONCOLON_MK) {
        YYDPRINTF((stderr, "patch.09 is applied\\n"));
        yyn = $bbb + 1;
        goto yyreduce;
      }
    }
  }
EOF
    ;
exit;

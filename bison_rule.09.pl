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
#  N::outer::inner x;
#

while ( <> ){
    chop;
    next if ( !/^[Ss]tate (.*)/ );
    $xxx = $1;
    $_ = <>;
    $_ = <>; chop;
    if ( !/nested_name_specifier: class_or_namespace_name COLONCOLON_MK \. nested_name_specifier/ ){
	next;
    }
    $_ = <>; chop;
    if ( !/class_or_namespace_name COLONCOLON_MK \./ ){
	next;
    }
    $_ = <>; chop;
    if ( !/class_or_namespace_name COLONCOLON_MK \. TEMPLATE_KW nested_name_specifier/ ){
	next;
    }
    goto label;
}

label:
print <<EOF
  if (yystate == $xxx) {
    if (cxx_compiler::parse::peek() != COLONCOLON_MK) {
      YYDPRINTF((stderr, "rule.09 is applied\\n"));
      goto yydefault;
    }
  }
EOF




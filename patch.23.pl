#State XXX
#
#  AAA type_specifier_seq: type_specifier . type_specifier_seq
#  BBB                   | type_specifier .
#
#  Example:
#
#  template<class C1, class C2> S<C1 C2::*> { /* ... */ };
#
#  For `C2', shift action is not correct. reduce action is correct.
while ( <> ){
    chop;
    next if ( !/^[Ss]tate (.*)/ );
    $xxx = $1;
    $_ = <>;
    $_ = <>; chop;
    next if ( !/([0-9]+) type_specifier_seq: type_specifier \. type_specifier_seq/ );
    $aaa = $1;
    $_ = <>;    
    next if ( !/([0-9]+) .* type_specifier \./ );
    $bbb = $1;
    goto label;
}

print STDERR "Error detected at $0\n";
print STDERR "aaa = $aaa", "\n";
print STDERR "bbb = $bbb", "\n";
exit 1;

label:
print <<EOF
  if (yystate == $xxx) {
    if (yychar == CLASS_NAME_LEX) {
      using namespace cxx_compiler;
      if (parse::peek() == COLONCOLON_MK) {
        YYDPRINTF((stderr, "patch.23 is applied\\n"));
        yyn = $bbb + 1;
        goto yyreduce;
      }
    }
  }
EOF
    ;
exit;

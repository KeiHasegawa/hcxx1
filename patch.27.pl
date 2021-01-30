#State XXX
#
#  AAA class_key: CLASS_KW .
#  BBB type_parameter: CLASS_KW . IDENTIFIER_LEX
#  CCC               | CLASS_KW . DOTS_MK IDENTIFIER_LEX
#  DDD               | CLASS_KW .
#  EEE               | CLASS_KW . DOTS_MK
#  FFF               | CLASS_KW . IDENTIFIER_LEX '=' $@17 type_id
#  GGG               | CLASS_KW . '=' $@18 type_id
while (<>) {
    chop;
    next if ( !/^[Ss]tate (.*)/ );
    $xxx = $1;
    $_ = <>;
    $_ = <>; chop;
    next if (!/([0-9]+) class_key: CLASS_KW \./);
    $aaa = $1;
    $_ = <>; chop;
    next if (!/([0-9]+) type_parameter: CLASS_KW \. IDENTIFIER_LEX/);
    $bbb = $1;
    goto label;
}

print STDERR "Error detected at $0\n";
exit 1;

label:
print<<EOF
  if (yystate == $xxx) {
    using namespace cxx_compiler::parse;
    YYDPRINTF((stderr, "patch.27 is applied\\n"));
    identifier::mode = identifier::new_obj;
  }
EOF

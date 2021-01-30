#State XXX
#
#  AAA elaborated_type_specifier: typenaming . COLONCOLON_MK move_to_root nested
#                               ...
#  BBB                          | typenaming . nested_name_specifier template_id
#  CCC type_parameter: typenaming . IDENTIFIER_LEX
#  DDD               | typenaming . DOTS_MK IDENTIFIER_LEX
#  EEE               | typenaming .
#  FFF               | typenaming . DOTS_MK
#  GGG               | typenaming . IDENTIFIER_LEX '=' $@19 type_id
#  HHH               | typenaming . '=' $@20 type_id
while (<>) {
    chop;
    next if ( !/^[Ss]tate (.*)/ );
    $xxx = $1;
    $_ = <>;
    $_ = <>; chop;
    next if (!/([0-9]+) elaborated_type_specifier: typenaming \. COLONCOLON_MK move_to_root nested/);
    $aaa = $1;
    while (<>) {
	chop;
	next if (/([0-9]+) +\| typenaming \. /);
	goto label2;
    }
label2:
    next if (!/([0-9]+) type_parameter: typenaming \. IDENTIFIER_LEX/);
    $ccc = $1;
    goto label;
}

print STDERR "Error detected at $0\n";
exit 1;

label:
print<<EOF
  if (yystate == $xxx) {
    using namespace cxx_compiler::parse;
    YYDPRINTF((stderr, "patch.28 is applied\\n"));
    identifier::mode = identifier::new_obj;
  }
EOF

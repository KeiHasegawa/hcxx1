#state XXX
#
#  AAA direct_abstract_declarator: '(' . enter_parameter parameter_declaration_clause leave_parameter ')' cvr_qualifier_seq exception_specification
#  BBB                           | '(' . enter_parameter parameter_declaration_clause leave_parameter ')' exception_specification
# ...
#    CLASS_NAME_LEX               shift, and go to state YYY
# ...
#    CLASS_NAME_LEX               [reduce using rule RRR (enter_parameter)]
# ...
#
#  Example
#
#  template<class> S;
#  template<class C> struct S<int(C)> { /* ... */ };
#
use Getopt::Long;

$opt_2 = 0;

GetOptions('2' => \$opt_2);

while (<>) {
    chop;
    next if ( !/^[Ss]tate (.*)/ );
    $xxx = $1;
    $_ = <>;
    $_ = <>; chop;
    next if (!/([0-9]+) direct_abstract_declarator: '\(' \./);
    if (!$opt_2) {
	goto label;
    }
    while (<>) {
	chop;
	next if (!/CLASS_NAME_LEX.*reduce using rule ([0-9]+) .*enter_parameter/);
	$rrr = $1;
	goto label2;	
    }
}

print STDERR "Error detected at $0\n";
exit 1;

label:
print<<EOF
  if (yystate == $xxx) {
    using namespace cxx_compiler::parse;
    YYDPRINTF((stderr, "patch.29 is applied\\n"));
    identifier::mode = identifier::look;
  }
EOF
    ;
exit;

label2:
print <<EOF2
  if (yystate == $xxx) {
      switch(yychar) {
	case ORIGINAL_NAMESPACE_NAME_LEX:
	case NAMESPACE_ALIAS_LEX:
	case CLASS_NAME_LEX:
	case TEMPLATE_NAME_LEX:
	case COLONCOLON_MK:
          YYDPRINTF((stderr, "patch.29.2 is applied\\n"));
	  yyn = $rrr + 1;
	  goto yyreduce;
        default:
          break;
      }
  }
EOF2



#State XXX
#
#   YYY type_specifier: simple_type_specifier .
#   ZZZ postfix_expression: simple_type_specifier . '(' expression_list ')'
#   WWW                   | simple_type_specifier . '(' ')'
#
#   Example:
#
#   void f()
#   {
#     int (n);
#     int (n+n);
#   }
#

use Getopt::Long;

$opt_header = 0;

GetOptions('header' => \$opt_header);

while ( <> ){
    chop;
    next if ( !/^[Ss]tate (.*)/ );
    $xxx = $1;
    $_ = <>;
    $_ = <>; chop;
    next if ( !/([0-9]+) type_specifier: simple_type_specifier/ );
    $yyy = $1;
    $_ = <>; chop;
    next if ( !/postfix_expression: simple_type_specifier/ );
    $_ = <>; chop;
    next if ( !/| simple_type_specifier/ );
    if ($opt_header) {
	goto label2;
    }
    goto label;
}

print STDERR "Error detected at $0\n";
exit 1;

label:
print <<EOF
  if (yystate == $xxx && yychar == '(') {
    using namespace cxx_compiler::parse;
    if (!context_t::retry[$xxx]) {
      YYDPRINTF((stderr, "patch.03 is applied\\n"));
      save(yystate, yyss, yyssp, yyvs, yyvsp);
      yyn = $yyy + 1;
      goto yyreduce;
    }
    identifier::mode = identifier::look;
  }
EOF
    ;
exit;

label2:
print <<EOF2
const int DECL_FCAST_CONFLICT_STATE = $xxx ;
EOF2

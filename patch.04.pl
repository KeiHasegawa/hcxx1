#State XXX
#
#  AAA simple_type_specifier: type_name .
#  BBB declarator_id: type_name .
#
#  Example:
#
#  struct T {
#    T();     // constructor declaration
#             // use declarator_id -> type_name
#  };
#
#  T (a);     // variable declaration
#             // use simple_type_specifier -> type_name
#
#  T();       // temporary object
#             // use simple_type_specifier -> type_name
#
#  struct S { 
#    S(
#      S&);   // `S' is not lookuped at 1st time but lookuped at retry time
#  };
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
    if ( !/([0-9]+) simple_type_specifier: type_name \./ ){
	next;
    }
    $aaa = $1;
    $_ = <>; chop;
    if ( !/([0-9]+) declarator_id: type_name \./ ){
	next;
    }
    $bbb = $1;
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
    using namespace cxx_compiler;
    if (scope::current->m_id == scope::TAG) {
      if (!parse::context_t::retry[$xxx])
        parse::save(yystate, yyss, yyssp, yyvs, yyvsp);
      else {
        YYDPRINTF((stderr, "patch.04 is applied\\n"));
        yyn = $bbb + 1;
        goto yyreduce;
      }
    }
  }
EOF
    ;
exit;

label2:
print <<EOF2
const int TYPE_NAME_CONFLICT_STATE = $xxx ;
EOF2

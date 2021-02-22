#State XXX
#
#  AAA class_name: template_id .
#  BBB unqualified_id: template_id .
#
#  Example:
#
#  template<class T> T* f(){ /* ... */ }
#
#  int* p = f<int>();  // use BBB rule
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
    next if (!/([0-9]+) class_name: template_id \./);
    $aaa = $1;
    $_ = <>; chop;
    next if (!/([0-9]+) unqualified_id: template_id \./);
    $bbb = $1;
    if ($opt_2) {
	goto label2;
    }
    goto label;
}

print STDERR "Error detected at $0\n";
exit 1;

label:
print<<EOF
  if (yystate == $xxx) {
    using namespace std;
    using namespace cxx_compiler;
    pair<usr*, tag*>* ut = yyvsp[0].m_ut;
    if (usr* u = ut->first) {
      usr::flag_t flag = u->m_flag;
      usr::flag2_t flag2 = u->m_flag2;
      if (!(flag & usr::TYPEDEF) && !(flag2 & usr::ALIAS)) {
	if (!(flag2 & usr::TEMPLATE) ||
	    !static_cast<template_usr*>(u)->m_express_type) {
          YYDPRINTF((stderr, "patch.15 is applied\\n"));
          yyn = $bbb + 1;
	}
      }
    }
  }
EOF
    ;
exit;

label2:
print <<EOF2
  if (yystate == $xxx) {
    using namespace std;
    using namespace cxx_compiler;
    pair<usr*, tag*>* p = yyvsp[0].m_ut;
    if (usr* u = p->first) {
      usr::flag_t flag = u->m_flag;
      usr::flag2_t flag2 = u->m_flag2;
      if ((flag & usr::TYPEDEF) || (flag2 & usr::ALIAS) ||
	  (flag2 & usr::TEMPLATE) &&
	  (static_cast<template_usr*>(u)->m_express_type)) {
	  parse::identifier::mode = parse::identifier::new_obj;
      }
    }
  }
EOF2


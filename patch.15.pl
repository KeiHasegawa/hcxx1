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
    if (ut->first) {
      YYDPRINTF((stderr, "patch.15 is applied\\n"));
      yyn = $bbb + 1;
    }
  }
EOF


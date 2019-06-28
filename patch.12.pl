#state XXX
#
#  AAA unqualified_id: '~' . class_name
#  BBB unary_operator: '~' .
#
#  Example:
#
#  struct X {
#    X& operator~();
#    ~X();
#  };
#
#  void g(X* p)
#  {
#    p->~X();  // explicit destructor call
#    ~X();     // operator~ call for temporary object
#  }
#
while (<>) {
    chop;
    next if ( !/^[Ss]tate (.*)/ );
    $state = $1;
    $_ = <>;
    $_ = <>; chop;
    if ( /[0-9]+ unqualified_id: \'~\' \. class_name/ ){
	$_ = <>; chop;
	if ( /([0-9]+) unary_operator: \'~\' \./ ){
	    $xxx = $state;
	    $bbb = $1;
	    next;
	}
    }
}

if ($xxx == 0) {
    print STDERR "Error detected at $0\n";
    exit 1;
}

print <<EOF
  if (yystate == $xxx) {
    using namespace cxx_compiler;
    if (scope::current->m_id != scope::TAG) {
      YYDPRINTF((stderr, "patch.12 is applied\\n"));
      yyn = $bbb + 1;
      goto yyreduce;
    }
  }
EOF

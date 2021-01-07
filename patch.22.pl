#State XXX
#
#  AAA primary_expression: compound_statement .
#  BBB statement: compound_statement .
#
#  Almost use BBB rule
while ( <> ) {
    chop;
    next if ( !/^[Ss]tate (.*)/ );
    $xxx = $1;
    $_ = <>;
    $_ = <>; chop;
    next if (!/([0-9]+) primary_expression: compound_statement \./);
    $aaa = $1;
    $_ = <>; chop;
    next if (!/([0-9]+) statement: compound_statement \./);
    $bbb = $1;
    goto label;
}

print STDERR "Error detected at $0\n";
exit 1;

label:
print<<EOF
  if (yystate == $xxx) {
    YYDPRINTF((stderr, "patch.22 is applied\\n"));
    yyn = $bbb + 1;
  }
EOF


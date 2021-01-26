#State XXX
#
# AAA primary_expression: '{' . '}'
# BBB                   | '{' . expression_list '}'
# CCC compound_statement: '{' . enter_block statement_seq leave_block '}'
# DDD                   | '{' . enter_block leave_block '}'
# ...
# ORIGINAL_NAMESPACE_NAME_LEX         [reduce using rule EEE (enter_block)]
#
#  Uuse EEE rule for reduce
while ( <> ) {
    chop;
    next if ( !/^[Ss]tate (.*)/ );
    $xxx = $1;
    $_ = <>;
    $_ = <>; chop;
    next if (!/([0-9]+) primary_expression: '{' \. '}'/);
    $aaa = $1;
    $_ = <>; chop;
    next if (!/([0-9]+) .*| '{' \. expression_list '}'/);
    $bbb = $1;
    $_ = <>; chop;
    next if (!/([0-9]+) compound_statement: '{' \. enter_block statement_seq leave_block '}'/);
    $ccc = $1;
    $_ = <>; chop;
    next if (!/([0-9]+) .*| '{' \. enter_block leave_block '}'/);
    $ddd = $1;
    while (<>) {
	chop;
	next if (!/reduce using rule ([0-9]+) \(enter_block\)/);
	$eee = $1;
	goto label;
    }
}

print STDERR "Error detected at $0\n";
exit 1;

label:
print<<EOF
  if (yystate == $xxx) {
    YYDPRINTF((stderr, "patch.22 is applied\\n"));
    yyn = $eee + 1;
    goto yyreduce;
  }
EOF


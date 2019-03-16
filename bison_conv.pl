print "#pragma warning ( disable : 4065 )\n";
print "#pragma warning ( disable : 4060 )\n";
print "#include \"stdafx.h\"\n";
print "#include \"yy.h\"\n";

$yychar_converted = 0;
$yydefault_converted = 0;
$patch08_inserted = 0;

while ( <> ){
  chop;
  s/	/        /;
  s/cxx\.tab\.c/cxx_y.cpp/;
  if ( /yychar = yylex \(\);/ ){
    print "      yychar = cxx_compiler::parse::get_token();\n";
    ++$yychar_converted;
    next;
  }
  if ( /yychar = YYLEX;/ ){
    print "      yychar = cxx_compiler::parse::get_token();\n";
    ++$yychar_converted;
    next;
  }
  if ( /if \(yyn < 0 \|\| YYLAST < yyn \|\| yycheck\[yyn\] \!= yytoken\)/ ){
    print $_, "\n";
    $_ = <>; chop;
    if (!/goto yydefault;/) {
	print STDERR "Error detected at $0\n";
	print STDERR "no goto yydefault;\n";
	exit 1;
    }
    print <<EOF2
  {
#include "patch.04.p"
    goto yydefault;
  }
#include "patch.03.p"
#include "patch.05.p"
#include "patch.09.p"
#include "patch.10.p"
EOF2
	;
    ++$yydefault_converted;
    next;
  }
  print $_,"\n";
  if ( /yystate = yydefgoto/ ){
    print "#include \"patch.00.p\"\n";
  }
  if ( /yystate = yyn/ ){
    print "#include \"patch.01.p\"\n";
    print "#include \"patch.02.p\"\n";
  }
  if ( /yyn.*=.*yydefact\[yystate\];/ ){
    print "#include \"patch.06.p\"\n";
    print "#include \"patch.11.p\"\n";
  }
  if ( /yystate = 0/ ){
    print "#include \"patch.07.p\"\n";
  }
  if ( /YY_SYMBOL_PRINT \(\"-> \$\$ =\", yyr1\[yyn\], &yyval, &yyloc\);/ ){
    print "#include \"patch.08.p\"\n";
    ++$patch08_inserted;
  }
  if (/^yyerrlab:/) {
      print<<EOF
  if (!cxx_compiler::parse::context_t::all.empty()) {
    cxx_compiler::parse::restore(&yystate, &yyss, &yyssp, yyssa, &yyvs, &yyvsp, yyvsa);
    ++cxx_compiler::parse::context_t::retry[yystate];
    YYDPRINTF((stderr, "retry!\\n"));
    YY_STACK_PRINT(yyss, yyssp);
    goto yynewstate;
  }
EOF
  }
}

if ($yychar_converted != 1) {
    print STDERR "Error detected at $0\n";
    print STDERR '$yychar_converted = ', $yychar_converted, "\n";
    exit 1;
}

if ($yydefault_converted != 1) {
    print STDERR "Error detected at $0\n";
    print STDERR '$yydefault_converted = ', $yydefault_converted, "\n";
    exit 1;
}

if ($patch08_inserted != 1) {
    print STDERR "Error detected at $0\n";
    print STDERR '$patch08_inserted = ', $patch08_inserted, "\n";
    exit 1;
}

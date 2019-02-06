while ( <> ){
    chop;
    if ( /^declarator \(([0-9]+)\)$/ ){
	print "  case $1: /* declarator */\n";
	print "    yyvaluep->m_var->backtrack();\n"
	print "    break;\n";
	next;
    }
    if ( /^decl_specifier_seq \(([0-9]+)\)$/ ){
	print "  case $1: /* decl_specifier_seq */\n";
	print "    yyvaluep->m_specifier_seq->backtrack();\n"
	print "    break;\n";
	next;
    }
}

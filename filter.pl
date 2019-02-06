while ( <> ){
    chop $_;
    if ( /cxx.y:([0-9]+)\.[0-9]+-[0-9]+:(.*)/ ){
	print STDERR "cxx.y:",$1,":",$2, "\n";
	next;
    }
    if ( /cxx.y:([0-9]+)\.[0-9]+:(.*)/ ){
	print STDERR "cxx.y:",$1,":",$2, "\n";
	next;
    }
    print STDERR $_, "\n";
}

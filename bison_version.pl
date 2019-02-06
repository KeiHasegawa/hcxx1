$mismatch = 0;

while ( <> ) {
    chop;
    if ( /bison \(GNU Bison\) ([0-9])\.([0-9])\.([0-9])/) {
	if ($1 == 3 && $2 == 0 && $3 == 4) {
	    # ok
	}
	else {
	    $mismatch = 1;
	}
    }
    print $_, "\n";
}

if ($mismatch) {
    print "version mismatch.";
    exit 1;
}

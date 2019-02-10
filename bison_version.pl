$mismatch = 0;

$ok = 0;

$version = 0;

while ( <> ) {
    chop;
    if ( /bison \(GNU Bison\) ([0-9])\.([0-9])\.([0-9])/) {
	$version = $1 * 100 + $2 * 10 + $3; 
	if (243 <= $version && $version <= 304 ) {
	    ++$ok;
	}
	else {
	    $mismatch = 1;
	}
    }
}

if ($ok != 1) {
    print "version checked failed. version = ", $version, "\n";
    exit 1;
}

if ($mismatch) {
    print "version mismatch. version = ", $version, "\n";
    exit 1;
}

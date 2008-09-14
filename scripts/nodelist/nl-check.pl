#!/usr/local/bin/perl
#
# Apply FIDO nodediff to nodelist
#

require "getopts.pl";

&Getopts('v');


if($#ARGV < 0) {
    print STDERR
	"usage:   nl-check [-v] nodelist.nnn ...\n\n",
	"options:   -v          verbose\n";
    exit 1;
}

for $f (@ARGV) {
    print "Checking nodelist $f\n" if($opt_v);
    
    open(F,"$f") || die "nl-check: can't open $f\n";
    $_ = <F>;

    # CRC check
    if( /Day number \d+ : (\d+)/ ) {
	$old = $1;
	print "CRC from 1st line of $f: $old\n" if($opt_v);
	print "CRC check for nodelist $f ... ";
	$new = `sumcrc -1z $f`;
	chop $new;
	if( $old == $new ) {
	    print "OK\n";
	    print "CRC computed for $f: $new\n" if($opt_v);
	}
	else {
	    print "ERROR\n\texpected: $old\n\tcomputed: $new\n";
	}
    }
    else {
	print "nl-check:$f: strange error - no CRC found in 1st line\n";
    }

    close(F);

}

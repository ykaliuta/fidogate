#!/usr/bin/perl
#
# List programs and cf_get_string() parameters
#

$SRCDIR="../src";
    
for $f (<$SRCDIR/common/*.c $SRCDIR/ffx/*.c $SRCDIR/gate/*.c $SRCDIR/tick/*.c $SRCDIR/toss/*.c $SRCDIR/util/*.c>) {

    undef $config;
    undef @parameter;
    $n = 0;

    # Process file
    open(F, "$f") || die "listcf: can't open $f\n";
    while(<F>) {
	if( /^\#define\s*CONFIG\s*(.*)\s*$/ ) {
	    $config = $1;
	}
	if( /=\s*cf_get_string\(\"(.*)\"\s*,\s*TRUE\s*\)/ ) {
	    $para = $1.":p";
	    $parameter[$n++] = "$1:p";
	}
	elsif( /cf_get_string\(\"(.*)\"\s*,\s*TRUE\s*\)/ ) {
	    $para = $1;
	    if( $para =~ /^(.*)\".*\|\|.*cf_get_string\(\"(.*)$/ ) {
		$parameter[$n++] = "$1:p";
		$parameter[$n++] = "$2:p";
	    }
	    else {
		$parameter[$n++] = "$1:p";
	    }
	}
    }
    close(F);

    $prog = $f;
    $prog =~ s/.*\///;
    $prog =~ s/\.c$//;
    
    $config =~ s/CONFIG_MAIN/config.main/;
    $config =~ s/CONFIG_FFX/config.ffx/;
    $config =~ s/CONFIG_GATE/config.gate/;

    if($n) {
	print
	    "\@noindent Special configuration \@code{$prog}",
	    "(config file \@code{$config}):\n\n",
	    "\@table \@code\n\n";
	for $p (@parameter) {
	    $pp = $p;
	    $p =~ s/:.$//;
	    print "\@item $p";
	    print " \@i{parameter}" if($pp =~ /:p$/);
	    print "\n\n";
	}
	print "\@end table\n\n\n";
    }
}

exit

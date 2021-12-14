#!/usr/local/bin/perl
#
# Apply FIDO nodediff to nodelist
#

require "getopts.pl";

&Getopts('b:i:o:vr');


if($#ARGV != 0) {
    print STDERR
	"usage:   nl-diff [-vr] [-o output] nodediff.nnn\n\n",
	"options:  -v           verbose\n",
	"          -r           remove old nodelist\n",
 	"          -i input     use file input for old nodelist\n",
	"          -o output    use file output for new nodelist\n",
	"          -b basename  basename of nodelist files [nodelist]\n";
    exit 1;
}


$diff     = $ARGV[0];

$basename = "nodelist";
$ddd      = "999";

if( $diff =~ /\.(\d\d\d)$/ ) {
    $ddd      = $1;
}
if( $opt_b ) {
    $basename = $opt_b;
}


open(DIFF, $diff) || die "nl-diff: can't open nodediff $diff\n";
$DIFF = DIFF;

# Read 1st line from diff
$l1st = <$DIFF>;

if( $l1st =~ /Day number (\d*)/ ) {
    $ooo = $1;
}

$input  = "$basename.$ooo";
$output = "$basename.$ddd";


open(IN, $input) || die "nl-diff: can't open old nodelist $input\n";
$IN = IN;

if($opt_o) {
    $output = $opt_o;
}
if($output eq "-") {
    $OUT = STDOUT;
}
else {
    open(OUT, ">$output") || die "nl-diff: can't open new nodelist $output\n";
    $OUT = OUT;
}

if( $opt_v ) {
    print "Processing $diff: $input -> $output\n";
}


# Process diff commands
$read1st  = 1;
$write1st = 1;

while(<$DIFF>) {
    if( /^\cZ/ ) {		# MSDOS ^Z EOF
	break;
    }
    if( /^A([0-9]+)/ ) {	# Add lines from diff
	$n = $1;
	while($n--) {
	    $_ = <$DIFF>;
	    if(! $_) {
		die "nl-diff: premature eof reading nodediff\n";
	    }
	    print $OUT $_;
	    if( $write1st ) {
		$write1st = 0;
		$wl1st = $_;
	    }
	}
    }
    elsif( /^C([0-9]+)/ ) {	# Copy lines from list
	$n = $1;
	while($n--) {
	    $_ = <$IN>;
	    if(! $_) {
		die "nl-diff: premature eof reading nodelist\n";
	    }
	    if($read1st) {
		$read1st = 0;
		if($_ ne $l1st) {
		    die "nl-diff: nodelist and nodediff not matching\n";
		}
	    }
	    print $OUT $_;
	    if( $write1st ) {
		$write1st = 0;
		$wl1st = $_;
	    }
	}
    }
    elsif( /^D([0-9]+)/ ) {	# Delete lines from list
	$n = $1;
	while($n--) {
	    $_ = <$IN>;
	    if(! $_) {
		die "nl-diff: premature eof reading nodelist\n";
	    }
	    if($read1st) {
		$read1st = 0;
		if($_ ne $l1st) {
		    die "nl-diff: nodelist and nodediff not matching\n";
		}
	    }
	}
    }
    else {
	die "nl-diff: unknown nodediff command at line $.\n";
    }
}

close($DIFF);
close($IN);
close($OUT);

# CRC check
if( $wl1st =~ /Day number \d+ : (\d+)/ ) {
    $old = $1;
    if( $opt_v ) {
	print "CRC check for nodelist $output ... ";
    }
    $new = `sumcrc -1z $output`;
    if( $old == $new ) {
	print "OK\n" if ($opt_v);
	if($opt_r) {
	    unlink($input) || die "nl-diff: can't remove $input\n";
	    print "Removing old nodelist $input\n";
	}
    }
    else {
	print "ERROR\n\texpected: $old\n\tcomputed: $new\n" if ($opt_v);
	die "nl-diff: checksum failure\n";
    }
}
else {
    die "nl-diff: strange error - no CRC found in 1st line\n";
}

exit 0

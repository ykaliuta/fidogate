#!/usr/local/bin/perl
#:ts=8

require "getopts.pl";

&Getopts('ni');

$isdn_flag = 1;

if( $opt_n ) {			# -n = non-ISDN nodelist
    $isdn_flag = 0;
}

if($#ARGV < 0) {
    print STDERR "usage: isdn.pl [-ni] nodelist.xxx ...\n";
    exit 1;
}

while(<>) {
    if ( /^;/ || /^\cZ/ ) {
#	print;
	next;
    }

    ($key,$num,$name,$loc,$sysop,$phone,$bps,$flags) = split(',', $_, 8);

    # Test for ISDN
    $isdn = $flags =~ /ISDN/;

    # Direct output, if no modifications needed
    if( ($isdn && $isdn_flag) || (!$isdn && !$isdn_flag)) {
	print;
    }
    # else remove CM flag
    else {
	$flags =~ s/CM,//;
	$flags =~ s/,CM//;
	if($flags eq "CM\r\n") {
	    $flags = "\r\n";
	}
	print $key,",",$num,",",$name,",",$loc,",",
	      $sysop,",",$phone,",",$bps,",",$flags;
    }
}

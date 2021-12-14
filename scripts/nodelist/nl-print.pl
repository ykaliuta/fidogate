#!/usr/local/bin/perl
#:ts=8

#require "getopts.pl";
#&Getopts('o:z:v');


$zone = 2;
$net  = 0;
$node = 0;

while(<>) {
    if ( /^;/ || /^\cZ/ ) {
	# Output comments as is
	next;
    }

    # Split input line
    ($key,$num,$name,$loc,$sysop,$phone,$bps,$flags) =
	split(',', $_, 8);

    if($key eq "Zone") {	# Zone line
	$zone = $num;
	$net  = $num;
	$node = 0;
    }				# 
    elsif($key eq "Region") {	# Region line
	$net  = $num;
	$node = 0;
    }
    elsif($key eq "Host") {	# Host line
	$net = $num;
	$node = 0;
    }
    else {
	$node = $num;
    }

    printf "%-16s", "$zone:$net/$node";

    print "$name\n";

}



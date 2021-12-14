#!/usr/bin/perl
#
# $Id: nl-del.pl,v 4.1 1997/06/21 21:16:42 mj Exp $
#

require "getopts.pl";

&Getopts('o:z:vcel');

$output  = "nodelist.all";

if($opt_o) {
    $output = $opt_o;
}

if($opt_z) {
    for $z (split(/[,; ]/, $opt_z)) {
	$zones{$z} = 1;
    }
}

if($#ARGV < 0) {
    print STDERR
	"usage: nl-del [-vcel] [-o output] [-z ZONE,...] nodelist.xxx ...\n\n",
	"options:  -v           verbose\n",
	"          -c           preserve comments\n",
	"          -e           remove empty comments and ;E\n",
	"          -l           leave Zone line for deleted zones\n",
	"          -o nodelist  output nodelist\n",
	"          -z zone,...  list of zones to be deleted\n";
    exit 1;
}


open(OUT, ">$output") || die "nl-del: can't open output nodelist $output\n";
select(OUT);

if($opt_v) {
    print STDERR "Writing nodelist $output ...\n";
}

$zone = 2;
$net  = 0;
$node = 0;

$skipping_zone = 0;

while(<>) {
    next if(/^\cZ/);

    next if($opt_e && /^;\s*\cM?\cJ?$/);
    next if($opt_e && /^;E/);

    if(/^;/) {
	# Output comments
	print if($opt_c);
	next;
    }

    # Split input line
    ($key,$num,$name,$loc,$sysop,$phone,$bps,$flags) =
	split(',', $_, 8);

    if($key eq "Zone") {	# Zone line
	$zone = $num;
	$net  = $num;
	$node = 0;
	$skipping_zone = 0;
	if($zones{$zone}) {	# Skip this zone
	    if($opt_v) {
		printf STDERR "%-70s\n", "Zone $zone ($name) ... deleted";
	    }
	    print if($opt_l);
	    print "; Zone $zone deleted ...\n";
	    $skipping_zone = 1;
	}
	else {
	    print;			# Always print Zone line
	    if($opt_v) {
		printf STDERR "%-70s\n", "Zone $zone ($name) ... ";
	    }
	}
	next;
    }
    if($skipping_zone) {
	next;
    }

    if($key eq "Region") {	# Region line
	if($opt_v) {
	    printf STDERR "%-70s\r", "Region $num ($name)";
	}
	$net  = $num;
    }
    elsif($key eq "Host") {	# Host line
#	print STDERR "Network $num ($name)\r";
	$net = $num;
    }
    else {
	$node = $num;
    }

    print;
}

print STDERR "\n";

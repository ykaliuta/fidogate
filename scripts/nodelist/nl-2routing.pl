#!/usr/bin/perl
#:ts=8
#
# $Id: nl-2routing.pl,v 4.1 1997/02/09 10:04:25 mj Exp $
#
# nl-2routing --- konvertiert Network-File in eine Routing-Tabelle
#                 um Hub-Routing mit Fidogate (oder Squish (ungetestet!))
#                 zu realisieren
#
# Contributed by Andreas Braukmann <andy@AbrA.DE>
#
# basiert auf nl-* von Martin Junius
#

require "getopts.pl";
&Getopts('b:t:x:');


$zone  = 2;
$net   = 2426;
$node  = 0;
$route = 0;
$host  = 0;

$routhead = "routing.head";
$routfoot = "routing.foot";

# routing fuer Points oder Nodes aus anderen Netzen wird in routing.haed 
# definiert
#
open(FILE, "$routhead");
while(<FILE>) {
	print;
}
close(FILE);

while(<>) {
    if ( /^;/ || /^\cZ/ ) {
	# Output comments as is
	next;
    }

    # Split input line
    chop;
    ($key,$num,$name,$loc,$sysop,$phone,$bps,$flags) =
	split(',', $_, 8);

    if($key eq "Zone") {	# Zone line
	$zone = $num;
	$net  = $num;
	$node = 0;
	$route = 0;
	$host = 1;
    }
    elsif($key eq "Region") {	# Region line
	$net  = $num;
	$node = 0;
	$route = 0;
	$host = 1;
    }
    elsif($key eq "Host") {	# Host line
	$net = $num;
	$node = 0;
	$route = 0;
	$host = 1;
    }
    elsif($key eq "Hub") {	# Hub line
	$node = $num;
	$route = 1;
	$host = 0;
    }
    else {
	$node = $num;
	$route = 2;
    }

    $addr = "$zone:$net/$node";

    print "\nroute\thold\t$zone:$net/$node " if($route == 1);
	# Hub gefunden

    print "$addr.all " if($route == 2 && $host == 0);
	# alle Nodes und Points ueber diesen Hub Routen
}
# alles anderen AKA's (/0, /1 etc) werden via default-Routing gerutet oder
# seperat in routing.haed angegeben.
#
# das defaultrouting wird in routing.foot definiert.
#
open(FILE, "$routfoot");
while(<FILE>) {
	print;
}
close(FILE);
# end

#!/usr/bin/perl

require "./fido.pli";

&readconfig;

$orig="2:2432/605.1";
$dest="2:2432/605";
$from="Matthias Schniedermeyer";
$to="Areafix";
$subject="GEHEIM";
$message="+LINUX.GER\r\n";
$tearline="TestProggie 1";
$area="DARKNESS.POINTS";
$origin=" * Origin: Halllllllooooooooooo Schwester (2:2432/605.1)\r";
$pw="GEHEIM\x00\x00";

&createpacket ($orig,$dest,$pw);
&addnm ($orig,$dest,$from,$to,$subject,$message,$tearline);
#&addem ($orig,$dest,$from,$to,$subject,$message,$tearline,$area,$origin);
&closepacket;

open (P,">test.pkt");
for (@packet) { print P; }
close P;

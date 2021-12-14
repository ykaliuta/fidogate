#!/usr/bin/perl

require "fido.pli";

&readconfig;
&parseareasbbs;
&aka2hash;

$test = 1;
$line = 1;
$last = $index{$line};
$createpacket = 1;

while ($test) {
  $line++;
  $area = $index{$line};
  if ($area le $last) {
    undef $test;
  }
  $last = $area;
}

$area = $last;

foreach $uplink (@uplink) {
  $test = 1;
  $zone = $uplink;
  $zone =~ s/:.*$//;
  while ($test) {
    if ($zone{$area} == $zone) {
      if ($subscribe{$area} eq "#+") {
	@linklist = split /\s+/,$links{$area};
	shift @linklist;
	$tmp = shift @linklist;
	unless ($tmp) {
	  $subscribe{$area}="#-";
	  open (TO,">>tounsubscribe");
	  print TO "$area\n";
	  close TO;
	  $areasbbschanged=1;
	}
      } else {
	@linklist = split /\s+/,$links{$area};
        shift @linklist;
        $tmp = shift @linklist;
	if ($tmp) {
          $subscribe{$area}="#+";
          open (TO,">>tosubscribe");
          print TO "$area\n";
          close TO;
          $areasbbschanged=1;
        }
      }
      $area = $index{$line++};
      $test=1;
    } else {
      $test = 0;
    }
  }
}
  
if ($areasbbschanged) {
  &writeareasbbs;
}

# Eine Area beim Uplink bestellen?
if ( -f "tosubscribe" ) {
  &subscribe;
}

# Eine Area beim Uplink abbestellen?
if ( -f "atounsubscribe" ) {
  &unsubscribe;
}

# Array in dem das Packet steht auf Platte schreiben
if ($packetexist) {
  &writepacket;
}

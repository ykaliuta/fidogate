#!/usr/bin/perl -w

require "fido.pli";
$filefix = 0;
$name = $0;
$name =~ /\/(.*)$/;
$name = $1;

if ($name =~ /filefix/i) {
  $filefix = 1;
}

# Ein paar Variablen (damit Perl sich nicht beschwert!)
$createpacket=1;
$areasbbschanged=0;
undef %akalist;
undef @toparsea;
undef @toparsef;

# Read Config
&readconfig;

# Erzeuge ein Hash der Akalist
&aka2hash;

# areas.bbs einlesen
&parseareasbbs;

# Und los
if ($config{"test"} == 0) {
  if ($filefix) {
    @toparse = @toparsef;
  } else {
    @toparse = @toparsea;
  }
  for $toparse (@toparse) {
    &getfilename ($toparse);
    if (-f $parsefilename) {
      &openpacket($parsefilename);
      # Packt erzeugen
      if ($createpacket) {
	&createpacket ($config{"mainaka"},$config{"mainaka"},"GEHEIM\x00\x00");
	undef $createpacket;
      }
      # Message einlesen, parsen, Antwort Mail schreiben
      while (&getnextmessage) {
	&parseareafixmail($orig,$from,$subject,$message);
	$zonetosend = $orig;
	$zonetosend =~ s/:.*$//;
	if ($filefix) {
	  &addnm ($akalist{$zonetosend},$orig,"Filefix",$from,"Your Filefix request",$messageback,$config{"tearlinef"});
	} else {
	  &addnm ($akalist{$zonetosend},$orig,"Areafix",$from,"Your Areafix request",$messageback,$config{"tearlinea"});
	}
      }
      close PACKET;
      unlink $parsefilename or die "Kann $parsefilename nicht loeschen";
    }    
  }
} else {
  &openpacket("test.pkt") or die "Kann Testfile nicht oeffnen\n";
  while (&getnextmessage) {
    &parseareafixmail($orig,$from,$subject,$message);
    print $messageback;
    $zonetosend = $orig;
    $zonetosend =~ s/:.*$//;
  }
  close PACKET;
}

# Geaenderte areas.bbs auf Platte schreiben.
if ($areasbbschanged) {
  &writeareasbbs;
}

# Eine Area beim Uplink bestellen?
if ( -f "tosubscribe" ) {
  &subscribe;
}

# Eine Area beim Uplink abbestellen?
if ( -f "tounsubscribe" ) {
  &unsubscribe;
}

# Array in dem das Packet steht auf Platte schreiben
if (!$createpacket) {
  &writepacket;
}

#!/usr/local/bin/perl
#
# $Id: logreport.pl,v 4.1 1998/01/02 14:37:05 mj Exp $
#
# Create report for "problems" in FIDOGATE tosser log file:
#   - Unknown areas
#   - Insecure EchoMail
#   - Routed EchoMail
#   - EchoMail with circular path
#   - Messages from gateway
#

$NEWSGROUPS = "fido.de.lists";
$SUBJECT    = "EchoMail problem report";


$INEWS      = "/usr/bin/inews -h -S";
$SENDMAIL   = "/usr/lib/sendmail";


require "getopts.pl";
&Getopts('g:s:nm:');

if($opt_g) {
    $NEWSGROUPS = $opt_g;
}
if($opt_s) {
    $SUBJECT    = $opt_s;
}
if($opt_n) {
    open(OUT, "|$INEWS") || die "logreport: can't open pipe to inews\n";
    select(OUT);
    $out_flag = 1;
}
if($opt_m) {
    open(OUT, "|$SENDMAIL $opt_m")
	|| die "logreport: can't open pipe to sendmail\n";
    select(OUT);
    $out_flag = 1;
}



while(<>) {
    # Unknown areas
    if( /ftntoss unknown area (.+) from ([0-9:\/.]+)$/ ) {
	$key = "$2 $1";
	$unknown{$key}++;
    }

    # Insecure EchoMail
    if( /ftntoss insecure echomail area (.+) from ([0-9:\/.]+)$/ ) {
	$key = "$2 $1";
	$insecure{$key}++;
    }

    # Routed EchoMail
    if( /ftntoss routed echomail area (.+) from ([0-9:\/.]+) to ([0-9:\/.]+)$/ ) {
	$key = "$2 $1 $3";
	$routed{$key}++;
    }

    # EchoMail with circular path
    if( /ftntoss circular path echomail area (.+) from ([0-9:\/.]+) to ([0-9:\/.]+)$/ ) {
	$key = "$2 $1 $3";
	$circular{$key}++;
    }

    # Messages from other FTN gateway
    if( /ftn2rfc skipping message from gateway.*origin=([0-9:\/.]+)$/ ) {
	$key = "$1";
	$gateway{$key}++;
    }
}

# Report
print "Newsgroups: $NEWSGROUPS\n" if($opt_n);
print "Subject: $SUBJECT\n";

print "\n";

if(scalar(%unknown)) {
    print "Unknown EchoMail areas:\n\n";
    print "    From                            ",
          "Area                            Msgs\n";
    print "    ----                            ",
          "----                            ----\n";
    
    for $k (sort keys(%unknown)) {
	($n,$a) = split(' ', $k);
    printf "    %-15.15s                 %-31.31s %d\n", $n, $a, $unknown{$k};
}

print "\n";
}

if(scalar(%insecure)) {
    print "Insecure EchoMail:\n\n";
    print "    From                            ",
          "Area                            Msgs\n";
    print "    ----                            ",
          "----                            ----\n";

    for $k (sort keys(%insecure)) {
	($n,$a) = split(' ', $k);
	printf "    %-15.15s                 %-31.31s %d\n",
	$n, $a, $insecure{$k};
    }

    print "\n";
}

if(scalar(%routed)) {
    print "Routed EchoMail:\n\n";
    print "    From            To              ",
          "Area                            Msgs\n";
    print "    ----            --              ",
          "----                            ----\n";
    
    for $k (sort keys(%routed)) {
	($n,$a,$t) = split(' ', $k);
	printf "    %-15.15s %-15.15s %-31.31s %d\n", $n, $t, $a, $routed{$k};
    }
    
    print "\n";
}

if(scalar(%circular)) {
    print "EchoMail with circular ^APATH:\n\n";
    print "    From            To              ",
          "Area                            Msgs\n";
    print "    ----            --              ",
          "----                            ----\n";
    
    for $k (sort keys(%circular)) {
	($n,$a,$t) = split(' ', $k);
	printf "    %-15.15s %-15.15s %-31.31s %d\n", $n, $t, $a, $circular{$k};
    }
    
    print "\n";
}

if(scalar(%gateway)) {
    print "EchoMail messages from other FTN gateways ",
          "(at least looking like that):\n\n";
    print "    From                            ",
          "                                Msgs\n";
    print "    ----                            ",
          "                                ----\n";
    
    for $k (sort keys(%gateway)) {
	($n) = split(' ', $k);
	printf "    %-15.15s %-15.15s %-31.31s %d\n", $n, "", "", $gateway{$k};
    }
    
    print "\n";
}

if($out_flag) {
    close(OUT);
}

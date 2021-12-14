#!/usr/local/bin/perl
#
# $Id: logstat.pl,v 4.0 1996/04/17 18:17:38 mj Exp $
#
# Statistics for ftntoss/ftnroute/ftnpack log files
#

$NEWSGROUPS = "fido.de";
$SUBJECT    = "EchoMail statistics report";


$INEWS      = "/usr/bin/inews -h -S";
$SENDMAIL   = "/usr/lib/sendmail";


require "getopts.pl";
&Getopts('g:s:t:nm:');

if($opt_g) {
    $NEWSGROUPS = $opt_g;
}
if($opt_s) {
    $SUBJECT    = $opt_s;
}
if($opt_t) {
    $SUBJECT    = "$SUBJECT $opt_t";
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
    
    if( /^(... .. ..:..:..) ftntoss packet.*\((\d+)b\) from ([0-9:\/.]+) / ) {
	$first = $1 if(! $first);
	$size = $2;
	$node = $3;

	$in_total += $size;
	$in_node{$node} += $size;
    }

    if( /(... .. ..:..:..) ftnpack .*packet \((\d+)b\) for ([0-9:\/.]+) / ) {
	$last = $1;
	$size = $2;
	$node = $3;

	$out_total += $size;
	$out_node{$node} += $size;
    }

}


sub in_bynumber  { $in_node{$b}  <=> $in_node{$a};  }
sub out_bynumber { $out_node{$b} <=> $out_node{$a}; }



print "Newsgroups: $NEWSGROUPS\n" if($opt_n);
print "Subject: $SUBJECT\n";

print "\n";


print "Period $first -- $last\n\n";


printf "In:  total           %7ldK\n", $in_total/1024;
for $n (sort in_bynumber keys(%in_node)) {
    printf "     %-16s%7ldK\n", $n, $in_node{$n}/1024;
}

printf "\n";

printf "Out: total           %7ldK\n", $out_total/1024;
for $n (sort out_bynumber keys(%out_node)) {
    printf "     %-16s%7ldK\n", $n, $out_node{$n}/1024;
}


if($out_flag) {
    close(OUT);
}

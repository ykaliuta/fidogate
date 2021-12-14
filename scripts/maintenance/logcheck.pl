#!/usr/bin/perl
#
# $Id: logcheck.pl,v 4.13 2001/10/30 20:02:52 mj Exp $
#
# Create report for sendmail check_mail/rcpt/relay rejects
#

use strict;
use Getopt::Std;
use FileHandle;


my $PROGRAM     = 'logcheck';
my $VERSION     = '1.0 $Revision: 4.13 $ ';


my $NEWSGROUPS  = "fido.de.lists";
my $SUBJECT     = "Fido.DE Sendmail Reject Report";
my $MAX         = 50;
my $TH_DISABLED = 100;
my $TH_RSS      = 50;

my $INEWS       = "/usr/bin/inews -h -S";
my $SENDMAIL    = "/usr/sbin/sendmail";

my $out_flag    = 0;




##### Command line ###########################################################
use vars qw($opt_g $opt_s $opt_n $opt_m $opt_v $opt_r
	    $opt_h $opt_h $opt_k $opt_N);
getopts('g:s:nm:vrhkN:');

if($opt_h) {
    print STDERR
      "\n",
      "$PROGRAM --- Summary of sendmail rejects\n",
      "\n",
      "usage:  $PROGRAM [-vh] [-r] [-k] [-N MAX]\n",
      "                 [-n] [-m EMAIL] [-g NEWSGROUPS] [-s SUBJECT]\n",
      "          -v               verbose\n",
      "          -h               this help\n",
      "          -r               include relay in output\n",
      "          -k               output killip commands\n",
      "          -N MAX           output top MAX entries only [default: $MAX]\n",
      "          -n               post output as news article\n",
      "          -m EMAIL         send output as mail to EMAIL\n",
      "          -g NEWSGROUPS    newsgroup(s) for posting [default: $NEWSGROUPS]\n",
      "          -s SUBJECT       subject for mail/news\n",
      "\n";
    
    exit 1;
}


if($opt_N) {
    $MAX        = $opt_N;
}
if($opt_g) {
    $NEWSGROUPS = $opt_g;
}
if($opt_s) {
    $SUBJECT    = $opt_s;
}
if($opt_n) {
    open(OUT, "|$INEWS")
      || die "logreport: can't open pipe to inews\n";
    select(OUT);
    $out_flag = 1;
}
if($opt_m) {
    open(OUT, "|$SENDMAIL $opt_m")
      || die "logreport: can't open pipe to sendmail\n";
    select(OUT);
    $out_flag = 1;
}
if($opt_k) {
    $opt_r = 1;
    $opt_n = 0;
    $opt_m = 0;
}



print "Newsgroups: $NEWSGROUPS\n" if($opt_n);
print "Subject: $SUBJECT\n" if($opt_m || $opt_n);
print "\n" if($opt_m || $opt_n);


my $first_date;
my $last_date;
my $addr;
my $r;
my $k;
my $n;

my %rbl_rss;
my %rbl_dul;
my %rbl_rbl;
my %reject;
my %nodns;
my %relay;
my %disabled;
my %rbl_orxx;
my $orxx;
my %count;



# Read sendmail log
while(<>) {
    chop;

    if( /^(... .\d \d\d:\d\d:\d\d) / ) {
	$first_date = $1 if(!$first_date);
	$last_date  = $1;
    }

    # RBL
    if( /ruleset=check_relay, arg1=(.*), arg2=(.*), relay=(.*), reject=.* Rejected -/ ) {
	$addr = $1;
	$addr = "<$addr>" if(! $addr =~ /^<.*>$/);
	$r = $3;
	$rbl_rbl{"$addr /// $r"}++;
	$count{$r}++;
	print "rbl rbl: $addr\n" if($opt_v);
    }
    # DUL
    elsif( /ruleset=check_relay, arg1=(.*), arg2=(.*), relay=(.*), reject=.* Dialup -/ ) {
	$addr = $1;
	$addr = "<$addr>" if(! $addr =~ /^<.*>$/);
	$r = $3;
	$rbl_dul{"$addr /// $r"}++;
	$count{$r}++;
	print "rbl dul: $addr\n" if($opt_v);
    }
    # RSS
    elsif( /ruleset=check_relay, arg1=(.*), arg2=(.*), relay=(.*), reject=.* Open spam relay -/ ) {
	$addr = $1;
	$addr = "<$addr>" if(! $addr =~ /^<.*>$/);
	$r = $3;
	if($opt_k) {
	    $rbl_rss{$r}++;
	}
	else {
	    $rbl_rss{"$addr /// $r"}++;
	}
	$count{$r}++;
	print "rbl rss: $addr\n" if($opt_v);
    }

    # ORxx
    elsif( /ruleset=check_relay, arg1=(.*), arg2=(.*), relay=(.*), reject=.* Open relay - see http:\/\/www\.(.*)\.org\/$/ ) {
#	$addr = $1;
#	$addr = "<$addr>" if(! $addr =~ /^<.*>$/);
	$r    = $1;
	$orxx = $4;
	$k    = $opt_k ? $r : "$addr /// $r";
	
	$rbl_orxx{$orxx} = {} unless defined($rbl_orxx{$orxx});
	$rbl_orxx{$orxx}->{$k}++;

	$count{$r}++;

	print "rbl $orxx: $addr\n" if($opt_v);
    }

    elsif( /ruleset=check_mail \(([^\)]*)\) rejection: 551/ ||
	   /ruleset=check_mail, arg1=(.*), relay=(.*), reject=55\d/ ) {
	$addr = $1;
	$addr = "<$addr>" if(! $addr =~ /^<.*>$/);
	$r = $opt_r ? $2 : "";
	$reject{"$addr /// $r"}++;
	print "reject: $addr\n" if($opt_v);
    }

    elsif( /ruleset=check_mail \(([^\)]*)\) rejection: 451/ ||
	   /ruleset=check_mail, arg1=(.*), relay=(.*), reject=(451|501)/ ) {
	$addr = $1;
	$addr = "<$addr>" if(! $addr =~ /^<.*>$/);
	$r = $opt_r ? $2 : "";
	$nodns{"$addr /// $r"}++;
	print "no DNS: $addr\n" if($opt_v);
    }

    # Local black list (To, "Mailbox disabled")
    elsif( /ruleset=check_rcpt, arg1=(.*),() relay=(.*), reject=550 .*Mailbox disabled/     ) {
	$addr = $1;
	$addr = "<$addr>" if(! $addr =~ /^<.*>$/);
	$r = $opt_r ? $3 : "";
	if($opt_k) {
	    $disabled{$r}++;
	}
	else {
	    $disabled{"$addr /// $r"}++;
	}
	print "disabled: $addr\n" if($opt_v);
    }

    elsif( /ruleset=check_rcpt \(([^\)]*)\)()() rejection: 551/             ||
	   /ruleset=check_mail, arg1=(.*),() relay=(.*), reject=551/        ||
	   /ruleset=check_relay, arg1=(.*), arg2=(.*), relay=(.*), reject=550/ ||
	   /ruleset=check_rcpt, arg1=(.*),() relay=(.*), reject=5\d\d/     ) {
	$addr = $1;
	$addr = "<$addr>" if(! $addr =~ /^<.*>$/);
	$r = $opt_r ? $3 : "";
	$relay{"$addr /// $r"}++;
	print "relay : $addr\n" if($opt_v);
    }

    elsif(/check_/) {
	print "NOT MATCHED: $_\n" if($opt_v);
    }
}



# Output killip commands
if($opt_k) {
#    print "# Local blacklist (To, \"Mailbox disabled\")\n";
#    for $k (sort { $disabled{$b} <=> $disabled{$a} } keys(%disabled)) {
#	$n = $disabled{$k};
#	printf "%5d %s\n", $n, $k;
#    }

    print "# ORxx >= $TH_RSS\n";
    for $k (sort { $count{$b} <=> $count{$a} } keys(%count)) {
	$n = $count{$k};
	next if($n < $TH_RSS);
	printf "%5d %s\n", $n, $k if($opt_v);
	if($k =~ /\[(.+)\]/) {
	    print "/root/m/killip $1\n";
	}
    }

    exit 0;
}



# Report
print "sendmail reject report: $first_date -- $last_date\n";

if(scalar(%reject)) {
    print
	"\nLocal blacklist rejects (From):\n",
	"-------------------------------\n";
    $n = 0;
    for $k (sort { $reject{$b} <=> $reject{$a} } keys(%reject)) {
	($addr, $r) = split(" /// ", $k);
	printf "%5d", $reject{$k};
	print " $addr\n";
	print "                relay: $r\n" if($opt_r);
	last if($MAX && $n++>$MAX);
    }
}

if(scalar(%disabled)) {
    print
	"\nLocal blacklist rejects (To):\n",
	"-----------------------------\n";
    $n = 0;
    for $k (sort { $disabled{$b} <=> $disabled{$a} } keys(%disabled)) {
	($addr, $r) = split(" /// ", $k);
	printf "%5d", $disabled{$k};
	print " $addr\n";
	print "                relay: $r\n" if($opt_r);
	last if($MAX && $n++>$MAX);
    }
}

if(scalar(%nodns)) {
    print
	"\nNo DNS rejects:\n",
	"---------------\n";
    $n = 0;
    for $k (sort { $nodns{$b} <=> $nodns{$a} } keys(%nodns)) {
	($addr, $r) = split(" /// ", $k);
	printf "%5d", $nodns{$k};
	print " $addr\n";
	print "                relay: $r\n" if($opt_r);
	last if($MAX && $n++>$MAX);
    }
}

if(scalar(%relay)) {
    print
	"\nRelay attempt rejects:\n",
	"----------------------\n";
    $n = 0;
    for $k (sort { $relay{$b} <=> $relay{$a} } keys(%relay)) {
	($addr, $r) = split(" /// ", $k);
	printf "%5d", $relay{$k};
	print " $addr\n";
	print "                relay: $r\n" if($opt_r);
	last if($MAX && $n++>$MAX);
    }
}

if(scalar(%rbl_rbl)) {
    print
	"\nMail-Abuse RBL rejects:\n",
	"-----------------------\n";
    $n = 0;
    for $k (sort { $rbl_rbl{$b} <=> $rbl_rbl{$a} } keys(%rbl_rbl)) {
	($addr, $r) = split(" /// ", $k);
	printf "%5d", $rbl_rbl{$k};
	print " relay: $r\n";
	last if($MAX && $n++>$MAX);
    }
}

if(scalar(%rbl_dul)) {
    print
	"\nMail-Abuse DUL rejects:\n",
	"-----------------------\n";
    $n = 0;
    for $k (sort { $rbl_dul{$b} <=> $rbl_dul{$a} } keys(%rbl_dul)) {
	($a, $r) = split(" /// ", $k);
	printf "%5d", $rbl_dul{$k};
	print " relay: $r\n";
	last if($MAX && $n++>$MAX);
    }
}

if(scalar(%rbl_rss)) {
    print
	"\nMail-Abuse RSS rejects:\n",
	"-----------------------\n";
    $n = 0;
    for $k (sort { $rbl_rss{$b} <=> $rbl_rss{$a} } keys(%rbl_rss)) {
	($addr, $r) = split(" /// ", $k);
	printf "%5d", $rbl_rss{$k};
	print " relay: $r\n";
	last if($MAX && $n++>$MAX);
    }
}

for $orxx (sort keys %rbl_orxx) {
    print
      "\n",
      uc($orxx), " rejects:\n",
      "-------------\n";
    $n = 0;
    for $k ( sort { $rbl_orxx{$orxx}->{$b} <=> $rbl_orxx{$orxx}->{$a} }
	     keys %{$rbl_orxx{$orxx}} ) {
	($addr, $r) = split(" /// ", $k);
	printf "%5d", $rbl_orxx{$orxx}->{$k};
	print " relay: $r\n";
	last if($MAX && $n++>$MAX);
    }

}


close(OUT) if($out_flag);

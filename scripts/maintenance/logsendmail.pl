#!/usr/bin/perl
#
# $Id: logsendmail.pl,v 4.5 2001/10/21 20:17:21 mj Exp $
#
# Gather statistics from sendmail V8 syslog output
#

require 5.000;

my $VERSION    = '$Revision: 4.5 $ ';
my $PROGRAM    = "logsendmail";

use strict;
use vars qw($opt_v $opt_c $opt_o $opt_g $opt_s $opt_n $opt_m);
use Getopt::Std;

# Common configuration for perl scripts 
<INCLUDE config.pl>


my $NEWSGROUPS = "fido.de.lists";
my $SUBJECT    = "Fido.DE Sendmail Accounting Report";

my $INEWS      = "/usr/bin/inews -h -S";
my $SENDMAIL   = "/usr/sbin/sendmail";


getopts('vc:o:g:s:nm:');

# read config
my $CONFIG     = $opt_c ? $opt_c : "<CONFIG_GATE>";
CONFIG_read($CONFIG);

my $HOSTNAME   = CONFIG_get("Hostname");
my $DOMAIN     = CONFIG_get("Domain");
$DOMAIN        = ".$DOMAIN" if(! $DOMAIN =~ /^\./);
my $FQDN       = $HOSTNAME.$DOMAIN;

print 
  "hostname = $HOSTNAME\n",
  "domain   = $DOMAIN\n",
  "fqdn     = $FQDN\n"
  if ($opt_v);


my $out_flag;
my $output;

if($opt_g) {
    $NEWSGROUPS = $opt_g;
}
if($opt_s) {
    $SUBJECT    = $opt_s;
}
if($opt_n) {
    open(OUT, "|$INEWS") || die "$PROGRAM: can't open pipe to inews\n";
    select(OUT);
    $out_flag = 1;
}
if($opt_m) {
    open(OUT, "|$SENDMAIL $opt_m")
	|| die "$PROGRAM: can't open pipe to sendmail\n";
    select(OUT);
    $out_flag = 1;
}
if($opt_o) {
    $output = $opt_o;
    open(OUT, ">$output") || die "$PROGRAM: can't open $output\n";
    select(OUT);
    $out_flag = 1;
}



print "Newsgroups: $NEWSGROUPS\n" if($opt_n);
print "Subject: $SUBJECT\n" if($opt_m || $opt_n);
print "\n" if($opt_m || $opt_n);


sub parse_addr {
    my($addr) = @_;
    my($user,$site);

    # Special: address is program
    if( $addr =~ /^\"\|/ ) {
	return ("", $FQDN);
    }

    # Parse address (1)
    if( $addr =~ /<(.+@.+)>/ ) {
	$a = $1;
    }
    elsif( $addr =~ /([^ ]+@.[^ ]+) \(.*\)/ ) {
	$a = $1;
    }
    else {
	$a = $addr;
    }
    $a =~ s/\s//g;			# Remove white space

    # Parse address (2)
    if( $a =~ /^(.+)@(.+)$/ ) {
	($user,$site) = ($1,$2);
    }
    elsif( $a =~ /^(.+)!(.+)$/ ) {
	($user,$site) = ($2,$1);
    }
    else {
	($user,$site) = ($a,$FQDN);
    }
    $site =~ tr/A-Z/a-z/;

    return ($user,$site);
}


my $first_date;
my $output_date;
my $last_date;
my $date;
my $id;
my $rest;
my $v;
my $to;
my $size;
my ($user, $site);
my $from;

my %id_size;
my %id_from;
my %entry;
my %dsn;
my %dsn_count;
my %ruleset_reject;
my $ruleset_count;

my %to_total_size;
my %to_total_msgs;
my $to_total_size;
my $to_total_msgs;
my %to_intern_size;
my %to_intern_msgs;
my $to_intern_size;
my $to_intern_msgs;
my %to_de_size;
my %to_de_msgs;
my $to_de_size;
my $to_de_msgs;
my %to_intl_size;
my %to_intl_msgs;
my $to_intl_size;
my $to_intl_msgs;

my %from_total_size;
my %from_total_msgs;
my $from_total_size;
my $from_total_msgs;
my %from_intern_size;
my %from_intern_msgs;
my $from_intern_size;
my $from_intern_msgs;
my %from_de_size;
my %from_de_msgs;
my $from_de_size;
my $from_de_msgs;
my %from_intl_size;
my %from_intl_msgs;
my $from_intl_size;
my $from_intl_msgs;

my $bouncemsgs;
my $unaccounted;
my $stat_host_unknown;
my $stat_ftn_unknown;
my $stat_service_unavail;
my $stat_user_unknown;



# 1st run: collect from=, size= data
while(<>) {
    chop;

    if( /^(.+) [a-zA-Z0-9_\-]+ sendmail\[\d+\]: ([A-Za-z0-9]+): (.*)$/ ) {
	$date = $1;
	$id   = $2;
	$rest = $3;

	if(!$first_date) {
	    $first_date = $1;
	    $output_date = substr($first_date, 0, 6);
	    $output_date =~ s/ /-/g;
	}
	$last_date  = $1;

	# clone
	if($rest =~ /^clone (\w+),/) {
	    print "# $id: $rest\n" if($opt_v);
	    $id_size{$id} = $id_size{$1};
	    $id_from{$id} = $id_from{$1};
	}

	# bounce message
	if($rest =~ /^([A-Za-z0-9]+): (.*?): (.*)$/) {
	    $dsn{$1} = "$id:$2:$3";
	    $dsn_count{$2}++;
	    print "DSN: $1, $id, $2, $3\n" if($opt_v);
	    next;
	}

	undef %entry;
	for $v (split(', ', $rest)) {
	    if( $v =~ /^(\w+)=(.*)$/ ) {
		$entry{$1} = $2;
	    }
        }

	# From entry
	if($entry{"from"}) {
	    ($user,$site) = &parse_addr($entry{"from"});
	    $size = $entry{"size"};
	    $id_size{$id} = $size;
	    $id_from{$id} = $site;
	}


	next if($entry{"stat"} =~ /^Queued/);
	next if($entry{"stat"} =~ /^Deferred/);

	# ruleset entry
	if($v = $entry{"ruleset"}) {
	    print "ruleset $v, from=$entry{\"arg1\"}\n" if($opt_v);
	    $ruleset_reject{$v}++;
	    $ruleset_count++;
	    next;
	}

	# To entry
	if($entry{"to"}) {
	    if($entry{"stat"} =~ /^Host unknown/) {
		$stat_host_unknown++;
		$stat_ftn_unknown++ if($entry{"mailer"} =~ /^ftn/);
		next;
	    }
	    if($entry{"stat"} =~ /^User unknown/) {
		$stat_user_unknown++;
		next;
	    }
	    if($entry{"stat"} =~ /^Service unavailable/) {
		$stat_service_unavail++;
		next;
	    }
	    if(! $entry{"stat"} =~ /^Sent/) {
		print "unknown stat=$entry{\"stat\"} for mail $id to=$entry{\"to\"}\n" if($opt_v);
		next;
	    }		

	    ($user,$site) = &parse_addr($entry{"to"});
	    $to   = $site;
	    if($size = $id_size{$id}) {
		$from = $id_from{$id};
	    }
	    elsif($v = $dsn{$id}) {
		print "bounce msg $id = $dsn{$id}\n" if($opt_v);
		$bouncemsgs++;
		next;
	    }
	    else {
		print "no from= for mail $id to=$entry{\"to\"}\n" if($opt_v);
		$unaccounted++;
		next;
	    }

	    # To Fido.DE sites
	    if($site =~ /fido\.de$/i) {
		##FIXME: not very generic
		$site = "morannon.fido.de" if($site =~ /^fido\.de$/i);
		$site =~ s/^p\d+\.//i;
		$site =~ s/\.fido\.de$//i;
		$to_total_size{$site} += $size;
		$to_total_msgs{$site} ++;
		$to_total_size        += $size;
		$to_total_msgs        ++;
		if($from =~ /\.fido\.de$/i || $from =~ /\.z2\.fidonet\.org$/i){
		    $to_intern_size{$site} += $size;
		    $to_intern_msgs{$site} ++;
		    $to_intern_size        += $size;
		    $to_intern_msgs        ++;
		}
		elsif($from =~ /\.de$/i) {
		    $to_de_size{$site} += $size;
		    $to_de_msgs{$site} ++;
		    $to_de_size        += $size;
		    $to_de_msgs        ++;
		}
		else {
		    $to_intl_size{$site} += $size;
		    $to_intl_msgs{$site} ++;
		    $to_intl_size        += $size;
		    $to_intl_msgs        ++;
		}
	    }

	    # From Fido.DE sites
	    $site = $from;
	    if($site =~ /fido\.de$/i) {
		##FIXME: not very generic
		$site = "morannon.fido.de" if($site =~ /^fido\.de$/i);
		$site =~ s/^p\d+\.//i;
		$site =~ s/\.fido\.de$//i;
		$from_total_size{$site} += $size;
		$from_total_msgs{$site} ++;
		$from_total_size        += $size;
		$from_total_msgs        ++;
		if($to =~ /\.fido\.de$/i || $to =~ /\.z2\.fidonet\.org$/i) {
		    $from_intern_size{$site} += $size;
		    $from_intern_msgs{$site} ++;
		    $from_intern_size        += $size;
		    $from_intern_msgs        ++;
		}
		elsif($to =~ /\.de$/i) {
		    $from_de_size{$site} += $size;
		    $from_de_msgs{$site} ++;
		    $from_de_size        += $size;
		    $from_de_msgs        ++;
		}
		else {
		    $from_intl_size{$site} += $size;
		    $from_intl_msgs{$site} ++;
		    $from_intl_size        += $size;
		    $from_intl_msgs        ++;
		}
	    }

	}

    }
}



# output

my $txt = "Sendmail statistics: $first_date -- $last_date";
my $len = length($txt);

print
  " " x ((79-$len)/2), $txt, "\n",
  " " x ((79-$len)/2), "=" x $len, "\n";

print
  "\n",
  "------------------------------------ T O T A L -----------------------------\n";
printf
  "IN : %4d msgs %6.2f MB\nOUT: %4d msgs %6.2f MB\n",
  $to_total_msgs, $to_total_size / 1024. / 1024.,
  $from_total_msgs, $from_total_size / 1024. / 1024.;

print
  "\n",
  "----------------------------------- B O U N C E ----------------------------\n";

printf
  "     %4d bounce messages sent by sendmail\n", $bouncemsgs
  if($bouncemsgs);
#printf
#  "     %4d messages not included (due to log file cycling)\n", $unaccounted
#  if($unaccounted);

print
  "\n";
printf
  "     %4d messages rejected (anti-SPAM rulesets)\n\n", $ruleset_count
  if($ruleset_count);
printf
  "     %4d messages bounced (Host unknown)\n", $stat_host_unknown
  if($stat_host_unknown);
printf
  "          (including %d RFC->FTN gateway)\n", $stat_ftn_unknown
  if($stat_ftn_unknown);
printf
  "     %4d messages bounced (User unknown)\n", $stat_user_unknown
  if($stat_user_unknown);
printf
  "     %4d messages bounced (Service unavailable)\n", $stat_service_unavail
  if($stat_service_unavail);

print
  "\n\n",
  "(traffic stats below do NOT include bounced messages!)\n",
  "\n",
  "-------------------------------- I N B O U N D -----------------------------\n",
  "              FROM->           All       Fido.DE           .DE         Other\n",
  "TO-v                    #    bytes    #    bytes    #    bytes    #    bytes\n",
  "-------------------- ------------- ------------- ------------- -------------\n";

for $site (sort keys(%to_total_msgs)) {
    printf "%-20.20s", $site;
    printf " %4d %8d", $to_total_msgs{$site}, $to_total_size{$site};
    printf " %4d %8d", $to_intern_msgs{$site}, $to_intern_size{$site};
    printf " %4d %8d", $to_de_msgs{$site}, $to_de_size{$site};
    printf " %4d %8d", $to_intl_msgs{$site}, $to_intl_size{$site};
    print "\n";
}

print "-------------------- ------------- ------------- ------------- -------------\n";
print "TOTAL               ";
printf " %4d %8d", $to_total_msgs, $to_total_size;
printf " %4d %8d", $to_intern_msgs, $to_intern_size;
printf " %4d %8d", $to_de_msgs, $to_de_size;
printf " %4d %8d", $to_intl_msgs, $to_intl_size;

print
  "\n\n",
  "------------------------------ O U T B O U N D -----------------------------\n",
  "                TO->           All       Fido.DE           .DE         Other\n",
  "FROM-v                  #    bytes    #    bytes    #    bytes    #    bytes\n",
  "-------------------- ------------- ------------- ------------- -------------\n";

for $site (sort keys(%from_total_msgs)) {
    printf "%-20.20s", $site;
    printf " %4d %8d", $from_total_msgs{$site}, $from_total_size{$site};
    printf " %4d %8d", $from_intern_msgs{$site}, $from_intern_size{$site};
    printf " %4d %8d", $from_de_msgs{$site}, $from_de_size{$site};
    printf " %4d %8d", $from_intl_msgs{$site}, $from_intl_size{$site};
    print "\n";
}

print "-------------------- ------------- ------------- ------------- -------------\n";
print "TOTAL               ";
printf " %4d %8d", $from_total_msgs, $from_total_size;
printf " %4d %8d", $from_intern_msgs, $from_intern_size;
printf " %4d %8d", $from_de_msgs, $from_de_size;
printf " %4d %8d", $from_intl_msgs, $from_intl_size;
print "\n\n";


close(OUT) if($out_flag);

#!/usr/bin/perl
#
# $Id: logsendmail2.pl,v 4.2 2004/03/14 20:31:58 n0ll Exp $
#
# Gather statistics from sendmail V8 syslog output
#

use strict;

my $VERSION    = '$Revision: 4.2 $ ';
my $PROGRAM    = "logsendmail2";

our($opt_v, $opt_c, $opt_o, $opt_g, $opt_s, $opt_n, $opt_m);
use Getopt::Std;

# Common configuration for perl scripts 
#<INCLUDE config.pl>


my $NEWSGROUPS = "fido.de.lists";
my $SUBJECT    = "morannon.fido.de sendmail accounting report";

my $INEWS      = "/usr/bin/inews -h -S";
my $SENDMAIL   = "/usr/sbin/sendmail";


getopts('vc:o:g:s:nm:');

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

    # only work with <user@do.main> addresses
    if($addr =~ /^<(.+)@(.+)>$/ ) {
	return $2;
    }

    return;
}



my $first_date;
my $output_date;
my $last_date;
my $date;
my $id;
my $rest;
my $v;
my %entry;
my $to;
my $size;
my $from;
my $mailer;
my $local;

my %id_from;
my %id_to;
my %id_size;

my %domain_total;
my %domain_count;

my %bounce_msg;



# collect from=, size= / to= data
while(<>) {
    chomp;

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
	##FIXME: is this still needed???
	if($rest =~ /^clone (\w+),/) {
	    print "# $id: $rest\n" if($opt_v);
	    $id_size{$id} = $id_size{$1};
	    $id_from{$id} = $id_from{$1};
	}

	# bounce message
	if($rest =~ /^([A-Za-z0-9]+): (.*?): (.*)$/) {
##NOT USED
#	    $dsn{$1} = "$id:$2:$3";
#	    $dsn_count{$2}++;
#	    print "DSN: $1, $id, $2, $3\n" if($opt_v);
	    next;
	}

	if($rest =~ /<.+?@.+?>\.\.\. (.*)$/) {
	    $v = $1;
#	    print "bounce=$v\n";
	    $v =~ s/address .+?@.+? does not exist/address ... does not exist/;
	    $v =~ s/\[?\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}\]?/.../;
	    $bounce_msg{$v}++;
	}


	undef %entry;
	for $v (split(', ', $rest)) {
	    if( $v =~ /^(\w+)=(.*)$/ ) {
		$entry{$1} = $2;
	    }
        }


	# from= entry
	if($rest =~ /^from=/) {
	    if($entry{"from"}) {
		$from = lc parse_addr($entry{"from"});
		$from = "LOCAL" unless($from);
		$size = $entry{"size"};
		$id_size{$id} = $size;
		$id_from{$id} = $from;
	    }
	}

	# to= entry
	if($rest =~ /^to=/) {
	    next if($entry{"stat"} =~ /^Queued/);
	    next if($entry{"stat"} =~ /^Deferred/);

	    if($entry{"to"}) {
		if($entry{"stat"} =~ /^Host unknown/) {
#		    $stat_host_unknown++;
#		    $stat_ftn_unknown++ if($entry{"mailer"} =~ /^ftn/);
		    next;
		}
		if($entry{"stat"} =~ /^User unknown/) {
#		    $stat_user_unknown++;
		    next;
		}
		if($entry{"stat"} =~ /^Service unavailable/) {
#		    $stat_service_unavail++;
		    next;
		}
		if(! $entry{"stat"} =~ /^Sent/) {
		    print "unknown stat=$entry{\"stat\"} for mail $id to=$entry{\"to\"}\n" if($opt_v);
		    next;
		}
		
		$to     = lc parse_addr($entry{"to"});
		next unless($to);
		$mailer = lc $entry{"mailer"};
		$local  = $mailer eq "local" || $mailer eq "ftn"
		  || $mailer eq "ftni";
		$size   = $id_size{$id};
		$from   = $id_from{$id};

		if(!$from) {
		    print "no from= for mail $id to=$entry{\"to\"}\n" if($opt_v);
		    next;
		}

		if($local) {
		    $domain_total{$to} += $size;
		    $domain_count{$to} ++;
		}

	    }
	}	    


	# ruleset entry
	if($v = $entry{"ruleset"}) {
#	    print "ruleset $v, from=$entry{\"arg1\"}\n" if($opt_v);
##NOT USED
#	    $ruleset_reject{$v}++;
#	    $ruleset_count++;
	    next;
	}


    }
}



# output

my $txt = "Sendmail statistics: $first_date -- $last_date";
my $len = length($txt);

print
  $txt, "\n",
  "=" x $len, "\n\n";

sub domainsort {
    my ($da, $db) = ($a, $b);
    if($a =~ /\.([a-z0-9\-]+\.[a-z]+)$/) {
	$da = $1;
    }
    if($b =~ /\.([a-z0-9\-]+\.[a-z]+)$/) {
	$db = $1;
    }
    return $da cmp $db if($da ne $db);
    return -1          if($da eq $a && $db ne $b);
    return 1           if($db eq $b && $da ne $a);
    return $a  cmp $b;
}

sub printdomain {
    my ($a) = @_;

    if($a =~ /^([a-z0-9\-.]+?)(\.[a-z0-9\-]+\.[a-z]+)$/) {
	return sprintf "%-15s%-16s ", $1, $2;
    }
    return sprintf "%-16s%-16s", "", $a;
}


print
  "-> Domain                            #     bytes\n",
  "------------------------------------------------\n";

my $sum_total = 0;
my $sum_count = 0;

for $v (sort domainsort keys %domain_total) {
    printf "%s %5d %9d\n",
      printdomain($v), $domain_count{$v}, $domain_total{$v};
    $sum_total += $domain_total{$v};
    $sum_count += $domain_count{$v};
}

print
  "------------------------------------------------\n";
printf "%-32s %5d %9d\n",
  "", $sum_count, $sum_total;

print
  "\n",
  "Bounce messages\n",
  "--------------------------------------------------------------------\n";

for $v (sort keys %bounce_msg) {
    printf "%-62.62s %5d\n", $v, $bounce_msg{$v};
}

print
  "--------------------------------------------------------------------\n";


close(OUT) if($out_flag);

#!/usr/bin/perl -w
#
# $Id: areassucksync.pl,v 1.3 2001/03/04 17:58:51 mj Exp $
#
# Syncronize groups between areas.bbs, active, sucknewsrc, NNTP list
#

use strict;

my $VERSION = '$Revision: 1.3 $ ';
my $PROGRAM = "areasbbssync";

use Getopt::Std;
use FileHandle;
use IO::Socket;

<INCLUDE config.pl>



# command line options
use vars qw($opt_v $opt_h $opt_c $opt_A $opt_B $opt_S $opt_N $opt_P
	    $opt_n $opt_g);
getopts('vhc:A:B:S:N:P:n:g:');
die
  "usage:   areasbbssync [-v] [-c GATE.CONF]\n",
  "                      [-A ACTIVE] [-B AREAS.BBS] [-S SUCKNEWSRC]\n",
  "                      [-N NEWSSERVER] [-P PATTERN] [-g WILDCARD]\n",
  "\n",
  "options: -v              verbose\n",
  "         -h              this help\n",
  "         -c CONF         alternate config file\n",
  "         -A ACTIVE       alternate active file\n",
  "         -B AREAS.BBS    alternate areas.bbs file\n",
  "         -S SUCKNEWSRC   alternate sucknewsrc file\n",
  "         -N NEWSSERVER   alternate news (NNTP) server\n",
  "         -P PATTERN      must match pattern (regex)\n",
  "         -g GROUP.*,...  INN-style wildcard newsgroup pattern\n",
  "         -n N            max. N articles from newly sucked groups\n"
  if($opt_h);



# read config
my $CONFIG = $opt_c ? $opt_c : "<CONFIG_GATE>";
CONFIG_read($CONFIG);

my $ACTIVE	 = $opt_A ? $opt_A : CONFIG_get("NewsVarDir") . "/active";
my $AREAS 	 = $opt_B ? $opt_B : CONFIG_get("AreasBBS");
my $SUCK         = $opt_S ? $opt_S : "/var/lib/suck/sucknewsrc";
my $SERVER       = $opt_N ? $opt_N : "localhost";
my $PATTERN      = $opt_P ? $opt_P : "^(de)\\.";
my $MAX          = $opt_n ? $opt_n : 50;

my $FTNAFUTIL    = CONFIG_get("BinDir") . "/ftnafutil -b$AREAS listgwlinks";


if($opt_g) {
    $PATTERN = $opt_g;
    $PATTERN =~ s/\./\\./g;
    $PATTERN =~ s/,/|/g;
    $PATTERN =~ s/\*/.*/g;
    $PATTERN = "^($PATTERN)";

    print "PATTERN = $PATTERN\n" if($opt_v);
}



##### main ###################################################################

my %nntp_list_count;
my %nntp_list_type;
my %suck_list;
my %active_list_count;
my %active_list_type;
my %areas_list;

my $sock;
my $resp;
my ($group, $n, $type);
local(*F);
local(*P);


#----- get list of newsgroups from server ------------------------------------
print "Connecting to $SERVER:nntp ...\n" if($opt_v);
$sock = new IO::Socket::INET(PeerAddr => $SERVER,
			     PeerPort => "nntp",
			     Proto    => "tcp",
			     Type     => SOCK_STREAM)
  || die "$PROGRAM: can't open NNTP connection to $SERVER: $!\n";
$sock->autoflush(1);

$resp = <$sock>;
$resp =~ s/\cM?\cJ?$//;
print "Greeting: $resp\n" if($opt_v);
die "$PROGRAM: unexpected \"$resp\" from $SERVER\n"
  unless($resp =~ /^200 /);

print "Listing newsgroups ...\n" if($opt_v);				 
print $sock "list\r\n";
$resp = <$sock>;
$resp =~ s/\cM?\cJ?$//;
print "Reply: $resp\n" if($opt_v);

do {
    $resp = <$sock>;
    $resp =~ s/\cM?\cJ?$//;
#    print "list: $resp\n" if($opt_v);

    ($group, $n, undef, $type) = split(' ', $resp);
    if($group =~ /$PATTERN/) {
	print "nntp group $group\n" if($opt_v);
	$nntp_list_count{$group} = $n + 0;
	$nntp_list_type {$group} = $type;
    }
} while($resp && $resp ne ".");

close($sock);


#----- get newsgroups from sucknewsrc ----------------------------------------
open(F, "<$SUCK")
  || die "$PROGRAM: can't open $SUCK: $!\n";

while(<F>) {
    s/\cM?\cJ?$//;
    next if(/^\s*\#/);

    ($group, $n) = split(' ');
    if($group =~ /$PATTERN/) {
	print "suck group $group\n" if($opt_v);
	$suck_list{$group} = $n;
    }
}

close(F);


#----- get local active newsgroups -------------------------------------------
open(F, "<$ACTIVE")
  || die "$PROGRAM: can't open $ACTIVE: $!\n";

while(<F>) {
    s/\cM?\cJ?$//;
    next if(/^\s*\#/);

    ($group, undef, $n, $type) = split(' ');
    if($group =~ /$PATTERN/) {
	print "active group $group\n" if($opt_v);
	$active_list_count{$group} = $n + 0;
	$active_list_type {$group} = $type;
    }
}

close(F);


#----- get areas from areas.bbs ----------------------------------------------
open(P, "$FTNAFUTIL|")
  || die "$PROGRAM: can't open pipe to $FTNAFUTIL: $!\n";

while(<P>) {
    s/\cM?\cJ?$//;
    
    ($group, undef, $n) = split(' ');
    $group = lc($group);
    $group =~ tr/_/-/;		# FIXME: this really should be removed
    print "line=$_ / group=$group n=$n\n" if($opt_v);
    if($group =~ /$PATTERN/) {
	print "areas.bbs group $group\n" if($opt_v);
	$areas_list{$group} = $n;
    }
}

close(P)
  || die "$PROGRAM: error in pipe to $FTNAFUTIL: $!\n";



#----- check areas with link against sucknewsrc ------------------------------
for $group (sort keys %areas_list) {
    next unless($areas_list{$group} > 0);

    print "checking areas.bbs $group\n" if($opt_v);
    if($suck_list{$group}) {
	print "                   already in sucknewsrc\n" if($opt_v);
	$n = $suck_list{$group};
    }
    else {
	print "                   not in sucknewsrc\n" if($opt_v);
	$n = $nntp_list_count{$group};
	unless($n) {
	    print "                   not in NNTP list, skipping\n" if($opt_v);
	    next;
	}
	$n -= $MAX;
	$n = 1 if($n < 1);
    }

    ##FIXME: write directly to sucknewsrc
    print "$group $n\n";
}



exit 0;

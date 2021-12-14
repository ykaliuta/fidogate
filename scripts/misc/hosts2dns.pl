#!/usr/bin/perl
#
# $Id: hosts2dns.pl,v 4.2 2001/05/28 15:09:25 mj Exp $
#
# Convert FIDOGATE hosts list to DNS MX record
#

use strict;
use Getopt::Std;
use FileHandle;


my $PROGRAM = 'hosts2dns';
my $VERSION = '0.0 $Revision: 4.2 $ ';

#my $DEF_MX  = "10\tmorannon.fido.de;100\tmonster.informatik.uni-oldenburg.de";
my $DEF_MX  = "10\tmorannon.fido.de";
my $DEF_EXCL= "berlin\$";

# Common configuration for perl scripts 
<INCLUDE config.pl>



##### Command line ###########################################################
use vars qw($opt_v $opt_h $opt_c $opt_H $opt_M $opt_X);
getopts('vhc:H:M:X:');

if($opt_h) {
    print STDERR
      "\n",
      "$PROGRAM --- Convert FIDOGATE hosts list to DNS MX records\n",
      "\n",
      "usage:  $PROGRAM [-vh]\n",
      "          -v               verbose\n",
      "          -h               this help\n",
      "          -c CONFIG        FIDOGATE config file\n",
      "          -H HOSTS         FIDOGATE hosts file\n",
      "          -M \"P MX;P MX\"   list of MX\n",
      "          -X PATTERN       exclude hosts matching PATTERN\n",
      "\n";

    exit 1;
}

# read config
my $CONFIG = $opt_c ? $opt_c : "<CONFIG_GATE>";
CONFIG_read($CONFIG);
my $HOSTS  = $opt_H ? $opt_H : CONFIG_get("Hosts");
my $MX     = $opt_M ? $opt_M : $DEF_MX;
my $XPAT   = $opt_X ? $opt_X : $DEF_EXCL;



##### Main ###################################################################

my %hosts;
my $h;

print "Reading host list from $HOSTS ...\n" if($opt_v);
open(F, "<$HOSTS") || die "$PROGRAM: can't open $HOSTS: $!\n";
while(<F>) {
    chop;
    s/\s*\#.*$//;
    next if(/^\s*$/);
    (undef,$h) = split(' ');
    next if($h =~ /$XPAT/);
    $hosts{$h} = 1;
#    print "  host=$h\n" if($opt_v);
}
close(F);
print "Hosts: ", join(' ', sort keys %hosts), "\n" if($opt_v);

my @mx;
my $i;

@mx = split(/[;,]/, $MX);

for $h (sort keys %hosts) {
    for $i (0..$#mx) {
	$mx[$i] .= "." unless($mx[$i] =~ /\.$/);
	if($i == 0) {
	    print "$h\t";
	    print "\t" if(length($h) < 8);
	    print "IN\tMX\t", $mx[$i], "\n";
	}
	else {
	    print "\t\tIN\tMX\t", $mx[$i], "\n";
	}
    }
}



exit 0;

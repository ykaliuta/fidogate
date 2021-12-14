#!/usr/bin/perl
#
# $Id: ftntickpost.pl,v 4.1 1999/06/27 19:18:13 mj Exp $
#
# Postprocessor for TIC files to be run by ftntick -x.
# Currently only a skeleton.
#
 
require 5.000;

my $PROGRAM = "ftntickpost";
 
use strict;
use vars qw($opt_v $opt_c);
use Getopt::Std;
use FileHandle;

<INCLUDE config.pl>

getopts('vc:');

# read config
my $CONFIG = $opt_c ? $opt_c : "<CONFIG_GATE>";
CONFIG_read($CONFIG);



# usage
die "usage: $PROGRAM [-v] [-c CONFIG] FILE.TIC\n" if($#ARGV != 0);

# read TIC
my $tic_file = $ARGV[0];
my %tic_data = TIC_read($tic_file);


##### do something not very useful #####
for (sort keys %tic_data) {
    printf "%10s = %s\n", $_, $tic_data{$_};
}



exit 0;




##### Read TIC file ##########################################################

sub TIC_read {
    my($tic) = @_;

    local(*F);
    my %v;
    my ($key, $val);

    open(F, $tic)
      || die "$PROGRAM: can't open TIC file $tic: $!\n";

    while(<F>) {
	s/\cM?\cJ?$//;
	($key, $val) = split(' ', $_, 2);
	
	if($v{$key}) {
	    $v{$key} .= " $val";
	}
	else {
	    $v{$key} = $val;
	}
    }

    close(F);

    return %v;
}

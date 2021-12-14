#!/usr/bin/perl
#
# $Id: out-ls.pl,v 4.3 2001/01/04 20:03:43 mj Exp $
#
# This script lists the outbound of a FIDOGATE system
#

require 5.000;

my $PROGRAM = "out-ls";
 
use strict;
use Getopt::Std;
use FileHandle;

<INCLUDE config.pl>

use vars qw($opt_v $opt_c $opt_p);
getopts('vc:p');

# read config
my $CONFIG = $opt_c ? $opt_c : "<CONFIG_FFX>";
CONFIG_read($CONFIG);



my $OUTBOUND = "<BTBASEDIR>";
my $z;
my $out;

for $z (sort keys %CONFIG_zone) {
    $out = $OUTBOUND . "/" . (split(' ', $CONFIG_zone{$z}))[2];
    do_dir($z, $out) if(-d $out);
}

exit 0;




sub file2addr {
    my ($zone, $name) = @_;

    my $net   = hex(substr($name,  0, 4));
    my $node  = hex(substr($name,  4, 4));
    my $point = hex(substr($name, 17, 4));

    if($point != 0) {
	return "$zone:$net/$node.$point";
    }
    else {
	return "$zone:$net/$node";
    }
}



sub do_file {
    my ($zone, $dir, $file) = @_;

    my ($flavor, $isflo, $isout, $addr, $t, $s, $n);

    if($file =~ /pnt/) {
	$flavor = substr($file, 22, 3);
    }
    else {
	$flavor = substr($file, 9, 3);
    }
    $flavor =~ tr/a-z/A-Z/;
    $isflo  =  $file =~ /\..lo$/;
    $isout  =  $file =~ /\..ut$/;
    $addr   =  file2addr($zone, $file);

    print $flavor, " ";
    printf "%-45.45s (%s)\n", $addr . " " . "-" x 45, $file;

    if($isflo) {
	open(FLO, "$dir/$file")
	  || die "$PROGRAM: can't open $dir/$file: $!";
	$s = 0;
	$n = 0;
	while(<FLO>) {
	    s/\cM?\cJ$//;
	    next if( /^;/ );
	    $s += print_flo_entry( $dir, $_ );
	    $n++;
	}
	print "    ", ksize($s), "\n" if $n>1;
    }
    if($isout) {
	($s, $t) = size_time("$dir/$file");
	print "    ";
	print ksize($s),   "  ";
	print asctime($t), "  ";
	print "\n";
    }
}



sub print_flo_entry {
    my ($dir, $line) = @_;

    my ($type, $drive, $file, $short, $t, $s);

    $type  = substr($line, 0, 1);
    if($type =~ /[\#~^]/ ) {
	$line = substr($line, 1, length($line)-1);
    }
    else {
	$type = " ";
    }

    if($line =~ /^[A-Z]:\\/i) {
	$line  =~ tr/[A-Z\\]/[a-z\/]/;
	$drive =  substr($line, 0, 2);
	$file  =  substr($line, 2, length($line)-2);
	$file  =  $CONFIG_dosdrive{$drive}.$file;
    }
    else {
	$file  = $line;
    }
    $short =  $file;
    $short =~ s+^$dir/++;

    ($s, $t) = size_time($file);

    print "    ";
    print ksize($s),   "  ";
    print asctime($t), "  ";
    print $type, " ", $short, "\n";

    return $s;
}



sub asctime {
    my ($time) = @_;

    if($time eq "") {
	return "              ";
    }
    else {
	my ($yr, $mn, $dy, $h, $m, $s);

	($s,$m,$h,$dy,$mn,$yr) = localtime($time);

	return sprintf("%02d.%02d.%02d %02d:%02d", $dy,$mn+1,$yr%100,$h,$m);
    }
}



sub size_time {
    my ($file) = @_;

    return (stat($file))[7,9];
}



sub ksize{
    my ($size) = @_;

    my ($k);

    if($size eq "") {
	return "   N/A";
    }
    else {
	if($size == 0) {
	    $k = 0;
	}
	elsif($size <= 1024) {
	    $k = 1;
	}
	else {
	    $k = $size / 1024;
	}
	return sprintf("%5dK", $k);
    }
}



sub do_point_dir {
    my ($zone, $dir, $pdir) = @_;

    opendir(DIR, "$dir/$pdir")
      || die "$PROGRAM: can't open $dir/$pdir: $!";
    my @files = readdir(DIR);
    closedir(DIR);
    @files = sort(@files);

    for(@files) {
	if( /^0000[0-9a-f]{4}\.(.lo|.ut|bsy)$/ ) {
	    do_file($zone,$dir,"$pdir/$_");
	}
    }
}



sub do_dir {
    my ($zone, $dir) = @_;

    print "zone=$zone, dir=$dir\n" if($opt_v);

    opendir(DIR, $dir)
      || die "$PROGRAM: can't open $dir: $!";
    my @files = readdir(DIR);
    closedir(DIR);
    @files = sort(@files);

    for(@files) {
	if( /^[0-9a-f]{8}\.(.lo|.ut|bsy|\$\$.)$/ ) {
	    do_file($zone,$dir,$_);
	}
	if( !$opt_p && /^[0-9a-f]{8}\.pnt$/ ) {
	    do_point_dir($zone,$dir,$_);
	}
    }
}

#!/usr/local/bin/perl
#
# $Id: out-freq.pl,v 4.1 1996/06/06 15:59:26 mj Exp $
#
# FRequest files
#

# Configuration
$outdir  = "<OUTBOUND>/out";
$outzone = 2;
$defzone = 2;


require "getopts.pl";

&Getopts('vn');



# Convert FIDO address to Binkley outbound name base

sub node2file {

    local($addr) = @_;
    local($zone,$net,$node,$point);
    local($dir,$nn,$pnt);

    if($addr =~ /^(\d+)\/(\d+)$/) {
	$zone  = $defzone;
	$net   = $1;
	$node  = $2;
	$point = 0;
    }
    elsif ($addr =~ /^(\d+):(\d+)\/(\d+)$/) {
	$zone  = $1;
	$net   = $2;
	$node  = $3;
	$point = 0;
    }
    elsif($addr =~ /^(\d+)\/(\d+)\.(\d+)$/) {
	$zone  = $defzone;
	$net   = $1;
	$node  = $2;
	$point = $3;
    }
    elsif ($addr =~ /^(\d+):(\d+)\/(\d+)\.(\d+)$/) {
	$zone  = $1;
	$net   = $2;
	$node  = $3;
	$point = $4;
    }
    else {
	print STDERR "out-freq: can't parse address $addr\n";
	exit 1;
    }

    if($zone != $outzone) {
	$dir = sprintf("$outdir.%03x", $zone);
    }
    else {
	$dir = $outdir;
    }
    $nn = sprintf("%04x%04x", $net, $node);
    if($point) {
	$pnt = sprintf(".pnt/0000%04x", $point);
    }
    else {
	$pnt = "";
    }

    return "$dir/$nn$pnt";
}



# Main

if($#ARGV < 1) {
    print STDERR "usage: out-freq [-vn] Z:N/F.P FILE ...\n";
    exit 1;
}

$node = shift;

$base = &node2file($node);

if( -f "$base.bsy" ) {
    print STDERR "out-freq: node $node is busy, no changes.\n";
    next;
}

$old_flo = 0;
$old_flo = "clo" if( -f "$base.clo" );
$old_flo = "dlo" if( -f "$base.dlo" );
$old_flo = "flo" if( -f "$base.flo" );
$old_flo = "hlo" if( -f "$base.hlo" );
$old_out = 0;
$old_out = "cut" if( -f "$base.cut" );
$old_out = "dut" if( -f "$base.dut" );
$old_out = "out" if( -f "$base.out" );
$old_out = "hut" if( -f "$base.hut" );

$new_flo = "clo";
$new_out = "cut";

# Create/append to request file
$req = "$base.req";
print "$req\n" if($opt_v);

open(REQ, ">>$req") || die "out-freq: can't append to $req\n";
for $f (@ARGV) {
    $f =~ tr/a-z/A-Z/;
    print "  requesting $f\n" if($opt_v);
    print REQ "$f\r\n";
}
close(REQ);

# Rename outbound files for polling
if(!$old_flo) {
    if(!$old_out) {
	# No ?LO file yet - create.
	$new = "$base.$new_flo";

	print "$new\n" if($opt_v);
	open(FLO, ">$new") || die "out-freq: can't open $base.$new_flo\n";
	close(FLO);
	chmod(0666, "$new");
    }
}
elsif($old_flo ne $new_flo) {
    $old = "$base.$old_flo";
    $new = "$base.$new_flo";

    print "$old -> $new\n" if($opt_v);
    if(!$opt_n) {
	rename($old,$new)
	    || print STDERR "out-freq: rename $old -> $new failed.\n";
    }
}

if($old_out && $old_out ne $new_out) {
    $old = "$base.$old_out";
    $new = "$base.$new_out";
    
    print "$old -> $new\n" if($opt_v);
    if(!$opt_n) {
	rename($old,$new)
	    || print STDERR "out-freq: rename $old -> $new failed.\n";
    }
}

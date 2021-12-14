#!/usr/bin/perl
#
# $Id: out-manip.pl,v 4.1 1999/03/06 17:51:22 mj Exp $
#
# This script can change the flavor of outbound files and create empty
# FLO files.
#

# Configuration
$outdir  = "<BTBASEDIR>/out";
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
	print STDERR "out-manip: can't parse address $addr\n";
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

if($#ARGV < 0) {
    print STDERR "usage: out-manip [-vn] CMD Z:N/F.P ...\n";
    print STDERR "  CMD:   crash, direct, normal, hold, kill, poll\n";
    exit 1;
}

$cmd = shift;
$cmd =~ tr/[A-Z]/[a-z]/;

for $node (@ARGV) {

    $base = &node2file($node);

#    if( -f "$base.bsy" ) {
#	print STDERR "out-manip: node $node is busy, no changes.\n";
#	next;
#    }

    $old_flo = 0;
    if( -f "$base.clo" ) {
	$old_flo = "clo";
    }
    if( -f "$base.dlo" ) {
	$old_flo = "dlo";
    }
    if( -f "$base.flo" ) {
	$old_flo = "flo";
    }
    if( -f "$base.hlo" ) {
	$old_flo = "hlo";
    }
    $old_out = 0;
    if( -f "$base.cut" ) {
	$old_out = "cut";
    }
    if( -f "$base.dut" ) {
	$old_out = "dut";
    }
    if( -f "$base.out" ) {
	$old_out = "out";
    }
    if( -f "$base.hut" ) {
	$old_out = "hut";
    }

    if($cmd eq "poll") {
	$create_empty_flo = 1;
	$glob = "$base.\\\$\\\$?";
	@call = <${glob}>;
	if(@call) {
	    print "Removing call count @call\n" if($opt_v);
	    unlink(@call) unless($opt_n);
	}
    }

    if($cmd eq "crash" || $cmd eq "poll") {
	$new_flo = "clo";
	$new_out = "cut";
    }
    elsif($cmd eq "direct") {
	$new_flo = "dlo";
	$new_out = "dut";
    }
    elsif($cmd eq "normal") {
	$new_flo = "flo";
	$new_out = "out";
    }
    elsif($cmd eq "hold") {
	$new_flo = "hlo";
	$new_out = "hut";
    }
    elsif($cmd eq "kill") {
	print STDERR "out-manip: kill not yet implemented.\n";
	exit 1;
    }
    else {
	print STDERR "out-manip: unknown command $cmd.\n";
	exit 1;
    }

    # Rename outbound files
    if(!$old_flo) {
	if(!$old_out && $create_empty_flo) {
	    # No ?LO file yet - create.
	    $new = "$base.$new_flo";

	    print "$new\n" if($opt_v);
	    open(FLO, ">$new") || die "out-manip: can't open $base.$new_flo\n";
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
		|| print STDERR "out-manip: rename $old -> $new failed.\n";
	}
    }

    if($old_out && $old_out ne $new_out) {
	$old = "$base.$old_out";
	$new = "$base.$new_out";

	print "$old -> $new\n" if($opt_v);
	if(!$opt_n) {
	    rename($old,$new)
		|| print STDERR "out-manip: rename $old -> $new failed.\n";
	}
    }
}

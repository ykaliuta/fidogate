#!/usr/bin/perl
#
# $Id: areasbbssync.pl,v 1.1 2001/01/04 20:03:43 mj Exp $
#
# Syncronize groups in active (INN) and areas.bbs (FIDOGATE)
#

$VERSION = '$Revision: 1.1 $ ';
$PROGRAM = "areasbbssync";


if($#ARGV < 0) {
    die
	"usage:   areasbbssync [-v] [-c GATE.CONF] [-a Z:N/F.P]\n",
	"                      [-A ACTIVE] [-B AREAS.BBS]\n",
	"                      [-P PATTERN] [-N PATTERN]\n",
	"                      [-x] [-l] [-n] [-r] [-w]\n",
	"\n",
	"options: -v              verbose\n",
	"         -c CONF         alternate config file\n",
	"         -a Z:N/F.P      gateway address\n",
	"         -A ACTIVE       alternate active file\n",
	"         -B AREAS.BBS    alternate areas.bbs file\n",
	"         -P PATTERN      must match pattern\n",
	"         -N PATTERN      must not match pattern\n",
	"         -x              write ftnaf commands\n",
	"         -l              list new/deleted areas\n",
	"         -n              new areas only\n",
	"         -r              removed areas only\n",
	"         -w              complete new areas.bbs\n";
}


# Common configuration for perl scripts 
<INCLUDE config.pl>

require "getopts.pl";
&Getopts('vc:a:A:B:P:N:xlnrw');

# read config
$CONFIG      = $opt_c ? $opt_c : "<CONFIG_GATE>";
&CONFIG_read($CONFIG);


$ADDRESS = $opt_a ? $opt_a : "";
$ACTIVE	 = $opt_A ? $opt_A : &CONFIG_get("NewsVarDir")."/active";
$AREAS 	 = $opt_B ? $opt_B : &CONFIG_get("AreasBBS");
$PATTERN = $opt_P ? $opt_P : 
    "^(comp|de|humanities|misc|news|rec|sci|soc|talk|ger)\\.";
$NPATTERN = $opt_N ? $opt_N : 
    "\\.(binaries|binaer|dateien)\\.";

if($opt_w || $opt_x || $opt_l) {
    $ADDRESS || die "$PROGRAM: gateway address not specified (option -a)\n";
}
$ACTIVE || die "$PROGRAM: active file not specified\n";
$AREAS  || die "$PROGRAM: areas.bbs file not specified\n";


##### Main ###################################################################

print "$PROGRAM: reading active file $ACTIVE\n" if($opt_v);
open(A, "$ACTIVE") || die "$PROGRAM: can't open $ACTIVE\n";
while(<A>) {
    ($a) = split(' ', $_);
    $a =~ tr/[a-z\-]/[A-Z_]/;
    if( ($a =~ /$PATTERN/i)  && ! ($a =~ /$NPATTERN/i) ) {
	$areas_active{$a} = 1;
    }
}
close(A);

print "$PROGRAM: reading areas.bbs file $AREAS\n" if($opt_v);
open(A, "$AREAS") || die "$PROGRAM: can't open $AREAS\n";
$_ = <A>;
while(<A>) {
    ($dir,$a) = split(' ', $_, 3);
    if( ($a =~ /$PATTERN/i) ) {
	$areas_bbs{$a} = 1;
    }
}
close(A);


# find new groups (only in active, not in areas.bbs)
if(!$opt_r) {
    print "\nNEW areas:\n" if($opt_l);
    for $area (sort keys %areas_active) {
	if($areas_bbs{$area}) {
#	print "checking active: area $area OK\n" if($opt_v);
	}
	else {
	    print "checking active: area $area MISSING\n" if($opt_v);
	    ##FIXME: use NEW command for FIDOGATE 4.3##
	    print "ftnaf $ADDRESS create $area\n" if($opt_x);
	    print "  + $area\n" if($opt_l);
	    $areas_new{$area} = 1 if($opt_w);
	}
    }
}


# find removed groups (not in active, only in areas.bbs)
if(!$opt_n) {
    print "\nDELETED areas:\n" if($opt_l);
    for $area (sort keys %areas_bbs) {
	if($areas_active{$area}) {
#	print "checking areas.bbs: area $area OK\n" if($opt_v);
	}
	else {
	    print "checking areas.bbs: area $area REMOVED\n" if($opt_v);
	    # Works only with FIDOGATE 4.3 ftnaf
	    print "ftnaf $ADDRESS delete $area\n" if($opt_x);
	    print "  - $area\n" if($opt_l);
	    $areas_delete{$area} = 1 if($opt_w);
	}
    }
}


# output complete new areas.bbs
if($opt_w) {
    # read old one, remove deleted areas
    open(A, "$AREAS") || die "$PROGRAM: can't open $AREAS\n";
    $_ = <A>;
    print;
    while(<A>) {
	($dir,$a) = split(' ', $_, 3);
	if( ($a =~ /$PATTERN/i) ) {
	    print if(!$areas_delete{$a});
	}
	else {
	    print;
	}
    }
    close(A);

    # write new areas
    for $area (sort keys %areas_new) {
	print "#+ $area $ADDRESS\r\n";
    }
}

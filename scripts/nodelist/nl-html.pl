#!/usr/bin/perl
#
# $Id: nl-html.pl,v 4.1 1997/06/22 10:25:19 mj Exp $
#
# nl-html --- convert nodelist to HTML pages
#

require "./html-tmpl.pl";
require "getopts.pl";


if($#ARGV == -1) {
    print
	"usage:   nl-html [-vi]\n",
	"                 [-b BASENAME] [-t TITLE] [-d DATE] [-T TEMPLATE]\n",
	"                 [-r REGIONS] [-z ZONES] NODELIST.NNN ...\n\n",
	"options:   -v           verbose\n",
	"           -i           main is index.html [BASENAME_main.html]\n",
	"           -b BASENAME  create BASENAME_*.html files [nodelist]\n",
	"           -t TITLE     set title [Nodelist]\n",
	"           -d DATE      set date [from nodelist]\n",
	"           -T TEMPLATE  use HTML template [template.html]\n",
	"           -r R;R;R;R   detailed list for these regions only [all]\n",
	"           -z Z;Z;Z;Z   list these zones only\n";
    exit;
}


&Getopts('b:t:d:r:z:viT:');


$zone  = 2;
$net   = 0;
$node  = 0;

$basename = $opt_b ? $opt_b : "nodelist";
$title    = $opt_t ? $opt_t : "Nodelist";
$date     = $opt_d ? $opt_d : "999";
$template = $opt_t ? $opt_t : "template.html";



if($opt_r) {
    for $i (split(/[ ,:;]/, $opt_r)) {
	$list_region{$i} = 1;
    }
}
if($opt_z) {
    for $i (split(/[ ,:;]/, $opt_z)) {
	$list_zone{$i} = 1;
    }
}


$HtmlSubst{"TITLE"}  = $title;
$HtmlSubst{"FOOTER"} = "Compiled by nl-html: ".&HtmlDate();


##### Read start of template and write to file ###############################
sub NLTmplStart {
    local($FILE,$tmpl) = @_;
    local($flag) = 1;

    open(TMPL, "$tmpl") || die "nl-html: can't open $tmpl.\n";
    while(<TMPL>) {
	if( /^\s*%MAIN%\s*$/ ) {
	    $flag = 0;
	    next;
	}

	# Substitutions
	s/%([A-Z]+)%/$HtmlSubst{$1}/g;

	print {$FILE} $_ if($flag);
    }
    close(TMPL);
    return;
}


##### Read end of template and write to file #################################
sub NLTmplEnd {
    local($FILE,$tmpl) = @_;
    local($flag) = 0;

    open(TMPL, "$tmpl") || die "nl-html: can't open $tmpl.\n";
    while(<TMPL>) {
	if( /^\s*%MAIN%\s*$/ ) {
	    $flag = 1;
	    next;
	}

	# Substitutions
	s/%([A-Z]+)%/$HtmlSubst{$1}/g;

	print {$FILE} $_ if($flag);
    }
    close(TMPL);
    return;
}


##### Various HTML output routines ###########################################
sub NLIntroStart {
    return
	"<font size=-1><ul>\n";
}

sub NLIntroEnd {
    return
	"</ul></font>\n";
}

sub NLZone {
    return
	"<hr>\n".
	"<font size=+1><b>Zone $zone: $name</b></font>\n".
	"<p>\n".
	"+$phone $isdn, ZC: <i>$sysop</i> @ $addr<p>\n";
}

sub NLRegionStart {
    return
	"<dl>\n";
}

sub NLRegion {
    return
	"<hr>\n",
	"<b>Region $net: $name</b>\n",
	"<p>\n",
	"+$phone $isdn, RC: <i>$sysop</i> @ $addr<p>\n";
}

sub NLRegionEnd {
    return
	"</dl>\n";
}

sub NLNetStart {
    return
	"<font size=+1><b>Net $net: $name, $loc</b></font>\n".
	"<p>\n".
	"+$phone $isdn, NC: <i>$sysop</i> @ $addr<p>\n".
	"<dl>\n";
}

sub NLNetEnd {
    return
	"</dl>\n";
}

sub NLNet {
    local($flag,$netname) = @_;

    if($flag) {
	return
	    "<dt><a href=\"$netname\">Net $net</a>: $name, $loc\n".
	    "<dd>+$phone $isdn, NC: <i>$sysop</i> @ $addr\n";
    } else {
	return
	    "<dt>Net $net: $name, $loc\n".
	    "<dd>+$phone $isdn, NC: <i>$sysop</i> @ $addr\n";
    }
}

sub NLNode {
    local($flag) = @_;
    
    $b1 = $flag ? "<b>"    : "";
    $b0 = $flag ? "</b>"   : "";
    $p  = $flag ? "<p>\n"  : "";
    return
	"$p".
	"<dt>$b1$addr: $name, $loc$b0\n".
	"<dd>+$phone $isdn, Sysop: <i>$sysop</i>\n";
}



##### Utility ################################################################
sub RegionStart {
    $regionstart = 1;
    return &NLRegionStart;
}

sub RegionEnd {
    if($regionstart) {
	$regionstart = 0;
	return &NLRegionEnd;
    }
    return "";
}



sub NetStart {
    local($netname) = @_;

    $netstart = 1;

    if($regionok) {
	$netopen = 1;

	open(NET, ">$netname") || die "nl-html: can't open $netname.\n";
	
	print STDERR "\n" if($opt_v && $pending_cr);
	print STDERR "Writing $netname ...\n" if($opt_v);
	$pending_cr = 0;
	
	&NLTmplStart(NET,$template);
	print NET &NLNetStart;
    }
}


sub NetEnd {
    if($netstart) {
	$netstart = 0;
	$netopen  = 0;

	print NET &NLNetEnd;
	&NLTmplEnd(NET,$template);
	close(NET);
    }
}



##### MAIN ###################################################################

$namemain = $opt_i ? "index.html" : $basename."_main.html";
open(MAIN, ">$namemain") || die "nl-m4: can't open $namemain.\n";

print STDERR "Writing $namemain ...\n" if($opt_v);

&NLTmplStart(MAIN,$template);

$zonestart   = 0;
$regionstart = 0;
$netstart    = 0;
$netopen     = 0;
$astart      = 0;
$pending_cr  = 0;
$independent = 0;

$regionok    = $opt_r ? 0 : 1;

while(<>) {
    chop;
    s/\cM//g;

    ### nodelist day number ###
    if( /^;A .*list for (.+) -- Day number (\d+)/ ) {
	$date = "$2, $1" if(!$opt_d);
	# Add to footer
	$HtmlSubst{"FOOTER"} = 
	    "Source nodelist day $date<br>\n".
	    $HtmlSubst{"FOOTER"};
    }

    ### ;A comments ###
    if( $astart!=-1 && /^;A/ ) {
	if(!$astart) {
	    $astart = 1;
	    print MAIN &NLIntroStart();
	}
	s/^;A//;
	s/^ *$/<p>/;
	s/^ *\.\. /<li>/;
	print MAIN $_,"\n";
	next;
    }
    print MAIN &NLIntroEnd() if($astart == 1);
    $astart = -1;

    ### other comments ###
    next if ( /^;/ || /^\cZ/ );


    ### split input line ###
    ($key,$num,$name,$loc,$sysop,$phone,$bps,$flags) =
	split(',', $_, 8);

    $flags =~ s/,U[^,]/,U,/;
    if( $flags =~ /^(.*),U,(.*)$/ ) {
	$fflags = $1;
	$uflags = $2;
    }
    else {
	$fflags = $flags;
	$uflags = "";
    }
    $fflags =~ s/,/ /g;
    $uflags =~ s/,/ /g;
    $name   =~ s/_/ /g;
    $loc    =~ s/_/ /g;
    $sysop  =~ s/_/ /g;

    $f      = ($flags =~ /ISDN/) || ($flags =~ /V110/) || ($flags =~ /X75/);
    $isdn   = $f ? "(ISDN)" : "";


    ### And now the action ...
    if($key eq "Zone") {	# Zone line
	$zone = $num;
	$net  = $num;
	$node = 0;
	$addr = "$zone:$net/$node";
	
	printf STDERR "%-70s\n", "Zone $zone ($name) ... " if($opt_v);

	print MAIN &RegionEnd;

	$independent = 0;

	&NetEnd;

	print MAIN &NLZone;
    }
    elsif($key eq "Region") {	# Region line
	$net  = $num;
	$node = 0;
	$addr = "$zone:$net/$node";

	printf STDERR "%-70s\r", "Region $num ($name)" if($opt_v);
	$pending_cr = 1;

	$independent = 0;

	&NetEnd;

	print MAIN   &RegionEnd;
	print MAIN &NLRegion;
	print MAIN   &RegionStart;

	$regionok    = $list_region{$num} if($opt_r);
    }
    elsif($key eq "Host") {	# Host line
	$net = $num;
	$node = 0;
	$addr = "$zone:$net/$node";

	print MAIN "<p>\n" if($independent);
	$independent = 0;

	&NetEnd;

	$netname = $basename."_z".$zone."_n".$net.".html";
	printf MAIN &NLNet($regionok,$netname);

	&NetStart($netname);
    }
    elsif($key eq "Hub") {	# Hub line
	$node = $num;
	$addr = "$zone:$net/$node";

	if($netopen) {
	    print NET &NLNode(1);
	}
	elsif(!$netstart) {
	    print MAIN &NLNode(1);
	    $independent = 1;
	}
    }
    else {
	$node = $num;
	$addr = "$zone:$net/$node";

	if($netopen) {
	    print NET &NLNode(0);
	}
	elsif(!$netstart) {
	    print MAIN &NLNode(0);
	    $independent = 1;
	}
    }
}


print STDERR " " x 70, "\n" if($opt_v);

print MAIN &NLRegionEnd;
&NLTmplEnd(MAIN,$template);
close(MAIN);

&NetEnd;

#!/usr/bin/perl
#
# $Id: info2txt.pl,v 4.1 1997/07/25 21:01:30 mj Exp $
#
# Stripping control stuff from .info file, yielding plain text
#

while(<>) {
    if( /^\c_/ ) {		# Start of node
	$x = <>;
	next;
    }

    if( /^\* Menu:/ ) {		# Menu
	next;
    }

    if( /^Node: .*\c?.*$/ ) {	# Nodes
	next;
    }

    if( /^\(Indirect\)/ ) {
	next;
    }

    if( /\.info-\d+: \d+$/ ) {
	next;
    }

    print $_;
}

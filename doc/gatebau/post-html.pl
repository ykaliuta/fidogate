#!/usr/bin/perl -i
#
# post-html.pl --- post processor for sgml2html
#

require "getopts.pl";

&Getopts("");


while (<>) {
    s/&circ;/^/g;
    s/^mailto:(.*) <\/LI>/<a href="mailto:$1">&lt;$1&gt;<\/a> <\/li>/;

    print;
}

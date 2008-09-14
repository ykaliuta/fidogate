#!/usr/bin/perl
#
# Search latest (youngest) file from command line
# 

require "getopts.pl";
&Getopts('l');


$t = -1;
$l = "";

for $f (@ARGV) {
    if(-f $f) {
	($x,$x,$x,$x,$x,$x,$x,$x, $atime, $mtime, $ctime, $x,$x) = stat($f);
	if($mtime > $t) {
	    $t = $mtime;
	    $l = $f;
	}
    }
}

print $l;
print "\n" if($l && !$opt_l);

exit ($l ? 0 : 1);

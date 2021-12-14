#!/usr/bin/perl
#
# $Id: fb-filelist.pl,v 4.7 2004/08/22 20:19:09 n0ll Exp $
#
# Generate file list
#
# Copyright (C) 1990-2002
#  _____ _____
# |     |___  |   Martin Junius             <mj.at.n0ll.dot.net>
# | | | |   | |   Radiumstr. 18
# |_|_|_|@home|   D-51069 Koeln, Germany
#
$LIBDIR="<LIBDIR>";


##### Options ################################################################
require "getopts.pl";

&Getopts('e:p:n:f:hv');

if($opt_h) {
    print
	"usage:   fb-filelist [-vh] [-e EPILOG.TXT] [-p PROLOG.TXT]\n",
	"                     [-n NEW] [-f FILEAREA.CTL]\n\n",
	"options: -e EPILOG.TXT    set epilog for filelist\n",
	"         -p PROLOG.TXT    set prolog for filelist\n",
	"         -n NEW           only files of last NEW days\n",
	"         -f FILEAREA.CTL  use Maximus FILEAREA.CTL area list\n",
        "         -v               more verbose\n",
	"         -h               this help\n";
    exit 1;
}

if($opt_n) {
    # -n days
    $time_new = time() - $opt_n*60*60*24;
}

$PROLOG   = $opt_p ? $opt_p : "$LIBDIR/prolog.txt";
$EPILOG   = $opt_e ? $opt_e : "$LIBDIR/epilog.txt";

$FILEAREA = $opt_f ? $opt_f : "$LIBDIR/fareas.bbs";



##### Configuration ##########################################################

# Missing text
$MISSING  = "--- file description missing ---";

# Path translation: MSDOS drives -> UNIX path names
%dirs = (
    'c:', 'c:',
    'd:', 'd:',
    'e:', 'e:',
    'f:', 'f:',
    'g:', 'g:',
    'h:', '/home',
    'i:', '/var/spool',
    'p:', '/u1',
    'q:', '/u2',
);



$total_global = 0;


# Generate listing for one directory

sub list_dir {
    local($dir) = @_;
    local($first,$file,$desc,@files,%files_dir);
    local($total) = 0;

    # Read contents of directory

    if(! opendir(DIR, $dir)) {
	print "--- area missing ---\r\n\r\n\r\n";
	return 1;
    }

    @files = readdir(DIR);
    closedir(DIR);

    for $file (@files) {
	# ignore directories
	if( -d $file ) {
	    next;
	}
	# ignore .*
	if( $file =~ /^\./ ) {
	    next;
	}
	# ignore files.{bbs,bak,dat,dmp,idx}
	if( $file =~ /^files\.(bbs|bak|dat|dmp|idx|tr)$/ ) {
	    next;
	}
	# ignore index.{dir,pag}
	if( $file =~ /^index\.(dir,pag)$/ ) {
	    next;
	}
	$files_dir{$file} = 1;
    }

    # Read and print contents of files.bbs

    if(open(FILES, "$dir/files.bbs")) {
	while(<FILES>) {
	    s/\cM?\cJ$//;
	    # File entry
	    if( /^[a-zA-Z0-9]/ ) {
		($file, $desc) = split(' ', $_, 2);
		$file  =~ tr+A-Z\\+a-z/+;
		$files_dir{$file} = 2;
		$total += &list_file($dir, $file, $desc);
	    }
	    elsif( /^-:crlf/ ) {
		next;
	    }
	    elsif(!$time_new) {
		print $_,"\r\n";
	    }
	}
	close(FILES);
    }
    else {
	print "--- no files.bbs ---\r\n";
    }

    # Print files missing in files.bbs / add to files.bbs
    open(FILES, ">>$dir/files.bbs")
	|| die "filelist: can't open $dir/files.bbs\n";
    for $file (sort keys(%files_dir)) {
	if($files_dir{$file} == 1) {
	    $total += &list_file($dir, $file, $MISSING);
	    printf FILES "%-12s  %s\r\n", $file, $MISSING;
	}
    }
    close(FILES);

    $total_global += $total;
    print "\r\n    File area total:  ",&ksize($total),"\r\n";

    print "\r\n\r\n";

}



sub list_file {
    local($dir,$file,$desc) = @_;
    local($x,$size,$time);

    ($x,$x,$x,$x,$x,$x,$x, $size ,$x, $time ,$x,$x,$x) = stat("$dir/$file");

    if($time_new && $time<$time_new) {
	return 0;
    }

    printf "%-12s  %s %s  %s\r\n", $file, &asctime($time), &ksize($size),
                                   $desc;

    return $size;
}



sub asctime {
    local($time) = @_;

    if($time eq "") {
	return "        ";
    }
    else {
	local($yr, $mn, $dy, $h, $m, $s, $xx);

	($s,$m,$h,$dy,$mn,$yr,$xx,$xx,$xx) = localtime($time);

	return sprintf("%02d.%02d.%02d", $dy,$mn+1,$yr, $h,$m);
    }
}



sub ksize{
    local($size) = @_;

    local($k);

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
	return sprintf("%6dK", $k);
    }
}



sub dump_file {
    local($file) = @_;

    open(F, "$file") || die "filelist: can't open $file\n";
    while(<F>) {
	s/\cM?\cJ$//;
	next if( /^\cZ/ );
	print $_,"\r\n";
    }
    close(F);
}



##### Main #####

if(-f $PROLOG) {
    &dump_file($PROLOG);
}

if($opt_n) {
    print "******** Listing of all files newer than ";
    print $opt_n, " days ******************************\r\n\r\n";
}


# Read Maximus filearea.ctl

open(F, "$FILEAREA") || die "filelist: can't open $FILEAREA\n";

<F> if(!$opt_f);	# Ignore 1st line of fareas.bbs

while(<F>) {
    s/\cM?\cJ$//;

    if($opt_f) {
	# Maximus filearea.ctl
	s/^\s*//;
	s/\s*$//;
	next if( /^%/ );
	next if( /^$/ );

	($keyw,$args) = split(' ', $_, 2);
	$keyw =~ tr/[A-Z]/[a-z]/;

	if   ($keyw eq "area"    ) {
	    $area = $args;
	}
	elsif($keyw eq "fileinfo") {
	    $info = $args;
	}
	elsif($keyw eq "download") {
	    $dir  =  $args;
	    $dir  =~ tr/[A-Z\\]/[a-z\/]/;
	    $drv  =  substr($dir, 0, 2);
	    $dir  =  substr($dir, 2, length($dir)-2);
	    $dir  =  $dirs{$drv}.$dir;
	}
	elsif($keyw eq "end"     ) {
	    $args =~ tr/[A-Z]/[a-z]/;
	    if($args eq "area") {
		$length = 50;
		if(length($info) > $length) {
		    $info = substr($info, 0, $length);
		}
		$length -= length($info);
		printf "### MAX file area \#%-3d ### ", $area;
		    print $info, " ", "#" x $length, "\r\n";
		print "\r\n";
		&list_dir($dir);
	    }
	}
    }
    else {
	# FIDOGATE fareas.bbs
	($dir,$area) = split(' ', $_);
	$dir =~ s/^\#//;

	if(! $dir_visited{$dir}) {
	    $dir_visited{$dir} = 1;
	    $header = "### File area $area ###";
	    $length = 78 - length($header);
	    print "$header", "#" x $length, "\r\n\r\n";
	    &list_dir($dir);
	}
    }

}

close(F);

print "########################################",
      "######################################\r\n";
print "\r\n      Listing total:  ",&ksize($total_global),"\r\n\r\n";
print "########################################",
      "######################################\r\n";

print "\r\n";
print "This list generated by:\r\n";
print '$Id: fb-filelist.pl,v 4.7 2004/08/22 20:19:09 n0ll Exp $', "\r\n";

if(-f $EPILOG) {
    &dump_file($EPILOG);
}

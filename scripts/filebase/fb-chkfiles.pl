#!/usr/local/bin/perl
#
# Check files.bbs
#

require "getopts.pl";

&Getopts('m');

if( $#ARGV == -1 ) {
    $dir = ".";
}
else {
    $dir = $ARGV[0];
}


# Read contents of directory

opendir(DIR, $dir)               || die "Can't open $dir";
@files = readdir(DIR);
closedir(DIR);

foreach $file (@files) {
    # ignore ., ..
    if( $file eq "." || $file eq ".." ) {
	next;
    }
    # ignore files.{bbs,bak,dat,dmp,idx,tr}
    if( $file =~ /^files\.(bbs|bak|dat|dmp|idx|tr)$/ ) {
	next;
    }
    $files_dir{$file} = 1;
}

# Read contents of files.bbs

open(FILES, "$dir/files.bbs")    || die "Can't open $dir/files.bbs";
while(<FILES>) {
    # File entry
    if( /^[a-zA-Z0-9]/ ) {
	($file, $desc) = split(' ', $_, 2);
	$file  =~ tr+A-Z\\+a-z/+;
	if($files_bbs{$file}) {
	    printf "Dupe:  %-12s  %s", $file, $desc;
	    printf " Old:                %s", $files_bbs{$file};
	}
	$files_bbs{$file} = $desc;
    }
}
close(FILES);

# Look for files missing from files.bbs

foreach $file (sort keys(%files_dir)) {
    if(! $files_bbs{$file} ) {
	printf "%-12s  ----- file description missing -----\r\n", $file;
    }
}

# Look for files missing in directory

foreach $file (sort keys(%files_bbs)) {
    if(! $files_dir{$file} ) {
	printf "No such file:  %-12s  %s", $file, $files_bbs{$file};
    }
}

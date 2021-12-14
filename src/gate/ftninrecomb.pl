#!/usr/bin/perl
#
# $Id: ftninrecomb.pl,v 4.5 1998/09/23 19:23:15 mj Exp $
#
# Recombine split mail and news messages.
#
 
require 5.000;

my $PROGRAM = "ftninrecomb";
 
#use strict;
use vars qw($opt_v $opt_c);
use Getopt::Std;
use FileHandle;

<INCLUDE config.pl>

getopts('vc:');

# read config
my $CONFIG = $opt_c ? $opt_c : "<CONFIG_GATE>";
CONFIG_read($CONFIG);

# options
my $options = "";

my $libdir   = CONFIG_get("LIBDIR");
my $maildir  = CONFIG_get("OUTRFC_MAIL");
my $newsdir  = CONFIG_get("OUTRFC_NEWS");
my $newsseq  = "seq.news";



#
# main program:
#

unsplit_mail();
split_newsfiles();
unsplit_news();

##################
#                #
#  Subroutines:  #
#                #
##################

#
# Unsplit mails:
#

sub unsplit_mail {
    #
    # generate databases of mails:
    #        (Splitline $; Part $; Parts) -> Filename
    #
    print "Reading mail file\n" if($opt_v);
    
    undef %msglist;
    opendir (DIR, "$maildir") || die "Couldn't open Dir $maildir\n";
    @messages = grep (/\.msg$/, readdir (DIR));
    closedir (DIR);

    MESSAGE:			
    for $f (@messages) {
	$msgfile = "$maildir/$f";

	print "Processing $msgfile\n" if ($opt_v);

	#
	# open message
	#
	if (!open(MESSAGE, $msgfile)) {
	    print STDERR "Can't open $msgfile -- continuing...\n";
	    next MESSAGE;
	}

	#
	# search "X-SPLIT:"
	#
	while (($_=<MESSAGE>) && (!/^X-SPLIT:/)) {
	}
	
	if (!/^X-SPLIT:/) {
	    #
	    # Messages was not splitted.
	    #
	    next MESSAGE;
	}
	
	# e.g.:
	# X-SPLIT: 30 Mar 90 11:12:34 @494/4       123   02/03 +++++++++++
	# (GIGO is broken, thus only 10 +)
	/^X-SPLIT: (.*) (\d\d)\/(\d\d) \+{10,}/;
	$id = ($1.$;.$2.$;.$3);
	
	print "ID: $id\n" if($opt_v);

	#
	# $id has now the format Splitline $; AktPart $; Parts
	#

	if ($msglist{$id}) {
	    #
	    # message is already in the database :-(
	    #
	    print "$msgfile seems to be a dupe. renaming to $msgfile.dupe\n";
	    rename ($msgfile, "$msgfile.dupe");
	}				
	else {			
	    #
	    # insert message to database
	    #
	    $msglist{$id} = $msgfile;
	}
    }

    #
    # Now walk through all mails and concatenate the parts
    #
    print "Processing split mails\n" if($opt_v);

    MAIL:
    foreach $aktmail (sort keys(%msglist)) {
	print "Processing mail file $msglist{$aktmail}\n" if($opt_v);

	# 
        # walk through the complete database.
	# 
	if (! -f $msglist{$aktmail}) {
	    next MAIL;
	}

	($splitid,$part,$parts) = split(/$;/,$aktmail);

	#
	# Test completeness of current message
	#
	$complete = 1;
	for ($p=1; $p <= $parts; $p++) { 
	    $part = sprintf ("%02d",$p);
	    if (! $msglist{$splitid.$;.$part.$;.$parts}) {
		$complete = 0;
		print "missing: $splitid  $part / $parts\n";
	    }
	}
	
	#
	# Now the unsplit
	#
	
	if ($complete) {
	    #
	    # All parts of the message exists.
	    #
	    print "Message $splitid complete, recombining\n" if($opt_v);

	    #
	    # process first message
	    #
	    $sp = $msglist{$splitid.$;.'01'.$;.$parts};
	    $nsp = $sp;
	    $usp = $sp.'.unsplit'; 

	    print "Part 01/$parts: $sp\n" if($opt_v);

	    open (SPLIT, "< $sp") || die "Couldn't open $sp\n";
	    open (UNSPLIT, "> $usp") || die "Couldn't open $usp\n"; 
	    
	    $linesline=0;	# the line in @all, where "Lines:" is found
	    undef @all;		# clean @all
	    
	    while (($_ = <SPLIT>) && (! /^$/) && (! /^Lines:/)) {
		push (@all,$_);
		$linesline++;
	    }
	    if (/^Lines: (\d+)$/) {
		$lines = $1;
	    }
	    push (@all, $_);
	    
	    while (($_ = <SPLIT>) && (! /^$/) && (! /^X-SPLIT:/)) {
		push (@all,$_);
	    }
	    if (!/^X-SPLIT:/) {
		#
		# delete "X-SPLIT:"-line:
		#
		push (@all,$_);
	    }

	    push (@all,<SPLIT>);
	    
	    close $SPLIT;
	    unlink $sp || die "Couldn't unlink $sp\n";
	    
	    #
	    # process the rest
	    #
	    for ($p=2; $p <= $parts; $p++) {
		$part = sprintf ("%02d",$p);
		$sp = $msglist{$splitid.$;.$part.$;.$parts};

		print "Part $part/$parts: $sp\n" if($opt_v);

		open (SPLIT, "< $sp") || die "Couldn't open $sp\n";
		while (($_ = <SPLIT>) && (! /^$/)) {
		    if (/^Lines: (\d+)$/) {
			$lines += $1;
		    }
		}
		push (@all,<SPLIT>);
		close $SPLIT;
		unlink $sp || die "Couldn't unlink $sp\n";
	    }
	    
	    @all[$linesline]="Lines: $lines\n";	# set "Lines:" to correct value
	    
	    #
	    # Output the recombined mail to file
	    #
	    foreach $line (@all) {
		print UNSPLIT $line;
	    }
	    close $UNSPLIT;
	    rename ($usp,$nsp) || die "Can't rename $usp -> $nsp\n";
	}
	else {
	    #
	    # cannot unsplit message because of missing parts.
	    #
	    print "Message <$msgid> incomplete!\n";
	}
    }
}    # end sub unsplit_mail




#
# split newsfiles:
#

sub split_newsfiles {
    opendir (DIR, "$newsdir") || die "Couldn't open Dir $maildir\n";
    @messages = grep (/\.msg$/, readdir (DIR));
    closedir (DIR);

    for $f (@messages) {
	$inname = "$newsdir/$f";
	&split_newsfile($inname);
    }	
}    # end sub split_newsfiles


sub split_newsfile {
    my ($inname) = @_;

    $nosplitname = sprintf ("$newsdir/%08ld.msg", &sequencer);
    open (NOSPLIT, "> $nosplitname") || die "Can't write $nosplitname\n";

    undef $rnews;

    open(IN, $inname) || die "Can't read $inname\n";
    while (<IN>) {
	$bytes = $_;
	chop $bytes;
	$bytes =~ s/^\#\! rnews (\d+)$/$1/;
	read (IN, $message, $bytes);
	# search "^X-SPLIT: " in the Header(!) :
	# X-SPLIT: 30 Mar 90 11:12:34 @494/4       123   02/03 +++++++++++
	if ($message =~ 
	    /\nX-SPLIT: [^\n\@]+\@\d+\/\d+[ \t]+\d+[ \t]+\d\d\/\d\d \+{11}/)
	{
	    #
	    # "X-SPLIT" found:
	    #
	    $message =~ /(.*)\n\n(.*)/;
	    $splitline = $1;
	    if (! $splitline =~ /X-SPLIT: /) {
		#
		# "X-SPLIT" not in Header!
		#
		print NOSPLIT "\#! rnews $bytes\n";
		print NOSPLIT $message;
	    } else {
		#
		# "X-SPLIT" found in Header
		#
		$outname = sprintf ("$newsdir/%08ld.msg.split", &sequencer);
		open (OUT, "> $outname") || die "Can't write $outname\n";
		print OUT "\#! rnews $bytes\n";
		print OUT $message;
		close OUT;
	    }
	} else {		
	    #
	    # Unsplitted Message
	    #
	    print NOSPLIT "\#! rnews $bytes\n";
	    print NOSPLIT $message;
	}
    }	
    close NOSPLIT;

    unlink $inname || die "Couldn't unlink $inname\n";} 
# end sub split_newsfile




#
# Unsplit news:
#

sub unsplit_news {

    #
    # generate databases of news:
    #        (Splitline $; Part $; Parts) -> Filename
    #
    
    undef %msglist;
    opendir (DIR, "$newsdir") || die "Couldn't open Dir $newsdir\n";
    @messages = grep (/\.msg\.split$/, readdir (DIR));
    closedir (DIR);



    NMESSAGE:			
    for $f (@messages) {
	$msgfile = "$newsdir/$f";

	#
	# open message
	#
	if (!open(MESSAGE, $msgfile)) {
	    print STDERR "Can't open $msgfile -- continuing...\n";
	    next NMESSAGE;
	}

	#
	# search "X-SPLIT:"
	#
	while (($_=<MESSAGE>) && (!/^X-SPLIT:/)) {
	}

	# e.g.:
	# X-SPLIT: 30 Mar 90 11:12:34 @494/4       123   02/03 +++++++++++
	#
	/^X-SPLIT: (.*) (\d\d)\/(\d\d) \+{11}/;
	$id = ($1.$;.$2.$;.$3);
	
	#print "id: $id\n";

	#
	# $id has now the format Splitline $; AktPart $; Parts
	#

	if ($msglist{$id}) {
	    #
	    # message is already in the database :-(
	    #
	    print "$msgfile seems to be a dupe. renaming to $msgfile.dupe\n";
	    rename ($msgfile, "$msgfile.dupe");
	}				
	else {			
	    #
	    # insert message to database
	    #
	    $msglist{$id} = $msgfile;
	}
    }

    #
    # Now walk through all news and concatenate the parts
    #
    
    NEWS:
    foreach $aktmail (sort keys(%msglist)) {
	# 
        # walk through the complete database.
	# 
	if (! -f $msglist{$aktmail}) {
	    next NEWS;
	}

	($splitid,$part,$parts) = split(/$;/,$aktmail);

	#
	# Test completeness actual message
	#
	$complete = 1;
	for ($p=1; $p <= $parts; $p++) { 
	    $part = sprintf ("%02d",$p);
	    if (! $msglist{$splitid.$;.$part.$;.$parts}) {
		$complete = 0;
		print "missing: $splitid  $part / $parts\n";
	    }
	}
	
	#
	# Now the unsplit
	#
	
	if ($complete) {
	    #
	    # All parts of the message exists.
	    #

	    #
	    # process first message
	    #
	    $sp = $msglist{$splitid.$;.'01'.$;.$parts};
	    $usp = $sp; 
	    $usp =~ s/\.split$//;
	    open (SPLIT, "< $sp") || die "Couldn't open $sp\n";
	    open (UNSPLIT, "> $usp") || die "Couldn't open $usp\n"; 
	    
	    $linesline=0;	# the line in @all, where "Lines:" is found
	    undef @all;		# clean @all
	    
	    while (($_ = <SPLIT>) && (! /^$/) && (! /^Lines:/)) {
		push (@all,$_);
		$linesline++;
	    }
	    if (/^Lines: (\d+)$/) {
		$lines = $1;
	    }
	    push (@all, $_);
	    while (($_ = <SPLIT>) && (! /^$/) && (! /^X-SPLIT:/)) {
		push (@all,$_);
	    }
	    if (!/^X-SPLIT:/) {	
		#
		# delete "X-SPLIT:"-line:
		#
		push (@all,$_);
	    }

	    push (@all,<SPLIT>);
	    
	    close $SPLIT;
	    unlink $sp || die "Couldn't unlink $sp\n";
	    
	    #
	    # process the rest
	    #
	    for ($p=2; $p <= $parts; $p++) {
		$part = sprintf ("%02d",$p);
		$sp = $msglist{$splitid.$;.$part.$;.$parts};
		open (SPLIT, "< $sp") || die "Couldn't open $sp\n";
		while (($_ = <SPLIT>) && (! /^$/)) {
		    if (/^Lines: (\d+)$/) {
			$lines += $1;
		    }
		}
		push (@all,<SPLIT>);
		close $SPLIT;
		unlink $sp || die "Couldn't unlink $sp\n";
	    }
	    
	    #$lines -= $p;
	    @all[$linesline]="Lines: $lines\n";	# set "Lines:" to correct value

	    $size = 0;
	    foreach $line (@all) {
		$size += length($line);
	    }
	    $size -= length($all[0]);
	    $all[0] =~ s/\#\! rnews \d+/\#\! rnews $size/;

	    #
	    # Output the unsplitted mail to file
	    #
	    foreach $line (@all) {
		print UNSPLIT $line;
	    }
	    close $UNSPLIT;
	}
	else {
	    #
	    # cannot unsplit message because of missing parts.
	    #
	    print "message <$msgid> incomplete!\n";
	}
    }
}    # end sub unsplit_news



# ----- Get number from seq.news ---------------------------------------------

sub sequencer {
    $nseq = `$libdir/ftnseq $options $newsseq`;
    die "Can't access $newsseq\n" if($nseq eq "ERROR" || $nseq eq "");

    return $nseq;
}

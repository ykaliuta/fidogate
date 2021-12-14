#!/usr/bin/perl
#
# $Id: rununpack.pl,v 4.10 1999/05/15 20:54:44 mj Exp $
#
# Unpack ArcMail archives
#
# Usage: rununpack name
#

require 5.000;

my $VERSION = '$Revision: 4.10 $ ';
my $PROGRAM = "rununpack";

use strict;
use vars qw($opt_v $opt_c);
use Getopt::Std;
use FileHandle;
## commented due to problems with RedHat 5.0 and 5.1
##use Sys::Syslog;

# Common configuration for perl scripts 
<INCLUDE config.pl>

my $BADDIR  = "bad";
my $TMPDIR  = "tmpunpack";
my $TMPOUT  = "tmpunpack.out";

my @arc_bindirs;
my %arc_l;
my %arc_x;
# Archiver programs configuration
# %X is replaced with settings from toss.conf
@arc_bindirs    = ( "/bin", "/usr/bin", "/usr/local/bin", "%N");
# %a is replaced with archive file name
$arc_l{"ARJ"}   = "unarj l    %a";
$arc_x{"ARJ"}   = "unarj e    %a";
$arc_l{"ARC"}   = "arc   l    %a";
$arc_x{"ARC"}   = "arc   eo   %a";
$arc_l{"ZIP"}   = "unzip -l   %a";
$arc_x{"ZIP"}   = "unzip -ojL %a";
$arc_l{"RAR"}   = "rar   l    %a";
$arc_x{"RAR"}   = "rar   e    %a";
$arc_l{"LHA"}   = "lha   l    %a";
$arc_x{"LHA"}   = "lha   eif  %a";
$arc_l{"ZOO"}   = "zoo   l    %a";
$arc_x{"ZOO"}   = "zoo   e:   %a";

getopts('vc:');

# read config
my $CONFIG = $opt_c ? $opt_c : "<CONFIG_MAIN>";
CONFIG_read($CONFIG);


my $PRG        = CONFIG_get("libdir");
my $SPOOL      = CONFIG_get("spooldir");
my $OUTBOUND   = CONFIG_get("btbasedir");
my $INBOUND    = CONFIG_get("inbound");
my $PINBOUND   = CONFIG_get("pinbound");
my $UUINBOUND  = CONFIG_get("uuinbound");
my $FTPINBOUND = CONFIG_get("ftpinbound");
my $LOGFILE    = CONFIG_get("logfile");

# syslog facility, level
my $FACILITY   = CONFIG_get("logfacility");
$FACILITY      = "local0" if(!$FACILITY);
my $LEVEL      = CONFIG_get("loglevel");
$LEVEL         = "notice" if(!$LEVEL);



if($#ARGV != 0) {
    die "usage: $PROGRAM NAME\n";
}
my $NAME = $ARGV[0];
my $INPUT;

# Set input and grade depending on NAME
if   ( $NAME eq "pin"  ) {
    $INPUT=$PINBOUND;
}
elsif( $NAME eq "in"   ) {
    $INPUT=$INBOUND;
}
elsif( $NAME eq "uuin" ) {
    $INPUT=$UUINBOUND;
}
elsif( $NAME eq "ftpin") {
    $INPUT=$FTPINBOUND;
}
elsif( $NAME =~ /^\/.+/ || $NAME =~ /^\.\/.+/ ) {
    $INPUT=$NAME;
}
else {
    die "$PROGRAM: unknown $NAME\n";
}

(-d $INPUT) || die "$PROGRAM: $INPUT: no such directory\n";



##### Log message ############################################################

my @month = ('Jan', 'Feb', 'Mar', 'Apr', 'May', 'Jun',
	     'Jul', 'Aug', 'Sep', 'Oct', 'Nov', 'Dec' );

sub log {
    my(@text) = @_;
    local(*F);
    my @x;
    
    print "$PROGRAM @text\n" if($opt_v);

    if($LOGFILE eq "syslog") {
	# syslog logging
## commented due to problems with RedHat 5.0 and 5.1
##	openlog($PROGRAM, 'pid', $FACILITY);
##	syslog($LEVEL, @text);
##	closelog();
    } else {
	# write to log file
	if($LOGFILE eq "stdout") {
	    open(F, ">&STDOUT") || die "$PROGRAM: can't open log $LOGFILE\n";
	}
	elsif($LOGFILE eq "stderr") {
	    open(F, ">&STDERR") || die "$PROGRAM: can't open log $LOGFILE\n";
	}
	else {
	    open(F, ">>$LOGFILE") || die "$PROGRAM: can't open log $LOGFILE\n";
	}
	
	@x = localtime;
	printf
	    F "%s %02d %02d:%02d:%02d ",
	    $month[$x[4]], $x[3], $x[2], $x[1], $x[0]; 
	print F "$PROGRAM @text\n";
	
	close(F);
    }
}



##### Determine archive type #################################################

sub arc_type {
    my($name) = @_;
    local(*F);
    my($buf, @b);

    sysopen(F, "$name", 0) || die "$PROGRAM: error reading $name\n";
    sysread(F, $buf, 6);
    @b = unpack("C6", $buf);
    close(F);

    return "ZIP" if($b[0]==ord('P') && $b[1]==ord('K') && $b[2]==3 &&$b[3]==4);
    return "ARC" if($b[0]==26);
    return "LHA" if($b[2]==ord('-') && $b[3]==ord('l') && $b[4]==ord('h'));
    return "ZOO" if($b[0]==ord('Z') && $b[1]==ord('O') && $b[2]==ord('O'));
    return "ARJ" if($b[0]==0x60 && $b[1]==0xea);
    return "RAR" if($b[0]==ord('R') && $b[1]==ord('a') && $b[2]==ord('r'));

    return "UNKNOWN";
}



##### Run program ############################################################

my $status = 0;				# Global status of last run_prog

sub run_prog {
    my($output, @args) = @_;
    my($rc);
    local(*SAVEOUT, *SAVEERR);

    open(SAVEOUT, ">&STDOUT") || die "$PROGRAM: can't save STDOUT\n";
    open(SAVEERR, ">&STDERR") || die "$PROGRAM: can't save STDERR\n";
    open(STDOUT,  ">$output") || die "$PROGRAM: can't open $output\n";
    open(STDERR,  ">&STDOUT") || die "$PROGRAM: can't dup STDOUT\n";

    $rc = system @args;
    $status = $rc >> 8;

    close(STDOUT);
    close(STDERR);
    open(STDOUT, ">&SAVEOUT") || die "$PROGRAM: can't restore STDOUT\n";
    open(STDERR, ">&SAVEERR") || die "$PROGRAM: can't restore STDERR\n";

    print "Status $status\n" if($opt_v);

    return $status == 0;
}



##### Run archive program ####################################################

sub run_arc {
    my($output, $cmd, $arc) = @_;
    my($prog, @args, $i, $d);

    $cmd =~ s/%a/$arc/g;
    @args = split(' ', $cmd);

    $prog = "";
    for $d (@arc_bindirs) {
	$d = CONFIG_expand($d);
	if(-x "$d/$args[0]") {
	    $prog = "$d/$args[0]";
	    last;
	}
    }
    return 0 if(!$prog);

    $args[0] = $prog;
    print "Run arc: @args\n" if($opt_v);
    
    return run_prog($output, @args);
}



##### Main ###################################################################

# Create necessary directories
(-d "$INPUT/$BADDIR")   || mkdir("$INPUT/$BADDIR", 0777);
(-d "$INPUT/$TMPDIR")   || mkdir("$INPUT/$TMPDIR", 0777);
chdir("$INPUT/$TMPDIR") || die "$PROGRAM: can't chdir to $INPUT/$TMPDIR\n";



# Process mail archives in $INPUT
my @files;
opendir(DIR, "$INPUT") || die "$PROGRAM: can't open $INPUT\n";
@files = grep(/\.(mo|tu|we|th|fr|sa|su).$/i, readdir(DIR));
closedir(DIR);

my $arc;
my $type;
my $cmd_l;
my $cmd_x;
my $ok;
my @xf;
my $f;
my $old;
my $new;
my $n;

for $arc (@files) {
    # Archive type
    $type = arc_type("$INPUT/$arc");
    if($type eq "UNKNOWN") {
	&log("unknown archive $INPUT/$arc, moving archive to $INPUT/$BADDIR");
	rename("$INPUT/$arc", "$INPUT/$BADDIR/$arc")
	    || die "$PROGRAM: can't rename $INPUT/$arc -> $INPUT/$BADDIR/$arc\n";
	next;
    }	
    &log("archive $INPUT/$arc ($type)");
    
    # List/extract program
    $cmd_l = $arc_l{$type};
    $cmd_x = $arc_x{$type};

    # Run list on archive, if it fails skip archive for now
    $ok = run_arc("/dev/null", $cmd_l, "$INPUT/$arc");
    print "List arc returned OK\n" if($opt_v && $ok);
    if(!$ok) {
	&log("WARNING: skipping archive $INPUT/$arc");
	next;
    }

    # Extract archive
    $ok = run_arc("$TMPOUT", $cmd_x, "$INPUT/$arc");
    print "Extract arc returned OK\n" if($opt_v && $ok);
    if(!$ok) {
	&log("ERROR: unpacking archive $INPUT/$arc failed");
	&log("ERROR: ouput of command $cmd_x:");
	open(F, "$TMPOUT") || die "$PROGRAM: can't open $TMPOUT\n";
	while(<F>) {
	    chop;
	    &log("ERROR:     $_");
	}
	close(F);

# 	&log("ERROR: removing extracted files");
# 	opendir(DIR, "$INPUT/$TMPDIR")
# 	    || die "$PROGRAM: can't open $INPUT/$TMPDIR\n";
# 	@xf = grep(/[^.].*/, readdir(DIR));
# 	closedir(DIR);
# 	unlink @xf || die "$PROGRAM: can't remove extracted files\n";

	&log("moving archive to $INPUT/$BADDIR");
	rename("$INPUT/$arc", "$INPUT/$BADDIR/$arc")
	    || die "$PROGRAM: can't rename $INPUT/$arc -> $INPUT/$BADDIR/$arc\n";
	next;
    }
    unlink($TMPOUT) || die "$PROGRAM: can't remove $TMPOUT\n";

    # Move extracted files to input directory
    opendir(DIR, "$INPUT/$TMPDIR")
	|| die "$PROGRAM: can't open $INPUT/$TMPDIR\n";
    @xf = grep(/[^.].*/, readdir(DIR));
    closedir(DIR);
    for $f (@xf) {
	$old = "$INPUT/$TMPDIR/$f";
	$new = "$INPUT/$f";
	$n   = 0;
	while(-f $new) {
	    $n++;
	    $new = "$INPUT/$n.$f";
	}
	&log("packet $f renamed to $n.$f") if($n);
	rename($old, $new) || die "$PROGRAM: can't rename $old -> $new\n";
    }

    # Remove archive
    unlink("$INPUT/$arc") || die "$PROGRAM: can't remove $INPUT/$arc\n";
}

exit 0;

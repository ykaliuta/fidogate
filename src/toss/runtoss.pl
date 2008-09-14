#!/usr/bin/perl
#
# $Id: runtoss.pl,v 4.9 1999/05/15 20:54:44 mj Exp $
#
# Wrapper for ftntoss, ftnroute, ftnpack doing the toss process
#
# Usage: runtoss name
#    or  runtoss /path/dir
#
 
require 5.000;

my $VERSION = '$Revision: 4.9 $ ';
my $PROGRAM = "runtoss";

use strict;
use vars qw($opt_v $opt_c);
use Getopt::Std;
use FileHandle;
## commented due to problems with RedHat 5.0 and 5.1
##use Sys::Syslog;

# Common configuration for perl scripts 
<INCLUDE config.pl>

getopts('vc:');

# read config
my $CONFIG = $opt_c ? $opt_c : "<CONFIG_MAIN>";
CONFIG_read($CONFIG);

# additional arguments for ftntoss/route/pack
my $ARGS  = "";
$ARGS .= " -c $opt_c" if($opt_c);
$ARGS .= " -v"        if($opt_v);


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

# Minimum free disk space required for tossing
my $MINFREE    = CONFIG_get("diskfreemin");
$MINFREE       = CONFIG_get("mindiskfree") if(!$MINFREE);
$MINFREE       = 10000                      if(!$MINFREE);
# Method for determining free disk space
#   prog = use DiskFreeProg
#   none = always assume enough free disk space
my $DFMETHOD   = CONFIG_get("diskfreemethod");
$DFMETHOD      = "prog" if(!$DFMETHOD);		# "prog" = Use DiskFreeProg
# df program, must behave like BSD df accepting path name
#   %p   = path name
my $DFPROG     = CONFIG_get("diskfreeprog");
$DFPROG        = "df -P %p" if(!$DFPROG);	# GNU fileutils df



if($#ARGV != 0) {
    die "usage: $PROGRAM NAME\n";
}
my $NAME = $ARGV[0];
my $INPUT;
my $FADIR;
my $GRADE;
my $FLAGS;

# Set input and grade depending on NAME
if   ( $NAME eq "pin"  ) {
    $INPUT=$PINBOUND;
    $FADIR="-F$INPUT";
    $GRADE="-gp";
    $FLAGS="-s";
}
elsif( $NAME eq "in"   ) {
    $INPUT=$INBOUND;
    $FADIR="-F$INPUT";
    $GRADE="-gi";
    $FLAGS="-s";
}
elsif( $NAME eq "uuin" ) {
    $INPUT=$UUINBOUND;
    $FADIR="";
    $GRADE="-gu";
    $FLAGS="-s";
}
elsif( $NAME eq "ftpin") {
    $INPUT=$FTPINBOUND;
    $FADIR="";
    $GRADE="-gf";
    $FLAGS="-s";
}
elsif( $NAME eq "outpkt") {
    $INPUT="$SPOOL/outpkt";
    $FADIR="";
    $GRADE="-go";
    $FLAGS="-n -t -p";
}
elsif( $NAME eq "outpkt/mail") {
    $INPUT="$SPOOL/outpkt/mail";
    $FADIR="";
    $GRADE="-gm";
    $FLAGS="-n -t -p";
}
elsif( $NAME eq "outpkt/news") {
    $INPUT="$SPOOL/outpkt/news";
    $FADIR="";
    $GRADE="-gn";
    $FLAGS="-n -t -p";
}
elsif( $NAME =~ /^\/.+/ || $NAME =~ /^\.\/.+/ ) {
    $INPUT=$NAME;
    $FADIR="-F$INPUT";
    $GRADE="";
    $FLAGS="-s";
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



##### Run df program #########################################################

sub df_prog {
    my($path) = @_;
    my(@args, @f, $free, $prog, $pid);
    local(*P);

    # df command, %p is replaced with path name
    $prog =  $DFPROG;
    $prog =~ s/%p/$path/g;
    @args =  split(' ', $prog);

    print "Running @args\n" if($opt_v);

    # ignore SIGPIPE
    $SIG{"PIPE"} = "IGNORE";

    # Safe pipe to df
    $pid = open(P, "-|");
    if ($pid) {   # parent
	while (<P>) {
	    chop;
	    next if(! /^\/dev\// );
	    @f = split(' ', $_);
	    $free = $f[3];
	}
	close(P);
    }
    else {        # child
	exec (@args) || die "$PROGRAM: can't exec df program: $!\n";
	die "$PROGRAM: impossible return from exec\n";
	# NOTREACHED
    }

    die "$PROGRAM: can't determine free disk space\n" if($free eq "");
    return $free;
}



##### Check for free disk space ##############################################

sub df_check {
    my($path) = @_;
    my($free);

    # Use method
    if($DFMETHOD eq "prog") {
	$free = df_prog($path);
    }
    elsif($DFMETHOD eq "none") {
	return 1;
    }
    else {
	return 1;
    }

    # Check against DiskFreeMin
    print "Free disk space in $path is $free K\n" if($opt_v);
    if($free < $MINFREE) {
	&log("disk space low in $path, $free K free");
	return 0;
    }
    return 1;
}



##### Run program ############################################################

my $status = 0;				# Global status of last run_prog

sub run_prog {
    my($cmd) = @_;
    my(@args, $rc);

    @args = split(' ', $cmd);
    $args[0] = "$PRG/$args[0]";
    print "Running @args\n" if($opt_v);
    $rc = system @args;
    $status = $rc >> 8;
    print "Status $status\n" if($opt_v);

    return $status == 0;
}



##### Main ###################################################################

# Check free disk space in SPOOL
df_check($SPOOL) || exit 1;

# Run ftntoss/ftnroute/ftnpack
my $flag = 1;

while($flag) {
    # Check free disk space in outbound (BTBASEDIR)
    df_check($OUTBOUND) || exit 1;

    # Toss
    run_prog("ftntoss -x -I $INPUT $GRADE $FLAGS $ARGS");
    if($status == 0) {		# Normal exit
	$flag = 0;
    }
    elsif($status == 2) {	# MSGID history or lock file busy
	sleep(60);
	$flag = 1;
	next;
    }
    elsif($status == 3) {	# Continue tossing
	$flag = 1;
    }
    else {			# Error
	die "$PROGRAM: ftntoss returned $status\n";
    }

    # Route
    run_prog("ftnroute $GRADE $ARGS");
    if($status != 0) {		# Error
	die "$PROGRAM: ftnroute returned $status\n";
    }

    # Pack
    run_prog("ftnpack $FADIR $GRADE $ARGS");
    if($status != 0) {		# Error
	die "$PROGRAM: ftnroute returned $status\n";
    }
}

exit 0;

#!/usr/bin/perl
#
# $Id: ffxrmail.pl,v 4.6 2000/11/18 17:57:56 mj Exp $
#
# sendmail frontend for processing ffx rmail commands
#

require 5.000;

my $PROGRAM = "ffxrmail";
 
use strict;
use vars qw($opt_v $opt_c $opt_f);
use Getopt::Std;
use FileHandle;

<INCLUDE config.pl>

getopts('vc:f:');

# read config
my $CONFIG = $opt_c ? $opt_c : "<CONFIG_FFX>";
CONFIG_read($CONFIG);

# configuration
my $SENDMAIL = CONFIG_get("FFXRmailSendmail");
die "ffxrmail:$CONFIG:FFXRmailSendmail not defined\n" if(! $SENDMAIL);
$SENDMAIL =~ s/-p[^ ]*/-pFFX:$opt_f/ if($opt_f);
print "SENDMAIL: $SENDMAIL\n" if($opt_v);

# ignore SIGPIPE
$SIG{"PIPE"} = "IGNORE";

# read From_ line from <STDIN>
my $from;
my @cmd;
$_ = <STDIN>;
if( /^From ([^ ]*) / ) {
    $from = $1;
    $from = "news\@localhost" if($from eq "");
}
else {
    die "ffxrmail: no From_ line at start of message\n";
}
print "ENVELOPE: $from\n" if($opt_v);

# sendmail command
undef @cmd;
push(@cmd, split(' ', $SENDMAIL));
push(@cmd, "-f");
push(@cmd, $from);
push(@cmd, @ARGV);

print "CMD: @cmd\n" if($opt_v);

# Safe pipe to sendmail
my $pid = open(PIPE, "|-");
if ($pid) {   # parent
  while (<STDIN>) {
    print PIPE $_;
  }
  close(PIPE);
  die "$PROGRAM: ERROR: close pipe to sendmail\n" if($?);
}
else {        # child
  exec (@cmd) || die "$PROGRAM: ERROR: can't exec $SENDMAIL program: $!\n";
  die "$PROGRAM: ERROR: impossible return from exec\n";
  # NOTREACHED
}


exit(0);

#!/usr/bin/perl -T

use strict;

use CGI;
use CGI::Carp;


$ENV{"PATH"} = "/bin:/usr/bin";
$ENV{"CDPATH"} = "";

my @DIRS = (
	    "<NEWSVARDIR>",
	    "/var/spool/news/outgoing",
	    "<VARDIR>",
	    "<LOCKDIR>",
	    "<SPOOLDIR>/outpkt -R",
	    "<BTBASEDIR>/in -R",
	    "<BTBASEDIR>/pin -R",
	    "<BTBASEDIR>/uuin -R",
	   );



##### Main ###################################################################

my $q = new CGI;
my @out;

my $size = "";
my $font = 'n';


print
  $q->header,
  $q->start_html(-title   => "morannon.fido.de Gateway Status",
		 -author  => 'mj.at.n0ll.dot.net',
		 -bgcolor => '#ffffff'    ), "\n\n",
  $q->h1("morannon.fido.de Gateway Status"), "\n",
  $q->b("Date: " . localtime(time)), $q->br, $q->br, $q->br,
  $q->hr, "\n";

$font = $q->param('font');
$size = "size=-1" if($font eq 's');
$size = ""        if($font eq 'n');


@out = my_run("uptime");
print
  "<b>Uptime</b>\n",
  "<font $size>\n",
  "<pre>",
  @out,
  "</pre>\n",
  "</font>\n";

print
  $q->hr, "\n";

@out = my_run("free");
print
  "<b>Free</b>\n",
  "<font $size>\n",
  "<pre>",
  @out,
  "</pre>\n",
  "</font>\n";

print
  $q->hr, "\n";

@out = my_run("who");
print
  "<b>Who</b>\n",
  "<font $size>\n",
  "<pre>",
  @out,
  "</pre>\n",
  "</font>\n";

print
  $q->hr, "\n";

@out = my_run("last -20");
print
  "<b>Last Logins</b>\n",
  "<font $size>\n",
  "<pre>",
  @out,
  "</pre>\n",
  "</font>\n";

print
  $q->hr, "\n";

@out = my_run("df");
print
  "<b>Disk Space</b>\n",
  "<font $size>\n",
  "<pre>",
  @out,
  "</pre>\n",
  "</font>\n";

print
  $q->hr, "\n";

@out = my_ps_news();
print
  "<b>News Processes</b>\n",
  "<font $size>\n",
  "<pre>\n",
  @out,
  "</pre>\n",
  "</font>\n";

my ($dir, $opts);

for (@DIRS) {
    ($dir, $opts) = split(' ');
    next if(! -d $dir);
    
    print
      $q->hr, "\n";
    @out = my_ls_l($dir, $opts);
    print
      "<b>Directory $dir</b>\n",
      "<font $size>\n",
      "<pre>",
      @out,
      "</pre>\n",
      "</font>\n";
}


print
  $q->hr, "\n",
  $q->p, "\n",
  '<font size=-2>FIDOGATE $Id: gate.cgi.pl,v 1.7 2004/08/22 20:19:09 n0ll Exp $ <br>',
  '&copy; Copyright 1990-2002 ',
  '<a href="mailto:mj.at.n0ll.dot.net">Martin Junius</a></font>',
  $q->end_html, "\n";

exit 0;



##### Run normal command and return output ###################################

sub my_run {
    my($cmd) = @_;

    my($tmp,@lines);
    local(*F);

    $tmp = "/tmp/gate.cgi.out.$$";
    system "$cmd >$tmp 2>&1";
    open(F, $tmp) || return ("ERROR");
    while(<F>) {
	push(@lines, $_);
    }
    close(F);
    unlink($tmp);
#    chop @lines;

    return @lines;
}



##### Run ps command and return post-processed output ########################

sub my_ps_news {
    my($tmp,@lines,$s,$x,$x1,$x2);
    local(*F);

#    $tmp = "/home/mj/psaux.morannon";
    $tmp = "/tmp/gate.cgi.out.$$";
    system "ps auxww >$tmp 2>/dev/null";
    open(F, $tmp) || return ("ERROR");
    while(<F>) {
	if( /^USER/ ) {
#	    push(@lines, "  PID %CPU %MEM  SIZE   RSS STAT START    TIME COMMAND\n");
	    push(@lines, "  PID %CPU %MEM   VSZ  RSS  STAT START    TIME COMMAND\n");
	    next;
	}
	next if(! /^news/ );

#	push(@lines, ">>>$_");

#  USER     (  PID %CPU %MEM   VSZ  RSS )TTY      STAT START   TIME COMMAND
#  news     (  580  0.0  0.2  5604  136 )?        S    10:18   0:01 /usr/bin/innd -p5
#  news     (  584  0.0  0.1  2536  100 )?        S    10:18   0:00 /usr/bin/actived
#/^.........(...........................)......*?([A-Z].*\d )( *\d+:\d\d).(.+)$/ );
	next if(! /^.........(...........................)......*?([A-Z].*\d )( *\d+:\d\d).(.+)$/ );
	$x  = "$1 $2";
	$x1 = $3;
	$x2 = $4;
	$x1 = " ". $x1 if(($x1 =~ /^ /) || ($x1 =~ /^\d\d\d:/));
	$x .= $x1;
	$_  = $x2;
	next if( /^time / );
	s/^sh //;
	s/^sh -c //;
	s/^perl //;
	s/^\/[a-zA-Z0-9\/.\-]*\///;
	$x = "$x $_";
	$x =~ s/^(.{1,110}).*$/$1/;
	push(@lines, "$x\n");
    }
    close(F);
    unlink($tmp);
#    chop @lines;

    return @lines;
}



##### Run ls -l command and return output ####################################

sub my_ls_l {
    my($dir, $opts) = @_;

    my($tmp,@lines);
    local(*F);

    @lines = ();

    $tmp = "/tmp/gate.cgi.out.$$";
    system "ls -lL $opts $dir >$tmp 2>/dev/null";
    open(F, $tmp) || return ("ERROR");
    while(<F>) {
	next if( /^total/ || /^d/ );
	s/ (\d{9}) /$1 /;
	push(@lines, $_);
    }
    close(F);
    unlink($tmp);
#    chop @lines;

    return @lines;
}

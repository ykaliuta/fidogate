#!/bin/sh
#
# $Id: runpoll.sh,v 4.15 2000/04/19 17:07:28 mj Exp $
#
# Generic poll script for FIDOGATE points
#

LIBDIR=<LIBDIR>
BINDIR=<BINDIR>
IFMAIL=<IFMAILDIR>

XTERM=/usr/X11R6/bin/xterm

UPLINK=f<NODE>.n<NET>.z2
#       ^^^^^^  ^^^^^
#       configure me!


# -xterm: run in XTerm window
if [ "$1" = "-xterm" ]; then
  exec $XTERM -display :0 -g 80x20 -title "FIDOGATE runpoll" -e $0
  exit 0
fi


# Show executed commands
set -x


# News gateway (INN)
$BINDIR/send-fidogate

# Tosser w/o file attachments
$BINDIR/runtoss outpkt
$BINDIR/runtoss outpkt/mail
$BINDIR/runtoss outpkt/news

# Poll
$IFMAIL/ifcico $UPLINK

# Tosser, only protected inbound
$BINDIR/rununpack pin
$BINDIR/runtoss   pin

# Gateway
$LIBDIR/ftnin -x %L/ftninpost

# Process tic files
$LIBDIR/ftntick

# Process mail queue (depending on your config, not strictly necessary)
/usr/sbin/sendmail -q

# Tosser expire
$LIBDIR/ftnexpire

#!/bin/sh
#
# $Id: runpoll-binkd.sh,v 4.7 2001/01/04 20:03:43 mj Exp $
#
# orodruin.fido.de's poll script using binkd
#

LIBDIR=<LIBDIR>
BINDIR=<BINDIR>

BINKD=/usr/sbin/binkd
BINKDCFG=/etc/binkd/binkd.cfg

XTERM=/usr/X11R6/bin/xterm

DUMMYADDR="nobody@fido.de"

BOSSNODE="242:1000/2"
BOSSNAME="morannon"


# -xterm: run in XTerm window
if [ "$1" = "-xterm" ]; then
  exec $XTERM -display :0 -g 100x20 -title "FIDOGATE runpoll (BinkD)" -e $0
  exit 0
fi


# Show executed commands
set -x

# Send dummy mail for polling
#mail -s "POLL" $DUMMYADDR <<EOF
#POLL
#EOF
#sleep 2

# Batch ffx news
$BINDIR/send-ffx

# Batch ffx mail
$LIBDIR/ftnpack -f $BOSSNODE -I %B/out.0f2/$BOSSNAME

# Gateway
$BINDIR/send-fidogate

# Tosser w/o file attachments
$BINDIR/runtoss outpkt
$BINDIR/runtoss outpkt/mail
$BINDIR/runtoss outpkt/news

# Poll (-P requires binkd 0.9.4)
$BINKD -P $BOSSNODE -p $BINKDCFG

# Tosser, only protected inbound
$BINDIR/rununpack pin
$BINDIR/runtoss   pin

# Gateway
$LIBDIR/ftnin -x %L/ftninpost

# Unbatch and process ffx files
$LIBDIR/ffxqt

# Process tic files
$LIBDIR/ftntick

# Process mail queue
/usr/sbin/sendmail -q

# Tosser expire
$LIBDIR/ftnexpire

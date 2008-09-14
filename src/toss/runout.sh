#!/bin/sh
#
# $Id: runout.sh,v 4.5 1998/04/07 12:22:00 mj Exp $
#
# Toss misc output (ftnafpkt, ftnoutpkt)
#
# Usage: runout
#

LOCK=runout

# Output to "log-out" log file
FIDOGATE_LOGFILE="%G/log-out"
export FIDOGATE_LOGFILE

# Lock it
<LIBDIR>/ftnlock -l $LOCK $$
st=$?
if [ $st -ne 0 ]; then
	exit 2
fi

<BINDIR>/runtoss outpkt

# Unlock it
<LIBDIR>/ftnlock -u $LOCK

exit 0

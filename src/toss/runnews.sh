#!/bin/sh
#
# $Id: runnews.sh,v 4.3 1998/04/07 12:22:00 mj Exp $
#
# Toss gateway output news
#
# Usage: runnews
#

LOCK=runnews

# Output to "log-out" log file
FIDOGATE_LOGFILE="%G/log-news"
export FIDOGATE_LOGFILE

# Lock it
<LIBDIR>/ftnlock -l $LOCK $$
st=$?
if [ $st -ne 0 ]; then
	exit 2
fi

<BINDIR>/runtoss outpkt/news

# Unlock it
<LIBDIR>/ftnlock -u $LOCK

exit 0

#!/bin/sh
#
# $Id: runmail.sh,v 4.4 1998/04/07 12:22:00 mj Exp $
#
# Toss gateway output mail
#
# Usage: runmail
#

LOCK=runmail

# Output to "log-out" log file
FIDOGATE_LOGFILE="%G/log-mail"
export FIDOGATE_LOGFILE

# Lock it
<LIBDIR>/ftnlock -l $LOCK $$
st=$?
if [ $st -ne 0 ]; then
	exit 2
fi

<BINDIR>/runtoss outpkt/mail

# Unlock it
<LIBDIR>/ftnlock -u $LOCK

exit 0

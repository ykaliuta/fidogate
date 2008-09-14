#!/bin/sh
#
# $Id: rungate.sh,v 4.1 1998/11/08 18:28:01 mj Exp $
#
# Run gateway programs
#
# Usage: rungate
#

LOCK=rungate

# Output to "log-in" log file
#FIDOGATE_LOGFILE="%G/log-gate"
#export FIDOGATE_LOGFILE

# Lock it
<LIBDIR>/ftnlock -l $LOCK $$
st=$?
if [ $st -ne 0 ]; then
	exit 2
fi


# Process packets for Internet gateway
<LIBDIR>/ftnin -x %L/ftninpost

# Process packets for FTN gateway
<LIBDIR>/ftn2ftn -A 2:2/242 -B 242:242/2


# Unlock it
<LIBDIR>/ftnlock -u $LOCK

exit 0

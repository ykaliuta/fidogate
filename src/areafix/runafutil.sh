#!/bin/sh
#
# $Id: runafutil.sh,v 1.1 2001/01/28 15:53:16 mj Exp $
#
# Run ftnafutil
#
# Usage: runafutil
#

V=-v

LOCK=runafutil

# Lock it
<LIBDIR>/ftnlock -l $LOCK $$
st=$?
if [ $st -ne 0 ]; then
	exit 2
fi


# Run ftnafutil
<BINDIR>/ftnafutil $V subscribe
<BINDIR>/ftnafutil $V unsubscribe


# Unlock it
<LIBDIR>/ftnlock -u $LOCK

exit 0

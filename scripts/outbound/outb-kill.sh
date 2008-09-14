#!/bin/sh
#
# $Id: outb-kill.sh,v 4.1 2001/01/28 15:53:16 mj Exp $
#
# Remove node from areas.bbs, kill outbound files
#

LIB=<LIBDIR>

for n in $*; do
	[ -z "$n" ] && exit 0;

	echo ""
	echo -n "Kill outbound files for node $n and remove from areas.bbs? [no] "
	read ans
	if [ "$ans" = "yes" ]; then
		echo "Currently subscribed areas:"
		$LIB/ftnaf -- $n query
		echo "Unsubscribing to all areas ..."
		$LIB/ftnaf -- $n '-*' 
		echo "Killing outbound files ..."
		$LIB/ftnflo -v -x "echo '  %s'" -- $n
	fi
done

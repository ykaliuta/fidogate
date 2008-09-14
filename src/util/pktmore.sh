#!/bin/sh
#
# $Id: pktmore.sh,v 4.0 1996/04/17 18:17:43 mj Exp $
#
# Pager wrapper for pktdebug
#

pager=${PAGER-more}

if [ $# -eq 0 ]; then
	echo "usage: pktmore pktfile ..."
	exit 1
fi

for f in $*; do
	( echo "***** $f *****"; pktdebug -mt $f ) | $pager
done

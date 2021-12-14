#!/bin/sh
#
# $Id: ffxnews.sh,v 4.2 2000/04/11 11:32:43 mj Exp $
#
# Transmit batch to $1, using a Fido mailer and the ffx remote execution
# protocol. See case statement for a translation of hostnames to Fido
# addresses.
#
# The 'exec' cuts down the number of active processes
#

batch="-b $1"

case $1 in
	morannon)
		ftn="242:1000/1"
		batch="-b $1"
		;;
	orodruin)
		ftn="242:1000/5"
		batch="-b $1"
		;;
	tux)
		ftn="242:1000/1.20"
		batch="-b $1"
		;;
	*)
		echo "No FTN address for system $1"
		exit 1
		;;
esac
	
exec <LIBDIR>/ffx -gn -FNormal $batch $ftn rnews

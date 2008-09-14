#!/bin/sh
#
# $Id: viaffx.sh,v 1.1 1996/10/13 15:08:43 mj Exp $
#
# Transmit batch to $1, using a Fido mailer and the ffx remote execution
# protocol. See case statement for a translation of hostnames to Fido
# addresses.
#
# The 'exec' cuts down the number of active processes
#

case $1 in
	morannon)
		ftn="242:1000/1"
		;;
	zruty)
		ftn="242:1000/4"
		;;
	orodruin)
		ftn="242:1000/5"
		;;
	sungate)
		ftn="242:4900/99"
		;;
	*)
		echo "No FTN address for system $1"
		exit 1
		;;
esac
	
exec <LIBDIR>/ffx -gn -FNormal $ftn rnews

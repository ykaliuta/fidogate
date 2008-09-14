#!/bin/sh
#
# $Id: ffxmail.sh,v 4.3 2001/01/04 20:03:43 mj Exp $
#
# Frontend for delivering mail via ffx/ffxqt
#
# Customize case statement for your system!!
#

if [ $# -lt 2 ] ;then
	echo "usage: ffxmail Z:N/F.P|HOST user@domain ..."
	exit 1
fi

node="$1"
shift
addr="$*"


case $node in

	242:4900/99 | 242:1000/2 | morannon.fido.de)
		batch="-b morannon"
		faddr="242:1000/2"
		;;

	242:1000/5 | orodruin.fido.de)
		batch="-b orodruin"
		faddr="242:1000/5"
		;;

	242:1000/1.20 | tux.fido.de)
		batch="-b tux"
		faddr="242:1000/1.20"
		;;

##### Insert other nodes here ... #####
#	xyz)
#		batch="..."
#		faddr="..."
#		;;

	# Unknown node
	*)
		echo "ffxmail: unknown node $node"
		# EX_NOHOST
		exit 68
		;;

esac

exec <LIBDIR>/ffx -n $batch -- $faddr rmail $addr

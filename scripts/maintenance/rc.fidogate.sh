#!/bin/sh
#
# $Id: rc.fidogate.sh,v 4.2 1998/11/08 18:27:57 mj Exp $
#
# FIDOGATE boot cleanup
#

# Remove lock files
rm -f <LOCKDIR>/*

# Remove .tmp files
rm -f <SPOOLDIR>/outpkt/*.tmp <SPOOLDIR>/outpkt/*/*.tmp
rm -f <SPOOLDIR>/toss/*/*.tmp

#!/bin/sh
#
# $Id: senduu.sh,v 4.4 1998/11/15 10:59:04 mj Exp $
#
# Send stuff on hold as UUENCODEd mail
#
# usage: senduu user@domain Z:N/F.P ...
#

if [ $# -ne 2 ]; then
  echo "usage: senduu user@domain Z:N/F.P ..."
  exit 1
fi

user=$1
shift

for node in $*; do
    <LIBDIR>/ftnflo -x "<BINDIR>/senduumail $user %s" $node
done

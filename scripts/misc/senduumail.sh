#!/bin/sh
#
# $Id: senduumail.sh,v 4.1 1996/12/29 12:18:02 mj Exp $
#
# Send file as UUENCODEd mail
#
# usage: senduumail user@domain file
#

# Configure me!!!
# MTA, bounced mail to /dev/null (nobody alias)
MAILER="/usr/sbin/sendmail -fnobody@fido.de"

if [ $# -ne 2 ]; then
  echo "usage: senduumail user@domain file"
  exit 1
fi

user=$1
file=$2

uuencode $file `basename $file` | $MAILER $user
st=$?
if [ $st -ne 0 ]; then
  echo "ERROR: senduumail $user $file failed"
  exit 1
fi

exit 0

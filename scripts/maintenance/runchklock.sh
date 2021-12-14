#!/bin/sh
#
# Remove stale FIDOGATE lock files
#

DIR=<LOCKDIR>

for f in $DIR/*; do
  if [ -f $f ] ; then
    pid=`cat $f`
    ps $pid >/dev/null 2>&1 && continue
    echo "runchklog: removing stale lock file $f for PID $pid"
    rm -f $f
  fi
done

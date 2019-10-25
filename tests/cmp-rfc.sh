#!/bin/sh

EXPECTED="$1"
RESULT="$2"

tmp1=$(mktemp)
tmp2=$(mktemp)

grep -v '^X-Gateway: ' < $EXPECTED > $tmp1
grep -v '^X-Gateway: ' < $RESULT > $tmp2

if ! diff $tmp1 $tmp2 > /dev/null; then
    echo "Content different:"
    diff -u $tmp1 $tmp2
    FAIL=true
fi

rm -f $tmp1 $tmp2

[ -z $FAIL ] || exit 1

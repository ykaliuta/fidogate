#!/bin/sh

OD="od -A x -t x1"

EXPECTED="$1"
RESULT="$2"

tmp1=$(mktemp)
tmp2=$(mktemp)

$OD -N 4 "$EXPECTED" > $tmp1
$OD -N 4 "$RESULT" > $tmp2

if ! diff $tmp1 $tmp2 > /dev/null; then
    echo "Node different:"
    diff -u $tmp1 $tmp2
    FAIL=true
fi

$OD -j 16 $EXPECTED > $tmp1
$OD -j 16 $RESULT > $tmp2

if ! diff $tmp1 $tmp2 > /dev/null; then
    echo "Content different:"
    diff -u $tmp1 $tmp2
    FAIL=true
fi

rm -f $tmp1 $tmp2

[ -z $FAIL ] || exit 1

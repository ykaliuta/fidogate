#!/bin/sh

#echo "area=$1,node_from=$2,state=$3,upl=$4,lvl=$5"

if test -n $3; then
    state=",state=$3"
fi
if test $2 != $4; then
    uplink=",upl=$4"
fi
if test $5 -gt 0; then
    level=",lvl=$5"
fi
if test $6 != NONE; then
    key=",key=$6"
fi
if test $7 != NONE; then
    desc=",desc=$7"
fi
if test $9 != "-#"; then
    pass=",nopass"
else
    pass=",pass"
fi

if test -z $10; then
    if test $10 = "-r"; then
	ro=",ro"
    elif test $10 = "-"; then
	ro=",rw"
    fi
fi

echo "area=$1,node_from=$2${state}${uplink}${level}${key}${desc},zone=$8${pass}${ro}"


exit 1;
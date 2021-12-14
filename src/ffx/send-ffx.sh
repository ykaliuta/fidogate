#!/bin/sh
#
# $Id: send-ffx.sh,v 4.2 2000/04/11 11:32:43 mj Exp $
#
# SH script to send batches via FIDOGATE ffx
#

. <NEWSLIBDIR>/innshellvars

PROGNAME=`basename $0`
LOCK=${LOCKS}/LOCK.${PROGNAME}
LOG=${MOST_LOGS}/${PROGNAME}.log

MAXJOBS=200

##  Start logging.
test ! -f ${LOG} && touch ${LOG}
chmod 0660 ${LOG}
exec >>${LOG} 2>&1
echo "${PROGNAME}: [$$] begin `date`"
cd ${BATCH}

##  Anyone else there?
trap 'rm -f ${LOCK} ; exit 1' 1 2 3 15
shlock -p $$ -f ${LOCK} || {
    echo "${PROGNAME}: [$$] locked by [`cat ${LOCK}`]"
    exit 0
}

##  Who are we sending to?
if [ -n "$1" ] ; then
    LIST="$*"
else
    LIST="morannon"
fi


##  Do the work...
for SITE in ${LIST}; do

    ##  See if any data is ready for host.
    BATCHFILE=${SITE}.ffx
    if [ -f ${SITE}.work ] ; then
	cat ${SITE}.work >>${BATCHFILE}
	rm -f ${SITE}.work
    fi
    mv ${SITE} ${SITE}.work
    ctlinnd -s -t30 flush ${SITE} || continue
    cat ${SITE}.work >>${BATCHFILE}
    rm -f ${SITE}.work
    if [ ! -s ${BATCHFILE} ] ; then
	echo "${PROGNAME}: [$$] no articles for ${SITE}"
	rm -f ${BATCHFILE}
	continue
    fi

    QUEUEJOBS=${MAXJOBS}

    echo "${PROGNAME}: [$$] begin ${SITE}"

    time batcher -N ${QUEUEJOBS} -b1000000 \
	-p"<LIBDIR>/ffxnews %s" \
	${SITE} ${BATCHFILE}

    echo "${PROGNAME}: [$$] end ${SITE}"
done

##  Remove the lock file.
rm -f ${LOCK}

echo "${PROGNAME}: [$$] end `date`"

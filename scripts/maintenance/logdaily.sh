#!/bin/sh
#
# $Id: logdaily.sh,v 4.4 1999/02/07 11:05:21 mj Exp $
#
# Daily log processing
#

# run logcheck
if [ -f /var/log/maillog.1.gz ]; then
    zcat /var/log/maillog.1.gz | <BINDIR>/logcheck -n
    zcat /var/log/maillog.1.gz | <BINDIR>/logcheck -r -m admin-logs
fi

# run logsendmail
if [ -f /var/log/maillog.1.gz ]; then
    zcat /var/log/maillog.1.gz | <BINDIR>/logsendmail -m admin-logs
fi

# run logstat
if [ -f <LOGDIR>/log-in.1.gz ]; then
    zcat <LOGDIR>/log-in.1.gz \
    | <BINDIR>/logstat -m admin-logs -t '(inbound)'
fi
if [ -f <LOGDIR>/log-mail.1.gz ]; then
    zcat <LOGDIR>/log-mail.1.gz \
    | <BINDIR>/logstat -m admin-logs -t '(gateway output mail)'
fi
if [ -f <LOGDIR>/log-news.1.gz ]; then
    zcat <LOGDIR>/log-news.1.gz \
    | <BINDIR>/logstat -m admin-logs -t '(gateway output news)'
fi

# run logreport
if [ -f <LOGDIR>/log-in.1.gz ]; then
    zcat <LOGDIR>/log-in.1.gz \
    | <BINDIR>/logreport -n
fi

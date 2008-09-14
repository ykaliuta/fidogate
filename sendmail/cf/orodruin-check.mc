#:ts=8
#
# $Id: orodruin-check.mc,v 4.4 2002/08/04 02:27:32 dyff Exp $
#
# orodruin.Fido.DE
#

include(`../m4/cf.m4')

define(`confCF_VERSION', `orodruin-check-4.1')

define(`confMIME_FORMAT_ERRORS', `False')
define(`confUSE_ERRORS_TO', `True')
define(`confMAX_HOP', `30')
define(`confMESSAGE_TIMEOUT', `5d/2d')
# RedHat 4.0 specific
define(`confDEF_USER_ID',``8:12'')

define(`ALIAS_FILE',`/etc/aliases,/usr/local/majordomo/majordomo.aliases')

VERSIONID(`$Id: orodruin-check.mc,v 4.4 2002/08/04 02:27:32 dyff Exp $')

OSTYPE(linux)dnl

undefine(`UUCP_RELAY')
undefine(`BITNET_RELAY')


define(`_CHECK_FROM_')
undefine(`_IP_LOOKUP_')
undefine(`_DNSVALID_')
HACK(check_mail3,`hash -o -a@JUNK /etc/mail/junk')

HACK(use_ip,`/etc/mail/local.ip')
HACK(use_relayto,`/etc/mail/local.relayto')
HACK(check_rcpt4)


FEATURE(redirect)dnl
FEATURE(notsticky)dnl
FEATURE(always_add_domain)dnl
FEATURE(nodns)dnl
FEATURE(nocanonify)dnl
FEATURE(mailertable,hash /etc/mail/mailertable.db)dnl
FEATURE(local_procmail)dnl

MAILER(procmail)dnl
MAILER(smtp)dnl
MAILER(ftn)dnl
MAILER(ffx)dnl

# Smart host and mailer
define(`SMART_HOST', ffx:morannon.fido.de)

# Map for `LOCAL_RULE_3' rewrite rules
Krewrite hash -o /etc/mail/rewrite.db


LOCAL_CONFIG
# More trusted users
Tnews
Tmajordomo


LOCAL_RULE_3
# Rewrite addresses according to rewrite.db map
R$* < @ $* > $*			$: $1 < @ $(rewrite $2 $@ %1 $: $2 $) > $3


LOCAL_NET_CONFIG

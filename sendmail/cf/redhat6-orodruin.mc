#
# $Id: redhat6-orodruin.mc,v 4.4 2002/08/04 02:27:32 dyff Exp $
#
# orodruin.Fido.DE
#

include(`../m4/cf.m4')

VERSIONID(`$Id: redhat6-orodruin.mc,v 4.4 2002/08/04 02:27:32 dyff Exp $')

dnl #
dnl # Configuration
dnl #
define(`confCF_VERSION', `redhat6-orodruin-4.1')
define(`confMIME_FORMAT_ERRORS', `False')
define(`confUSE_ERRORS_TO', `True')
define(`confMAX_HOP', `30')
define(`confMESSAGE_TIMEOUT', `5d/2d')
define(`confAUTO_REBUILD',true)
define(`confTO_CONNECT', `1m')
define(`confDONT_PROBE_INTERFACES',true)

dnl # RedHat specific
define(`confDEF_USER_ID',``8:12'')
dnl # It's a Linux box ;-)
OSTYPE(`linux')

define(`PROCMAIL_MAILER_PATH',`/usr/bin/procmail')
dnl # extra aliases file for majordomo
define(`ALIAS_FILE',`/etc/aliases,/usr/local/majordomo/majordomo.aliases')

FEATURE(`smrsh',`/usr/sbin/smrsh')
FEATURE(`virtusertable',`hash -o /etc/mail/virtusertable')
FEATURE(redirect)
FEATURE(always_add_domain)
FEATURE(use_cw_file)
FEATURE(local_procmail)
FEATURE(`access_db')
FEATURE(`blacklist_recipients')
dnl We strongly recommend to comment this one out if you want to protect
dnl yourself from spam. However, the laptop and users on computers that do
dnl not hav 24x7 DNS do need this.
FEATURE(`accept_unresolvable_domains')
dnl FEATURE(`relay_based_on_MX')
FEATURE(redirect)
dnl # because orodruin is not directly connected to the Internet
FEATURE(nocanonify)
FEATURE(mailertable,hash /etc/mail/mailertable.db)dnl

MAILER(procmail)
MAILER(smtp)
MAILER(ftn)
MAILER(ffx)


# Smart host and mailer
define(`SMART_HOST', ffx:morannon.fido.de)


LOCAL_CONFIG
# More trusted users
Tnews
Tmajordomo

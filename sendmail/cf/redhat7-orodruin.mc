#
# $Id: redhat7-orodruin.mc,v 4.3 2002/08/04 02:27:32 dyff Exp $
#
# orodruin.Fido.DE
#

divert(-1)

include(`../m4/cf.m4')

VERSIONID(`$Id: redhat7-orodruin.mc,v 4.3 2002/08/04 02:27:32 dyff Exp $')dnl

define(`confCF_VERSION', `redhat7-orodruin-1.1')
define(`confMIME_FORMAT_ERRORS', `False')
define(`confUSE_ERRORS_TO', `True')
define(`confMAX_HOP', `30')
define(`confMESSAGE_TIMEOUT', `5d/2d')
define(`confAUTO_REBUILD',true)
define(`confTO_CONNECT', `1m')
define(`confDONT_PROBE_INTERFACES',true)
undefine(`UUCP_RELAY')
undefine(`BITNET_RELAY')
define(`confTRY_NULL_MX_LIST',true)
define(`confDONT_PROBE_INTERFACES',true)
define(`ALIAS_FILE', `/etc/aliases')
define(`STATUS_FILE', `/var/log/sendmail.st')
define(`UUCP_MAILER_MAX', `2000000')
define(`confUSERDB_SPEC', `/etc/mail/userdb.db')
define(`confPRIVACY_FLAGS', `authwarnings,novrfy,noexpn,restrictqrun')
define(`confAUTH_OPTIONS', `A')
dnl TRUST_AUTH_MECH(`DIGEST-MD5 CRAM-MD5 LOGIN PLAIN')
dnl define(`confAUTH_MECHANISMS', `DIGEST-MD5 CRAM-MD5 LOGIN PLAIN')
dnl define(`confTO_QUEUEWARN', `4h')
dnl define(`confTO_QUEUERETURN', `5d')
dnl define(`confQUEUE_LA', `12')
dnl define(`confREFUSE_LA', `18')

dnl # It's a Linux box ;-)
OSTYPE(`linux')
dnl # RedHat specific
define(`confDEF_USER_ID',``8:12'')dnl
define(`PROCMAIL_MAILER_PATH',`/usr/bin/procmail')
FEATURE(local_procmail)

dnl # extra aliases file for majordomo
define(`ALIAS_FILE',`/etc/aliases,/usr/local/majordomo/majordomo.aliases')

dnl FEATURE(delay_checks)dnl
FEATURE(`no_default_msa',`')
FEATURE(`smrsh',`/usr/sbin/smrsh')
FEATURE(`mailertable',`hash -o /etc/mail/mailertable')
FEATURE(`virtusertable',`hash -o /etc/mail/virtusertable')
FEATURE(redirect)
FEATURE(always_add_domain)
FEATURE(use_cw_file)
FEATURE(use_ct_file)
FEATURE(`access_db')
FEATURE(`blacklist_recipients')
FEATURE(nocanonify)
EXPOSED_USER(`root')

dnl This changes sendmail to only listen on the loopback device 127.0.0.1
dnl and not on any other network devices. Comment this out if you want
dnl to accept email over the network.
dnl DAEMON_OPTIONS(`Port=smtp,Addr=127.0.0.1, Name=MTA')

dnl We strongly recommend to comment this one out if you want to protect
dnl yourself from spam. However, the laptop and users on computers that do
dnl not have 24x7 DNS do need this.
FEATURE(`accept_unresolvable_domains')
dnl FEATURE(`relay_based_on_MX')

MAILER(smtp)
MAILER(procmail)
MAILER(ftn)
MAILER(ffx)


# Smart host and mailer
define(`SMART_HOST', ffx:morannon.fido.de)


LOCAL_CONFIG
# More trusted users
Tnews
Tmajordomo

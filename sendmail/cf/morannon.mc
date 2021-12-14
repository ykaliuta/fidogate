#:ts=8
#
# $Id: morannon.mc,v 4.7 2002/08/04 02:27:32 dyff Exp $
#
# Fido.DE domain gateway morannon.fido.de sendmail V8 configuration
#

include(`../m4/cf.m4')
VERSIONID(`$Id: morannon.mc,v 4.7 2002/08/04 02:27:32 dyff Exp $')
OSTYPE(linux)

define(`confCF_VERSION', `morannon-4.5')

define(`confMIME_FORMAT_ERRORS', `False')
define(`confUSE_ERRORS_TO', `True')
define(`confMAX_HOP', `30')
define(`confMESSAGE_TIMEOUT', `5d/2d')

define(`ALIAS_FILE',`/etc/aliases,/usr/local/majordomo/majordomo.aliases')

FEATURE(notsticky)
FEATURE(always_add_domain)
FEATURE(mailertable,hash /etc/mail/mailertable.db)
FEATURE(local_procmail)

MAILER(local)
MAILER(smtp)
MAILER(uucp)
MAILER(ftn)
MAILER(ffx)

# Alternate names
Cwmorannon-ftn.fido.de morannon-ftn
Cwsungate-ftn.fido.de sungate-ftn
Cwsungate.fido.de sungate
Cwfido.de
Cwmorannon.faho.rwth-aachen.de
Cwmorannon.fido.de morannon
Cwwww.fido.de www
Cwftp.fido.de ftp

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

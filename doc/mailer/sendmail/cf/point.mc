#
# $Id$
#
# sendmail V8 configuration for a FIDO point, routing all Internet mail
# via FIDOGATE and a FIDO-Internet gateway
#

include(`../m4/cf.m4')
VERSIONID(`$Id$')
OSTYPE(linux)

define(`confMIME_FORMAT_ERRORS', `False')
define(`confUSE_ERRORS_TO', `True')

FEATURE(notsticky)
FEATURE(always_add_domain)
FEATURE(nodns)
FEATURE(nocanonify)
FEATURE(nouucp)

MAILER(local)
MAILER(smtp)
MAILER(uucp)
MAILER(ftn)

define(`SMART_HOST', ftni:INSERT-YOUR-UPLINK-HERE)


LOCAL_CONFIG
# More trusted users
Tnews

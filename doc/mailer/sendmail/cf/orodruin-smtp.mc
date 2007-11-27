#:ts=8
#
# $Id$
#
# Fido.DE subsidiary machine (SMTP)
#

include(`../m4/cf.m4')
VERSIONID(`$Id$')
OSTYPE(linux)dnl

define(`confCF_VERSION', `orodruin-4.1')

define(`confMIME_FORMAT_ERRORS', `False')
define(`confUSE_ERRORS_TO', `True')
define(`confMAX_HOP', `30')
define(`confMESSAGE_TIMEOUT', `5d/2d')

FEATURE(notsticky)dnl
FEATURE(always_add_domain)dnl
FEATURE(nodns)dnl
FEATURE(nocanonify)dnl

MAILER(local)dnl
MAILER(smtp)dnl

# Smart host and mailer
define(`SMART_HOST', smtp:morannon.)


LOCAL_CONFIG
# More trusted users
Tnews

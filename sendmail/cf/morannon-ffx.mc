#:ts=8
#
# $Id: morannon-ffx.mc,v 4.4 2002/08/04 02:27:32 dyff Exp $
#
# Fido.DE domain gateway sendmail V8 configuration
#

include(`../m4/cf.m4')
VERSIONID(`$Id: morannon-ffx.mc,v 4.4 2002/08/04 02:27:32 dyff Exp $')
OSTYPE(linux)dnl

define(`VERSION_NUMBER', `Fido.DE-4.1')

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
MAILER(uucp)dnl
MAILER(ftn)dnl
MAILER(ffx)dnl

# Alternate names
Cwmorannon-ftn.fido.de morannon-ftn
Cwsungate-ftn.fido.de sungate-ftn
Cwsungate.fido.de sungate

# Local hosts via SMTP
CSorodruin

# Gateway for FIDO mail (warning: conflicts with FAX relay!)
DFsungate-ftn.fido.de

# FIDO domains via mailer ftni (no leading "."!)
CIz242.fido.de z254.fido.de z2.fidonet.org

# FIDO domains via mailer ftn (no leading "."!)
CNfido.de

# Smart host and mailer
define(`SMART_HOST', ffx:zruty.dfv.rwth-aachen.de)

# Map for `LOCAL_RULE_3' rewrite rules
Krewrite dbm -o /etc/mail/rewrite



LOCAL_CONFIG
# More trusted users
Tnews



LOCAL_RULE_3
# Rewrite addresses according to rewrite.db map
R$* < @ $* > $*			$: $1 < @ $(rewrite $2 $@ %1 $: $2 $) > $3

# local SMTP hosts are canonical
R$* < @ $=S > $*		$: $1 < @ $2 .$m. > $3
R$* < @ $=S . $m > $*		$: $1 < @ $2 .$m. > $3

# class I/N domains are canonical
R$* < @ $* .$=I > $*		$: $1 < @ $2 .$3. > $4
R$* < @ $* .$=N > $*		$: $1 < @ $2 .$3. > $4

# handle addresses in local domain (useful only for a domain gateway!)
R$* < @ $* $m > $*		$: $1 < @ $2 $m. > $3



LOCAL_NET_CONFIG
# Mail to domain only is local (useful only for a domain gateway!)
R$+ < @ $=m . >			$#local $: $1

# Mail to class S hosts directly, not via MX!
R$* <@ $=S . $m . > $*		$#smtp $@$2. $:$1<@$2.$m.>$3

# Fido mail to class I via ftni
R$* <@ $* . $=I . > $*		$#ftni $@$F $:$1<@$2.$3.>$4

# Fido mail to class N via ftn
R$* <@ $* . $=N . > $*		$#ftn $@$F $:$1<@$2.$3.>$4


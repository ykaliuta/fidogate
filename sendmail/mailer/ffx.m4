PUSHDIVERT(-1)
#
# $Id: ffx.m4,v 4.6 2003/04/15 18:45:23 n0ll Exp $
#
# FIDOGATE FFX mailer for sendmail V8
#

ifdef(`confFIDOGATE_LIBDIR',,
  `define(`confFIDOGATE_LIBDIR', `/usr/lib/fidogate')')

ifdef(`FFX_MAILER_PATH',,
  `define(`FFX_MAILER_PATH', confFIDOGATE_LIBDIR/ffxmail)')
ifdef(`FFX_MAILER_ARGS',,
  `define(`FFX_MAILER_ARGS', `ffxmail $h $u')')
ifdef(`FFX_MAILER_FLAGS',,
  `define(`FFX_MAILER_FLAGS', `')')
POPDIVERT


#####################################
###    FFX Mailer specification   ###
#####################################

VERSIONID(`$Revision: 4.6 $')

# FIDOGATE mailer
Mffx,	P=FFX_MAILER_PATH, F=CONCAT(mDFMuX8, FFX_MAILER_FLAGS), S=11/31, R=ifdef(`_ALL_MASQUERADE_', `21/31', `21'),
	A=FFX_MAILER_ARGS


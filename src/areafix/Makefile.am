#:ts=8
#
#    $Id: Makefile.am,v 5.2 2004/11/23 00:50:39 anray Exp $
#
#    Makefile for FIDOGATE src/areafix/
#

AM_CPPFLAGS		= -I$(top_srcdir)/src/include

LDADD			= $(top_srcdir)/src/common/libfidogate.la

AM_CFLAGS		= -Wall

bin_PROGRAMS		= ftnaf ftnafutil pkttmpl
libexec_PROGRAMS        = ftnafpkt ftnafmail

ftnaf_SOURCES		= ftnaf.c areafix.c mailer.c
ftnafutil_SOURCES	= ftnafutil.c areafix.c
ftnafmail_SOURCES	= ftnafmail.c areafix.c
ftnafpkt_SOURCES	= ftnafpkt.c areafix.c

installcheck-am:
	chmod 4755 $(DESTDIR)$(libexecdir)/ftnafmail$(EXEEXT)

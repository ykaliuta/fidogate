#:ts=8
#
#    $Id: Makefile.am,v 5.2 2004/11/23 00:50:41 anray Exp $
#
#    Makefile for FIDOGATE src/toss/
#

AUTOMAKE_OPTIONS        = subdir-objects

AM_CPPFLAGS		= -I$(top_srcdir)/src/include

if enable_static
LDADD			= $(top_srcdir)/src/common/libfidogate.a
else
LDADD			= $(top_srcdir)/src/common/libfidogate.la
endif

AM_CFLAGS		= -Wall

bin_PROGRAMS		= ftnexpire runinc ngoper
libexec_PROGRAMS	= ftn2ftn ftnpack ftnroute ftntoss
libexec_SCRIPTS		= report_traffic

ftntoss_SOURCES		= ftntoss.c \
			  history.c \
			  ../../src/areafix/areafix.c \
                          ../../src/areafix/mailer.c

report_traffic:
	$(SUBSTCMD) $@.in > $@
	chmod +x $@

clean-am:
	rm -f report_traffic

installcheck-am:
	if test `id -u` -eq 0; then \
	chown news $(DESTDIR)$(bindir)/ngoper; \
	fi
	chmod 4550 $(DESTDIR)$(bindir)/ngoper

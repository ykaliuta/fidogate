#:ts=8
#
#    $Id$
#
#    Makefile for FIDOGATE src/toss/
#

INCLUDES		= -I$(top_srcdir)/src/include

if enable_static
LDADD			= $(top_srcdir)/src/common/libfidogate.a
else
LDADD			= $(top_srcdir)/src/common/libfidogate.la
endif

CFLAGS			= -Wall

runinc_CFLAGS		= -g -O3

bin_PROGRAMS		= ftnexpire runinc ngoper
libexec_PROGRAMS	= ftn2ftn ftnpack ftnroute ftntoss
libexec_SCRIPTS		= report_traffic

ftntoss_SOURCES		= ftntoss.c \
			  history.c \
			  $(top_srcdir)/src/areafix/areafix.c \
                          $(top_srcdir)/src/areafix/mailer.c

report_traffic:
	$(SUBSTCMD) $@.in > $@
	chmod +x $@

clean-am:
	rm -f report_traffic

install-exec-am:
	chown news $(DESTDIR)$(bindir)/ngoper
	chmod 4550 $(DESTDIR)$(bindir)/ngoper

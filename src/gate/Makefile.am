#:ts=8
#
#    $Id$
#
#    Makefile for FIDOGATE src/gate/
#

INCLUDES		= -I$(top_srcdir)/src/include

if enable_static
LDADD			= $(top_srcdir)/src/common/libfidogate.a
else
LDADD			= $(top_srcdir)/src/common/libfidogate.la
endif

AM_CFLAGS		= -Wall

libexec_SCRIPTS		= ftninrecomb send-fidogate


if ftnnews
libexec_PROGRAMS	= ftn2rfc ftnin ftninpost rfc2ftn ftnnews ftnmail
else
libexec_PROGRAMS	= ftn2rfc ftnin ftninpost rfc2ftn ftnmail
endif

$(libexec_SCRIPTS):
	$(SUBSTCMD) $@.in > $@
	chmod +x $@

clean-am:
	rm -f $(libexec_SCRIPTS)

install-exec-am:
	chmod 4755 $(DESTDIR)$(libexecdir)/ftnmail$(EXEEXT)
	@if test -e $(srcdir)/ftnnews$(EXEEXT); then \
	chmod 4750 $(DESTDIR)$(libexecdir)/ftnnews$(EXEEXT); \
	echo " chmod 4750 $(DESTDIR)$(libexecdir)/ftnnews$(EXEEXT)"; \
	if test `id -u` -eq 0; then \
	chown news $(DESTDIR)$(libexecdir)/ftnnews$(EXEEXT); \
	echo " chown news $(DESTDIR)$(libexecdir)/ftnnews$(EXEEXT)"; \
	fi; \
	fi

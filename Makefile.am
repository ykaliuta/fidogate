#
#
# $Id: Makefile.am,v 5.3 2006/05/18 18:35:07 anray Exp $
#
# Makefile FIDOGATE TOPDIR
#

AUTOMAKE_OPTIONS=foreign dist-bzip2 1.5

include $(top_builddir)/build/config.make


TOPDIR		= .

SUBDIRS		= src scripts doc tests

FDESTDIR	= $(DEFAULT_V_CONFIGDIR) \
		  $(DEFAULT_V_LOGDIR) \
		  $(DEFAULT_V_VARDIR) \
		  $(DEFAULT_V_VARDIR)/seq \
		  $(DEFAULT_V_LOCKDIR) \
		  $(DEFAULT_V_SPOOLDIR) \
		  $(DEFAULT_V_SPOOLDIR)/outrfc \
		  $(DEFAULT_V_SPOOLDIR)/outrfc/mail \
		  $(DEFAULT_V_SPOOLDIR)/outrfc/news \
		  $(DEFAULT_V_SPOOLDIR)/outpkt \
		  $(DEFAULT_V_SPOOLDIR)/outpkt/mail \
		  $(DEFAULT_V_SPOOLDIR)/outpkt/news \
		  $(DEFAULT_V_SPOOLDIR)/toss \
		  $(DEFAULT_V_SPOOLDIR)/toss/toss \
		  $(DEFAULT_V_SPOOLDIR)/toss/route \
		  $(DEFAULT_V_SPOOLDIR)/toss/pack \
		  $(DEFAULT_V_SPOOLDIR)/toss/bad \
		  $(DEFAULT_V_SPOOLDIR)/ftpin \
		  $(DEFAULT_V_SPOOLDIR)/uuin \
		  $(DEFAULT_V_BTBASEDIR) \
		  $(DEFAULT_V_BTBASEDIR)/tick \
		  $(DEFAULT_V_INBOUND) \
		  $(DEFAULT_V_INBOUND)/bad \
		  $(DEFAULT_V_INBOUND)/tmpunpack \
		  $(DEFAULT_V_PINBOUND) \
		  $(DEFAULT_V_PINBOUND)/bad \
		  $(DEFAULT_V_PINBOUND)/tmpunpack

OWNER = @OWNER@
GROUP = @GROUP@

install-am: installdirs

installdirs:
	@for d in $(FDESTDIR); do \
	if ! test -d $(DESTDIR)$$d; then \
	$(mkinstalldirs) $(DESTDIR)$$d || exit 1; \
	echo "create directory $(DESTDIR)$$d"; \
	fi; \
	done; \
	if test `id -u` -eq 0 && test -n "${OWNER}" && test -n "${GROUP}"; then \
	for d in $(FDESTDIR); do \
	chown ${OWNER}:${GROUP} $(DESTDIR)$$d || exit 1; \
	echo "chown ${OWNER}:${GROUP} $(DESTDIR)$$d"; \
	done; \
	fi

install-config: install-dirs
	if [ -f doc/Makefile ]; then $(MAKE) -C doc $@ || exit 1; fi

ACLOCAL_AMFLAGS = -I m4

EXTRA_DIST = build/config.rpath m4/ChangeLog

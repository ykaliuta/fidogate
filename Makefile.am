#
#
# $Id: Makefile.am,v 1.1.1.1 2003/09/23 17:26:56 rusfidogate Exp $
#
# Makefile FIDOGATE TOPDIR
#

AUTOMAKE_OPTIONS=foreign

include $(top_builddir)/build/config.make


TOPDIR		= .

SUBDIRS		= src scripts doc test

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
		  $(DEFAULT_V_BTBASEDIR) \
		  $(DEFAULT_V_BTBASEDIR)/tick \
		  $(DEFAULT_V_INBOUND) \
		  $(DEFAULT_V_INBOUND)/bad \
		  $(DEFAULT_V_INBOUND)/tmpunpack \
		  $(DEFAULT_V_PINBOUND) \
		  $(DEFAULT_V_PINBOUND)/bad \
		  $(DEFAULT_V_PINBOUND)/tmpunpack 

install-am: installdirs

installdirs:
	@for d in $(FDESTDIR); do\
	if ! test -d $(DESTDIR)$$d; then \
	$(mkinstalldirs) $(DESTDIR)$$d || exit 1;\
	echo "create directory $(DESTDIR)$$d"; \
	fi; \
	done

install-config: install-dirs
	if [ -f doc/Makefile ]; then $(MAKE) -C doc $@ || exit 1; fi

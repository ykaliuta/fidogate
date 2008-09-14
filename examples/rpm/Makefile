#:ts=8
#
#    $Id: Makefile,v 1.3 2002/08/05 23:53:47 dyff Exp $
#
#    Makefile for FIDOGATE examples/rpm
#

#
# Normally it should not be necessary to modify this file, see
# ../config.make and ../rules.make
#

TOPDIR		= ../..

include $(TOPDIR)/config.make
include $(TOPDIR)/rules.make


CF_ETC_FIDOGATE	= aliases areas fidogate.conf hosts packing passwd routing
CF_VAR_FIDOGATE = areas.bbs fareas.bbs
CF_ETC_NEWS	= newsfeeds.fidogate


all:		


install-config:
	if [ -z "$(PREFIX)" ]; then \
	  echo "!!! Prefix must be set for make install-config !!!"; \
	  exit 1; \
	fi
	$(INSTALL_DATA) $(CF_ETC_FIDOGATE) $(PREFIX)$(DEFAULT_V_CONFIGDIR)
	$(INSTALL_DATA) $(CF_VAR_FIDOGATE) $(PREFIX)$(DEFAULT_V_VARDIR)
	$(INSTALL_DATA) $(CF_ETC_NEWS) $(PREFIX)$(DEFAULT_F_NEWSETCDIR)


clean:

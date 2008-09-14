#:ts=8
#
#    $Id: Makefile,v 4.7 1999/05/15 20:54:36 mj Exp $
#
#    Makefile for FIDOGATE scripts/filebase/
#

#
# Normally it should not be necessary to modify this file, see
# ../config.make and ../rules.make
#

TOPDIR		= ../..

include $(TOPDIR)/config.make
include $(TOPDIR)/rules.make


PROGS		= latest\
		  nl-autoupd nl-del nl-diff nl-html nl-isdn nl-print nl-check\
		  nl-2hosts nl-2routing


all:		$(PROGS)
$(PROGS):	$(TOPDIR)/config.make $(TOPDIR)/config.pl



install:
	$(INSTALL_PROG) latest $(PREFIX)$(BINDIR)
	$(INSTALL_PROG) nl-autoupd $(PREFIX)$(BINDIR)
	$(INSTALL_PROG) nl-del $(PREFIX)$(BINDIR)
	$(INSTALL_PROG) nl-diff $(PREFIX)$(BINDIR)
	$(INSTALL_PROG) nl-check $(PREFIX)$(BINDIR)


clean:
	rm -f $(PROGS)
	rm -f *~ *.bak *.o tags TAGS core paths.tmp


#
# Dependencies
#
depend:

depend-all:

ifeq (.depend,$(wildcard .depend))
include .depend
endif

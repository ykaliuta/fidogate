#:ts=8
#
#    $Id: Makefile,v 4.10 2000/04/19 17:07:28 mj Exp $
#
#    Makefile for FIDOGATE scripts/run/
#

#
# Normally it should not be necessary to modify this file, see
# ../config.make and ../rules.make
#

TOPDIR		= ../..

include $(TOPDIR)/config.make
include $(TOPDIR)/rules.make


PROGS		= runpoll runpoll-orodruin runpoll-binkd


all:		$(PROGS)
$(PROGS):	$(TOPDIR)/config.make $(TOPDIR)/config.pl



install:
	$(INSTALL_PROG) runpoll          $(PREFIX)$(BINDIR)
	$(INSTALL_PROG) runpoll-orodruin $(PREFIX)$(BINDIR)
	$(INSTALL_PROG) runpoll-binkd    $(PREFIX)$(BINDIR)


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

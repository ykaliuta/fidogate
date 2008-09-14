#:ts=8
#
#    $Id: Makefile,v 1.2 1999/05/15 20:54:35 mj Exp $
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


PROGS		= run-batch viaffx viafidogate


all:		$(PROGS)
$(PROGS):	$(TOPDIR)/config.make $(TOPDIR)/config.pl


install:


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

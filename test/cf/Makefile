#
# $Id: Makefile,v 1.1 2003/09/23 17:45:44 rusfidogate Exp $
#
# Makefile FIDOGATE doc/examples/dyff
#

TOPDIR		= ../..

include $(TOPDIR)/build/config.make
include $(TOPDIR)/build/rules.make

CONFIGS		= packing

CF_BBS	= areas.bbs \
	fareas.bbs

all:	$(CONFIGS)

$(CONFIGS):	$(TOPDIR)/config.pl

clean distclean:
	rm -f *~ *.bak *.o tags TAGS core config.pl $(CONFIGS)

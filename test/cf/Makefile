#
# $Id$
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

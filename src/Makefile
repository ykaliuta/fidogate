#:ts=8
#
#    $Id: Makefile,v 4.13 1999/08/08 09:14:31 mj Exp $
#
#    Makefile for FIDOGATE src/
#

#
# Normally it should not be necessary to modify this file, see
# ../config.make and ../rules.make
#

TOPDIR		= ..

include $(TOPDIR)/config.make
include $(TOPDIR)/rules.make


SUBDIRS		= include common charset areafix ffx gate tick toss util


all::

install::


all clean veryclean depend install check::
	for d in $(SUBDIRS); do $(MAKE) -C $$d $@ || exit 1; done 

clean::
	rm -f *~ *.bak *.o tags TAGS core config.pl

check::
verify:	check
test:	check

tags:
	etags */*.[hcy] */*.pl

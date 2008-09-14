#
# $Id: Makefile,v 1.5 1997/10/11 21:24:19 mj Exp $
#
# Makefile FIDOGATE doc/html/
#

TOPDIR		= ../..

include $(TOPDIR)/config.make
include $(TOPDIR)/rules.make


all:		fidogate_toc.html


fidogate_toc.html:	../fidogate.texi
	-texi2html -verbose -menu -split_chapter ../fidogate.texi


clean:
	rm -f *~ *.bak *.o tags TAGS core

veryclean:	clean
	rm -f fidogate_*.html


check:

install:
	cp fidogate_*.html $(PREFIX)$(HTMLDIR)

depend:

#
# $Id: Makefile,v 4.9 1999/04/03 13:28:56 mj Exp $
#
# Makefile FIDOGATE doc/
#

TOPDIR		= ..

include $(TOPDIR)/config.make
include $(TOPDIR)/rules.make

SUBDIRS		= html gatebau


all::		fidogate.info fidogate.txt

all clean veryclean check depend install::
	for d in $(SUBDIRS); do \
	  if [ -f $$d/Makefile ]; then $(MAKE) -C $$d $@ || exit 1; fi; \
	done


doc:		fidogate.info fidogate.txt fidogate.ps


fidogate.info:	fidogate.texi
	-makeinfo fidogate.texi

fidogate.txt:	fidogate.info
	$(PERL) info2txt.pl fidogate.info-* >fidogate.txt

fidogate.dvi:	fidogate.texi
	texi2dvi fidogate.texi

fidogate.ps:	fidogate.dvi
	dvips fidogate -o


clean::
	rm -f *~ *.bak *.o tags TAGS core
	rm -f *.cp *.cps *.fn *.fns *.ky *.kys *.pg *.pgs *.toc *.aux
	rm -f *.tp *.tps *.vr *.vrs *.log

veryclean::	clean
	rm -f fidogate.info* fidogate.ps fidogate.dvi fidogate.txt

check::
verify:	check
test:	check


install::
	$(INSTALL_DATA) fidogate.info* $(PREFIX)$(INFODIR)

depend::

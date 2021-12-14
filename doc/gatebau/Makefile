#
# $Id: Makefile,v 1.5 1999/01/10 16:55:58 mj Exp $
#
# Makefile FIDOGATE doc/gatebau/
#

TOPDIR		= ../..

include $(TOPDIR)/config.make
include $(TOPDIR)/rules.make


all:		msgid.html msgid.tex msgid.txt


msgid.html:	msgid.sgml
	-sgml2html msgid.sgml
	./post-html.pl msgid*.html

msgid.tex:	msgid.sgml
	-sgml2latex -o tex msgid.sgml

msgid.txt:	msgid.sgml
	-sgml2txt msgid.sgml


clean:
	rm -f *~ *.bak *.o tags TAGS core

veryclean:	clean
	rm -f *.html *.tex *.txt


check:

install:
	cp msgid*.html $(PREFIX)$(HTMLDIR)

depend:

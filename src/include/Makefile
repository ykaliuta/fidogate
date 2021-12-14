#:ts=8
#
#    $Id: Makefile,v 4.3 1998/01/13 20:33:51 mj Exp $
#
#    Makefile for FIDOGATE src/include/
#

#
# Normally it should not be necessary to modify this file, see
# ../config.make and ../rules.make
#

TOPDIR		= ../..

include $(TOPDIR)/config.make
include $(TOPDIR)/rules.make


all:		paths.mak cf_funcs.mak

paths.mak:	$(TOPDIR)/config.make $(TOPDIR)/subst.pl
	$(PERL) $(TOPDIR)/subst.pl -c$(TOPDIR)/config.make -h >paths.mak
	diff paths.mak paths.h >/dev/null || cp paths.mak paths.h

cf_funcs.mak:	$(TOPDIR)/config.make $(TOPDIR)/subst.pl
	$(PERL) $(TOPDIR)/subst.pl -c$(TOPDIR)/config.make -H >cf_funcs.mak
	diff cf_funcs.mak cf_funcs.h >/dev/null || cp cf_funcs.mak cf_funcs.h

install:


clean veryclean:
	rm -f *~ *.bak *.o tags TAGS core paths.tmp

check:

depend:

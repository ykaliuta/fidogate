#:ts=8
#
#    $Id: Makefile,v 1.2 2001/01/07 15:29:44 mj Exp $
#
#    Makefile for FIDOGATE scripts/areas/
#

#
# Normally it should not be necessary to modify this file, see
# ../config.make and ../rules.make
#

TOPDIR		= ../..

include $(TOPDIR)/config.make
include $(TOPDIR)/rules.make


PROGS		= areasbbssync areassucksync


all:		$(PROGS)
$(PROGS):	$(TOPDIR)/config.make $(TOPDIR)/config.pl


SUBSTCMD	= $(PERL) $(TOPDIR)/subst.pl -t$(TOPDIR) \
		  -c$(TOPDIR)/examples/morannon/config.make.morannon -p

morannon:
	$(SUBSTCMD) areasbbssync.pl >areasbbssync
	chmod +x areasbbssync


install:
	$(INSTALL_PROG) areasbbssync $(PREFIX)$(BINDIR)


clean:
	rm -f $(PROGS)
	rm -f *~ *.bak *.o tags TAGS core paths.tmp

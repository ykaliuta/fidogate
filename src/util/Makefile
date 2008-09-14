#:ts=8
#
#    $Id: Makefile,v 4.10 1998/04/03 20:15:37 mj Exp $
#
#    Makefile for FIDOGATE src/util/
#

#
# Normally it should not be necessary to modify this file, see
# ../config.make and ../rules.make
#

TOPDIR		= ../..

include $(TOPDIR)/config.make
include $(TOPDIR)/rules.make


PROGS		= ftnfattach ftnbsy ftnconfig ftnlock ftnlog ftnseq ftnflo\
		  pktdebug pktmore sumcrc ftnoutpkt


all:		$(PROGS)

ftnbsy:		$(TOPDIR)/src/common/$(LIB)
ftnconfig:	$(TOPDIR)/src/common/$(LIB)
ftnfattach:	$(TOPDIR)/src/common/$(LIB)
ftnflo:		$(TOPDIR)/src/common/$(LIB)
ftnlock:	$(TOPDIR)/src/common/$(LIB)
ftnlog:		$(TOPDIR)/src/common/$(LIB)
ftnseq:		$(TOPDIR)/src/common/$(LIB)
pktdebug:	$(TOPDIR)/src/common/$(LIB)
sumcrc:		$(TOPDIR)/src/common/$(LIB)
ftnoutpkt:	$(TOPDIR)/src/common/$(LIB)


install:
	$(INSTALL_PROG) ftnbsy$(EXE) $(PREFIX)$(LIBDIR)
	$(INSTALL_PROG) ftnconfig$(EXE) $(PREFIX)$(LIBDIR)
	$(INSTALL_PROG) ftnseq$(EXE) $(PREFIX)$(LIBDIR)
	$(INSTALL_PROG) ftnfattach$(EXE) $(PREFIX)$(BINDIR)
	$(INSTALL_PROG) ftnflo$(EXE) $(PREFIX)$(LIBDIR)
	$(INSTALL_PROG) sumcrc$(EXE) $(PREFIX)$(BINDIR)
	$(INSTALL_PROG) ftnlock$(EXE) $(PREFIX)$(LIBDIR)
	$(INSTALL_PROG) ftnlog$(EXE) $(PREFIX)$(LIBDIR)
	$(INSTALL_PROG) ftnoutpkt$(EXE) $(PREFIX)$(BINDIR)
	$(INSTALL_PROG) pktdebug$(EXE) $(PREFIX)$(BINDIR)
	$(INSTALL_PROG) pktmore $(PREFIX)$(BINDIR)


clean veryclean:
	rm -f $(PROGS)
	rm -f *~ *.bak *.o tags TAGS core paths.tmp

check:


#
# Dependencies
#
depend:
	$(CC) $(CFLAGS) -MM *.c >.depend

depend-all:
	$(CC) $(CFLAGS) -M *.c >.depend

ifeq (.depend,$(wildcard .depend))
include .depend
endif

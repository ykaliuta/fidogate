#:ts=8
#
#    $Id: Makefile,v 4.9 2000/04/11 11:32:43 mj Exp $
#
#    Makefile for FIDOGATE src/ffx/
#

#
# Normally it should not be necessary to modify this file, see
# ../config.make and ../rules.make
#

TOPDIR		= ../..

include $(TOPDIR)/config.make
include $(TOPDIR)/rules.make


SCRIPTS		= ffxmail ffxnews ffxrmail runffx send-ffx
PROGS		= ffx ffxqt $(SCRIPTS)


all:		$(PROGS)

ffx:		$(TOPDIR)/src/common/$(LIB)
ffxqt:		$(TOPDIR)/src/common/$(LIB)
ffxbatch:	$(TOPDIR)/src/common/$(LIB)

$(SCRIPTS):	$(TOPDIR)/subst.pl $(TOPDIR)/config.pl


install:
	$(INSTALL_SETUID) ffx$(EXE) $(PREFIX)$(LIBDIR)
	$(INSTALL_PROG) ffxqt$(EXE) $(PREFIX)$(LIBDIR)
	$(INSTALL_PROG) ffxmail $(PREFIX)$(LIBDIR)
	$(INSTALL_PROG) ffxrmail $(PREFIX)$(LIBDIR)
	$(INSTALL_PROG) ffxnews $(PREFIX)$(LIBDIR)
	$(INSTALL_PROG) runffx $(PREFIX)$(BINDIR)
	$(INSTALL_PROG) send-ffx $(PREFIX)$(BINDIR)


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

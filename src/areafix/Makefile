#:ts=8
#
#    $Id: Makefile,v 1.7 2001/01/28 15:53:16 mj Exp $
#
#    Makefile for FIDOGATE src/areafix/
#

#
# Normally it should not be necessary to modify this file, see
# ../config.make and ../rules.make
#

TOPDIR		= ../..

include $(TOPDIR)/config.make
include $(TOPDIR)/rules.make


PROGS		= ftnaf ftnafmail ftnafpkt ftnafutil
SCRIPTS		= runafutil

OBJS_AF		= ftnaf.o areafix.o
OBJS_AFPKT	= ftnafpkt.o areafix.o
OBJS_AFUTIL	= ftnafutil.o areafix.o

LOCAL_CFLAGS	=
LOCAL_LFLAGS	=

TESTS		= testpkttmpl


all:		$(PROGS) $(SCRIPTS)

ftnafmail:	$(TOPDIR)/src/common/$(LIB)

ftnaf:		$(TOPDIR)/src/common/$(LIB) $(OBJS_AF)
	$(CC) $(LFLAGS) $(LOCAL_LFLAGS) -o ftnaf $(OBJS_AF) $(LIBS)

ftnafpkt:	$(TOPDIR)/src/common/$(LIB) $(OBJS_AFPKT)
	$(CC) $(LFLAGS) $(LOCAL_LFLAGS) -o ftnafpkt $(OBJS_AFPKT) $(LIBS)

ftnafutil:	$(TOPDIR)/src/common/$(LIB) $(OBJS_AFUTIL)
	$(CC) $(LFLAGS) $(LOCAL_LFLAGS) -o ftnafutil $(OBJS_AFUTIL) $(LIBS)


install:
	$(INSTALL_PROG) ftnaf$(EXE) $(PREFIX)$(LIBDIR)
	$(INSTALL_PROG) ftnafpkt$(EXE) $(PREFIX)$(LIBDIR)
	$(INSTALL_PROG) ftnafutil$(EXE) $(PREFIX)$(BINDIR)
	$(INSTALL_SETUID) ftnafmail$(EXE) $(PREFIX)$(LIBDIR)
	$(INSTALL_DATA) areafix.help $(PREFIX)$(LIBDIR)
	$(INSTALL_PROG) runafutil $(PREFIX)$(BINDIR)


clean veryclean:
	rm -f $(PROGS) $(TESTS)
	rm -f *~ *.bak *.o tags TAGS core paths.tmp


#
# Test programs
#
check:		$(TESTS)

testpkttmpl:	pkttmpl.o $(TOPDIR)/src/common/$(LIB)
	$(CC) $(LFLAGS) -o testpkttmpl pkttmpl.o $(LIBS)


#
# Dependencies
#
depend:
	$(CC) $(CFLAGS) $(LOCAL_CFLAGS) -MM *.c >.depend

depend-all:
	$(CC) $(CFLAGS) $(LOCAL_CFLAGS) -M *.c >.depend

ifeq (.depend,$(wildcard .depend))
include .depend
endif

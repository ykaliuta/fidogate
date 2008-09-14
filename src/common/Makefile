#:ts=8
#
#    $Id: Makefile,v 4.21 2000/11/17 21:18:06 mj Exp $
#
#    Makefile for FIDOGATE src/common/
#

#
# Normally it should not be necessary to modify this file, see
# ../config.make and ../rules.make
#

TOPDIR		= ../..

include $(TOPDIR)/config.make
include $(TOPDIR)/rules.make


OBJS		= acl.o \
		  address.o \
		  aliases.o \
		  areas.o \
		  areasbbs.o \
		  binkley.o \
		  bounce.o \
		  cf_funcs.o \
		  config.o \
		  crc16.o \
		  crc32.o \
		  date.o \
		  dir.o \
		  file.o \
		  flo.o \
		  fopen.o \
		  ftnaddr.o \
		  getopt.o \
		  getopt1.o \
		  gettime.o \
		  hosts.o \
		  kludge.o \
		  lock.o \
		  log.o \
		  mail.o \
		  message.o \
		  mime.o \
		  misc.o \
		  msgid.o \
		  node.o \
		  outpkt.o \
		  packet.o \
		  parsedate.o \
		  parsenode.o \
		  passwd.o \
		  read.o \
		  rematch.o \
		  rfcaddr.o \
		  rfcheader.o \
		  routing.o \
		  sequencer.o \
		  strtok_r.o \
		  textlist.o \
		  tick.o \
		  tmps.o \
		  version.o \
		  wildmat.o \
		  xalloc.o



LIBOBJS		= $(OBJS:%=$(LIB)(%))

TESTS		= testparse testmatch testflo testheader testdir testtmps \
		  testftnaddr testdate teststrtok testrematch
TESTOBJS	= $(TESTS:%=%.o)


all:		cf_funcs.mak cf_abbrev.mak cf_env.mak $(LIB)


$(LIB):		$(LIBOBJS)
	$(RANLIB) $(LIB)

parsedate.c:	parsedate.y
	@echo "expect 6 shift/reduce conflicts"
	$(YACC) parsedate.y
	mv y.tab.c parsedate.c

cf_funcs.mak:	$(TOPDIR)/config.make $(TOPDIR)/subst.pl
	$(PERL) $(TOPDIR)/subst.pl -c$(TOPDIR)/config.make -C >cf_funcs.mak
	diff cf_funcs.mak cf_funcs.c >/dev/null || cp cf_funcs.mak cf_funcs.c

cf_abbrev.mak:	$(TOPDIR)/config.make $(TOPDIR)/subst.pl
	$(PERL) $(TOPDIR)/subst.pl -c$(TOPDIR)/config.make -A >cf_abbrev.mak
	diff cf_abbrev.mak cf_abbrev.c >/dev/null || cp cf_abbrev.mak cf_abbrev.c

cf_env.mak:	$(TOPDIR)/config.make $(TOPDIR)/subst.pl
	$(PERL) $(TOPDIR)/subst.pl -c$(TOPDIR)/config.make -E >cf_env.mak
	diff cf_env.mak cf_env.c >/dev/null || cp cf_env.mak cf_env.c

install:

clean veryclean:
	rm -f $(PROGS) $(LIB)
	rm -f $(TESTS) $(TESTOBJS)
	rm -f *~ *.bak *.o tags TAGS core


#
# Test programs
#
check:		all $(TESTS)

testparse:	testparse.o $(LIB)
	$(CC) $(LFLAGS) -o testparse testparse.o $(LIBS)
testparse.o:	parsenode.c
	$(CC) $(CFLAGS) $(LOCAL_CFLAGS) -DTEST -c parsenode.c -o testparse.o

testmatch:	testmatch.o $(LIB)
	$(CC) $(LFLAGS) -o testmatch testmatch.o $(LIBS)
testmatch.o:	wildmat.c
	$(CC) $(CFLAGS) $(LOCAL_CFLAGS) -DTEST -c wildmat.c -o testmatch.o

testflo:	testflo.o $(LIB)
	$(CC) $(LFLAGS) -o testflo testflo.o $(LIBS)
testflo.o:	flo.c
	$(CC) $(CFLAGS) $(LOCAL_CFLAGS) -DTEST -c flo.c -o testflo.o

testheader:	testheader.o $(LIB)
	$(CC) $(LFLAGS) -o testheader testheader.o $(LIBS)
testheader.o:	rfcheader.c
	$(CC) $(CFLAGS) $(LOCAL_CFLAGS) -DTEST -c rfcheader.c -o testheader.o

testdir:	testdir.o $(LIB)
	$(CC) $(LFLAGS) -o testdir testdir.o $(LIBS)
testdir.o:	dir.c
	$(CC) $(CFLAGS) $(LOCAL_CFLAGS) -DTEST -c dir.c -o testdir.o

testtmps:	testtmps.o $(LIB)
	$(CC) $(LFLAGS) -o testtmps testtmps.o $(LIBS)
testtmps.o:	tmps.c
	$(CC) $(CFLAGS) $(LOCAL_CFLAGS) -DTEST -c tmps.c -o testtmps.o

testftnaddr:	testftnaddr.o $(LIB)
	$(CC) $(LFLAGS) -o testftnaddr testftnaddr.o $(LIBS)
testftnaddr.o:	ftnaddr.c
	$(CC) $(CFLAGS) $(LOCAL_CFLAGS) -DTEST -c ftnaddr.c -o testftnaddr.o

testdate:	testdate.o $(LIB)
	$(CC) $(LFLAGS) -o testdate testdate.o $(LIBS)
testdate.o:	parsedate.c
	$(CC) $(CFLAGS) $(LOCAL_CFLAGS) -DTEST -c parsedate.c -o testdate.o

teststrtok:	teststrtok.o $(LIB)
	$(CC) $(LFLAGS) -o teststrtok teststrtok.o $(LIBS)
teststrtok.o:	strtok_r.c
	$(CC) $(CFLAGS) $(LOCAL_CFLAGS) -DTEST -c strtok_r.c -o teststrtok.o

testrematch:	testrematch.o $(LIB)
	$(CC) $(LFLAGS) -o testrematch testrematch.o $(LIBS)
testrematch.o:	rematch.c
	$(CC) $(CFLAGS) $(LOCAL_CFLAGS) -DTEST -c rematch.c -o testrematch.o


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

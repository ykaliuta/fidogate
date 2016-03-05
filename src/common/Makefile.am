#
#    $Id: Makefile.am,v 5.4 2006/10/31 21:24:58 anray Exp $
#
#    Makefile for FIDOGATE src/common/
#

AUTOMAKE_OPTIONS = foreign


if enable_static
noinst_LIBRARIES = libfidogate.a

libfidogate_a_CFLAGS = -Wall -g
libfidogate_a_SOURCES = $(SRC)
install-data-am:	install-libLTLIBRARIES
else

lib_LTLIBRARIES = libfidogate.la
libfidogate_la_CFLAGS = -Wall -g -fPIC
endif

libfidogate_la_SOURCES = $(SRC)
libfidogate_la_LDFLAGS = -shared -rpath $(libdir) -soname $(libdir) -version-info 5:0:0

libfidogate_la_LIBADD = @LIBICONV@

AM_CPPFLAGS = -I$(top_srcdir)/src/include

LIBTOOL_DEPS = @LIBTOOL_DEPS@

libtool:
	$(LIBTOOL_DEPS)
	$(SHELL) ../../config.status --recheck

SRC	   = acl.c \
	    acl_ftn.c \
	    active.c \
	    address.c \
	    aliases.c \
	    areas.c \
	    areasbbs.c \
	    binkley.c \
	    bounce.c \
	    cf_funcs.c \
	    charset.c \
	    config.c \
	    crc16.c \
	    crc32.c \
	    date.c \
	    dbz.c \
	    dir.c \
	    file.c \
	    flo.c \
	    fopen.c \
	    ftnaddr.c \
	    getopt.c \
	    getopt1.c \
	    gettime.c \
	    hosts.c \
	    histdb.c \
	    kludge.c \
	    lock.c \
	    log.c \
	    mail.c \
	    message.c \
	    mime.c \
	    misc.c \
	    msgid.c \
	    node.c \
	    outpkt.c \
	    packet.c \
	    parsedate.y \
	    parsenode.c \
	    passwd.c \
	    read.c \
	    rematch.c \
	    rfcaddr.c \
	    rfcheader.c \
	    routing.c \
	    sequencer.c \
	    spyes.c \
	    strtok_r.c \
	    textlist.c \
	    tick.c \
	    tmps.c \
	    uplinks.c \
	    version.c \
	    wildmat.c \
	    xalloc.c \
	    xstrnlen.c

distclean-am:
	rm -f cf_* parsedate.c parsedate.h

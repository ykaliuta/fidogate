#
#    $Id: Makefile.am,v 5.4 2006/10/31 21:24:58 anray Exp $
#
#    Makefile for FIDOGATE src/common/
#

noinst_LIBRARIES = libfidogate.a
libfidogate_a_LIBADD = @LIBICONV@

libfidogate_a_SOURCES = $(SRC)

AM_CPPFLAGS = -I$(top_srcdir)/src/include

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

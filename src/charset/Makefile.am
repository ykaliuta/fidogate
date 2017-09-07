#:ts=8
#
#    $Id: Makefile.am,v 5.2 2004/11/23 00:50:39 anray Exp $
#
#    Makefile for FIDOGATE src/charset/
#

AM_CPPFLAGS		= -I$(top_srcdir)/src/include

if ENABLE_STATIC
LDADD			= $(top_srcdir)/src/common/libfidogate.a
else
LDADD			= $(top_srcdir)/src/common/libfidogate.la
endif

AM_CFLAGS		= -Wall

libexec_PROGRAMS	= charsetc
libexec_EXTRA		= charset.bin

charset.bin:
	./charsetc -v charset.map charset.bin

clean-am:
	rm -f charset.bin

installcheck-am:
	chmod 0644 $(DESTDIR)$(libexecdir)/charset.bin

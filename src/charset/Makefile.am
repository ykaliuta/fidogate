#:ts=8
#
#    $Id: Makefile.am,v 5.2 2004/11/23 00:50:39 anray Exp $
#
#    Makefile for FIDOGATE src/charset/
#

INCLUDES		= -I$(top_srcdir)/src/include

if enable_static
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

install-data-am:
	chmod 644 $(DESTDIR)$(libexecdir)/charset.bin

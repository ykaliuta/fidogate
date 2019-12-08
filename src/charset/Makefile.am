#:ts=8
#
#    $Id: Makefile.am,v 5.2 2004/11/23 00:50:39 anray Exp $
#
#    Makefile for FIDOGATE src/charset/
#

AM_CPPFLAGS		= -I$(top_srcdir)/src/include

LDADD			= $(top_srcdir)/src/common/libfidogate.a

AM_CFLAGS		= -Wall

libexec_PROGRAMS	= charsetc
libexec_SCRIPTS		= charset.bin

charset.bin: charsetc
	./charsetc -v charset.map charset.bin

clean-am:
	rm -f charset.bin

installcheck-am:
	chmod 0644 $(DESTDIR)$(libexecdir)/charset.bin

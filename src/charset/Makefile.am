#:ts=8
#
#    $Id$
#
#    Makefile for FIDOGATE src/charset/
#

INCLUDES		= -I$(top_srcdir)/src/include

if enable_static
LDADD			= $(top_srcdir)/src/common/libfidogate.a
else
LDADD			= $(top_srcdir)/src/common/libfidogate.la
endif

CFLAGS			= -Wall

noinst_PROGRAMS		= charsetc
libexec_DATA		= charset.bin

charset.bin:
	./charsetc -v charset.map charset.bin

clean-am:
	rm -f charset.bin

install-data-am:
	chmod 644 $(DESTDIR)$(libexecdir)/charset.bin

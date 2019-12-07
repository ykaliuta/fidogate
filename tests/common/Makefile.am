
TOP_SRC = ../../src

AM_CPPFLAGS = -I$(TOP_SRC)/include
AM_CPPFLAGS += -I$(TOP_SRC)/common

LDADD = -lcgreen
LDFLAGS = -no-install

TESTS = test_log test_config test_packet test_mime test_charset
check_PROGRAMS = test_log test_config test_packet test_mime test_charset

# dependencies, fetched by xalloc.c + read.c and xstrnlen.c
LIBFIDOGATE_MIN = $(top_srcdir)/src/common/xalloc.c \
	$(top_srcdir)/src/common/hosts.c \
	$(top_srcdir)/src/common/uplinks.c \
	$(top_srcdir)/src/common/areasbbs.c \
	$(top_srcdir)/src/common/config.c \
	$(top_srcdir)/src/common/cf_funcs.c \
	$(top_srcdir)/src/common/passwd.c \
	$(top_srcdir)/src/common/fopen.c \
	$(top_srcdir)/src/common/strtok_r.c \
	$(top_srcdir)/src/common/parsenode.c \
	$(top_srcdir)/src/common/node.c \
	$(top_srcdir)/src/common/charset.c \
	$(top_srcdir)/src/common/misc.c \
	$(top_srcdir)/src/common/wildmat.c \
	$(top_srcdir)/src/common/address.c \
	$(top_srcdir)/src/common/tmps.c \
	$(top_srcdir)/src/common/rfcaddr.c \
	$(top_srcdir)/src/common/acl.c \
	$(top_srcdir)/src/common/routing.c \
	$(top_srcdir)/src/common/read.c \
	$(top_srcdir)/src/common/xstrnlen.c \
	$(top_srcdir)/src/common/textlist.c

test_log_SOURCES = test-log.c
test_config_SOURCES = test-config.c
test_packet_SOURCES = test-packet-doubles.c
test_packet_SOURCES += test-packet.c \
	$(LIBFIDOGATE_MIN) \
	$(top_srcdir)/src/common/packet.c \
	$(top_srcdir)/src/common/sequencer.c \
	$(top_srcdir)/src/common/binkley.c \
	$(top_srcdir)/src/common/parsedate.c \
	$(top_srcdir)/src/common/date.c \
	$(top_srcdir)/src/common/gettime.c \
	$(top_srcdir)/src/common/version.c \
	$(top_srcdir)/src/common/lock.c \
	$(top_srcdir)/src/common/flo.c

test_mime_SOURCES = test-mime.c \
	$(LIBFIDOGATE_MIN) \
        $(top_srcdir)/src/common/rfcheader.c \
        $(top_srcdir)/src/common/mime.c

test_charset_SOURCES = test-charset.c \
	$(LIBFIDOGATE_MIN)

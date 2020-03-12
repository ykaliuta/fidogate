TOP_SRC = $(top_srcdir)/src

AM_CPPFLAGS = -I$(TOP_SRC)/include
AM_CPPFLAGS += -I$(TOP_SRC)/common

check_PROGRAMS = cmp-pkt
cmp_pkt_SOURCES = cmp-pkt.c
cmp_pkt_LDADD = $(top_srcdir)/src/common/libfidogate.a

LDADD = -lcgreen @LIBICONV@

TESTS = $(TESTS_COMMON) $(TESTS_GATE)

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

# common
BINARIES_COMMON = common/test_log common/test_config common/test_packet \
		  common/test_mime common/test_charset common/test_date
TESTS_COMMON = $(BINARIES_COMMON)
check_PROGRAMS += $(BINARIES_COMMON)

common_test_log_SOURCES = common/test-log.c
common_test_config_SOURCES = common/test-config.c
common_test_packet_SOURCES = common/test-packet-doubles.c
common_test_packet_SOURCES += common/test-packet.c \
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

common_test_mime_SOURCES = common/test-mime.c \
	$(LIBFIDOGATE_MIN) \
        $(top_srcdir)/src/common/rfcheader.c \
        $(top_srcdir)/src/common/mime.c

common_test_charset_SOURCES = common/test-charset.c \
	$(LIBFIDOGATE_MIN)

common_test_date_SOURCES = common/test-date.c \
	$(LIBFIDOGATE_MIN) \
	$(top_srcdir)/src/common/parsedate.c \
	$(top_srcdir)/src/common/gettime.c \
	$(top_srcdir)/src/common/date.c \
	$(top_srcdir)/src/common/log.c


# gate
AM_CPPFLAGS += -I$(TOP_SRC)/gate
BINARIES_GATE = gate/test_rfc2ftn
check_PROGRAMS += $(BINARIES_GATE)
TESTS_GATE = $(BINARIES_GATE) gate/test_rfc2ftn.sh gate/test_ftn2rfc.sh

gate_test_rfc2ftn_LDADD = $(TOP_SRC)/common/libfidogate.a
gate_test_rfc2ftn_SOURCES = gate/test-rfc2ftn.c \
			gate/rfc2ftn-wrapper.c gate/rfc2ftn-stubs.c

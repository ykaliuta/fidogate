
TOP_SRC = $(top_srcdir)/src

AM_CPPFLAGS = -I$(TOP_SRC)/include
AM_CPPFLAGS += -I$(TOP_SRC)/gate

LDADD = -lcgreen $(TOP_SRC)/common/libfidogate.la

TESTS = test_rfc2ftn test_rfc2ftn.sh test_ftn2rfc.sh

check_PROGRAMS = test_rfc2ftn
test_rfc2ftn_SOURCES = test-rfc2ftn.c rfc2ftn-wrapper.c rfc2ftn-stubs.c

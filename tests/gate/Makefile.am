
TOP_SRC = $(top_srcdir)/src

AM_CPPFLAGS = -I$(TOP_SRC)/include
AM_CPPFLAGS += -I$(TOP_SRC)/gate

LDADD = -lcgreen $(TOP_SRC)/common/libfidogate.la

TESTS = test_rfc2ftn

check_PROGRAMS = test_buffered_printf test_rfc2ftn
test_rfc2ftn_SOURCES = test-rfc2ftn.c rfc2ftn-wrapper.c
test_rfc2ftn_SOURCES += buffered-printf.c mock-log.c
test_rfc2ftn_SOURCES += rfc2ftn-stubs.c

test_buffered_printf_SOURCES = test-buffered-printf.c buffered-printf.c

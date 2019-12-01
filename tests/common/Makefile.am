
TOP_SRC = ../../src

AM_CPPFLAGS = -I$(TOP_SRC)/include
AM_CPPFLAGS += -I$(TOP_SRC)/common

LDADD = -lcgreen $(TOP_SRC)/common/libfidogate.la

TESTS = test_log test_config test_packet test_mime test_charset
check_PROGRAMS = test_log test_config test_packet test_mime test_charset
test_log_SOURCES = test-log.c
test_config_SOURCES = test-config.c
# packet.c binary code comes from the library
test_packet_SOURCES = test-packet-doubles.c
test_packet_SOURCES += test-packet.c
test_mime_SOURCES = test-mime.c
test_charset_SOURCES = test-charset.c

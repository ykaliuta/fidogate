
TOP_SRC = ../../src

AM_CPPFLAGS = -I$(TOP_SRC)/include
AM_CPPFLAGS += -I$(TOP_SRC)/common

LDADD = -lcgreen

TESTS = test_log
check_PROGRAMS = tests test_log
test_log_SOURCES = test-log-main.c test-log.c

tests_SOURCES = tests.c test-log.c




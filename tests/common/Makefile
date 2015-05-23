
TOP_SRC := ../../src

INCS := $(TOP_SRC)/include
INCS += $(TOP_SRC)/common
INCS += $(TOP_SRC)/..

CFLAGS := -g -O0 -Wall -Wno-format-security

CFLAGS += $(patsubst %,-I%,$(INCS))

LDFLAGS += -lcgreen

BINS :=
SRCS :=

include Makefile.inc.*

OBJS := $(patsubst %.c,%.o,$(SRCS))

all: build-all run-all

.PHONY: build-all run-all

run-all: build-all
	@for i in $(BINS); do \
		./$$i; \
	done

$(OBJS): $(wildcard test-*.c)

build-all: FORCE $(BINS)

%.i: %.c
	$(CC) $(CFLAGS) -E -dD -o $@ $^

clean:
	rm -f $(BINS) *.o

FORCE:


TOP_SRC := ../../src

BIN := tests
INCS := $(TOP_SRC)/include
INCS += $(TOP_SRC)/common
INCS += $(TOP_SRC)/..

CFLAGS := -g -O0 -Wall -Wno-format-security

CFLAGS += $(patsubst %,-I%,$(INCS))

LDFLAGS += -lcgreen

SRCS := tests.c test-log.c
OBJS := $(patsubst %.c,%.o,$(SRCS))

all: build-all run-all

.PHONY: build-all run-all

run-all: build-all
	./$(BIN)

$(OBJS): $(wildcard test-*.c)
$(BIN): $(OBJS)

build-all: FORCE $(BIN)

%.i: %.c
	$(CC) $(CFLAGS) -E -dD -o $@ $^

clean:
	rm -f $(BIN) *.o

FORCE:

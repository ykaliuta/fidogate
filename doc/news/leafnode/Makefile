CC = gcc
CFLAGS = -D_BSD
DIRS = lib new_rnews send-fidogate leafnode-group
INSTALL_DIRS = new_rnews send-fidogate leafnode-group
DESTDIR = /usr/local/fidogate/leafnode
DESTMAN = /usr/local/man/man1

all:
	${CC} ctrld.c -o ctrld
	for d in ${DIRS}; do (cd $$d && echo $$d && ${MAKE} CC=$(CC) CFG=$(CFLAGS) DESTDIR=$(DESTDIR) DESTMAN=$(DESTMAN)) || exit; done;

clean:
	rm -f ctrld
	for d in ${DIRS}; do (cd $$d && echo $$d && ${MAKE}  CC=$(CC) CFG=$(CFLAGS) DESTDIR=$(DESTDIR) DESTMAN=$(DESTMAN) $@) || exit; done;

save:
	save common.h
	for d in ${DIRS}; do (cd $$d && echo $$d && ${MAKE}  CC=$(CC) CFG=$(CFLAGS) DESTDIR=$(DESTDIR) DESTMAN=$(DESTMAN) $@) || exit; done;

install:
	install -o news -g news -m 750 ctrld ${DESTDIR}/ctrld
	for d in ${INSTALL_DIRS}; do (cd $$d && echo $$d && ${MAKE}  CC=$(CC) CFG=$(CFLAGS) DESTDIR=$(DESTDIR) DESTMAN=$(DESTMAN) $@) || exit; done;

uninstall:
	rm -f ${DESTDIR}/ctrld
	for d in ${INSTALL_DIRS}; do (cd $$d && echo $$d && ${MAKE}  CC=$(CC) CFG=$(CFLAGS) DESTDIR=$(DESTDIR) DESTMAN=$(DESTMAN) $@) || exit; done;

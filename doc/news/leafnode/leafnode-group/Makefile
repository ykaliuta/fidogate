CFLAGS = -O $(CFG)
LDFLAGS = -s
LIBS = ../lib/nntpftn.a

I = /usr/include

SRC_leafnode =  leafnode-group.c movefile.c util.c

OBJ_leafnode =  leafnode-group.o movefile.o util.o

leafnode-group: $(OBJ_leafnode)
	$(CC) $(LDFLAGS) -o leafnode-group $(OBJ_leafnode) $(LIBS)

leafnode-group.o:leafnode-group.c ../common.h 
movefile.o:     movefile.c      
util.o:         util.c          ../common.h 

clean:
	rm -f $(OBJ_leafnode) leafnode-group core 0 back.out

save:
	save $(SRC_leafnode) Makefile  *.1
uninstall:
	rm -f $(DESTDIR)/leafnode-group
install:
	install -o news -g news -m 750 leafnode-group $(DESTDIR)/leafnode-group
	cp leafnode-group.1 $(DESTMAN)/leafnode-group.1

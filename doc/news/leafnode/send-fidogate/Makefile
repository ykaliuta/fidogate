CFLAGS = -O2 $(CFG)
LDFLAGS = -s
LIBS = ../lib/nntpftn.a

I = /usr/include

SRC_send-fidogate =  send-fidogate.c

OBJ_send-fidogate =  send-fidogate.o

send-fidogate:  $(OBJ_send-fidogate) $(LIBS)
	$(CC) $(LDFLAGS) -o send-fidogate $(OBJ_send-fidogate) $(LIBS)

send-fidogate.o:  send-fidogate.c ../common.h ../config.h

clean:
	rm -f $(OBJ_send-fidogate) core back.out 0 send-fidogate

save:
	save $(SRC_send-fidogate)
	
install:
	install -o news -g news -m 755 send-fidogate $(DESTDIR)/send-fidogate
	chmod gu+s $(DESTDIR)/send-fidogate
uninstall:
	rm -f $(DESTDIR)/send-fidogate
	rm -f $(DESTMAN)/send-fidogate.1	

CC = gcc
CFLAGS = -O2 $(CFG) #-DDEBUG #-DREMOVE_SUCCESS
LDFLAGS = -s
LIBS = ../lib/nntpftn.a
I = /usr/include

SRC_rnews =  readnews.c rnews.c
SRC_m2news = mail2news.c m2news.c

OBJ_rnews =  readnews.o rnews.o 
OBJ_m2news = mail2news.o m2news.o

all:           leafnode-rnews m2news

leafnode-rnews:        $(OBJ_rnews) $(LIBS)
	$(CC) $(LDFLAGS) -o leafnode-rnews $(OBJ_rnews) $(LIBS)

m2news:        $(OBJ_m2news) $(LIBS)
	$(CC) $(LDFLAGS) -o m2news $(OBJ_m2news) $(LIBS)

readnews.o:     readnews.c      ../common.h ../config.h
rnews.o:        rnews.c         ../common.h ../config.h
m2news.o:       m2news.c        ../common.h ../config.h
mail2news.o:    mail2news.c     ../common.h ../config.h

install:	leafnode-rnews m2news
	install -o news -g news -m 755 leafnode-rnews $(DESTDIR)/leafnode-rnews
	install -o root -g news -m 755 m2news $(DESTDIR)/m2news
	chmod gu+s $(DESTDIR)/leafnode-rnews
	chmod gu+s $(DESTDIR)/m2news

uninstall:	leafnode-rnews m2news
	rm -f $(DESTDIR)/leafnode-rnews
	rm -f $(DESTDIR)/m2news

save:
	save $(SRC_rnews) $(SRC_m2news)

clean:
	rm -f $(OBJ_rnews) $(OBJ_m2news) leafnode-rnews m2news core back.out 0

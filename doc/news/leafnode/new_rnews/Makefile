CC = gcc
CFLAGS = -O2 $(CFG) #-DDEBUG #-DREMOVE_SUCCESS
LDFLAGS = -s
LIBS = ../lib/nntpftn.a
I = /usr/include

SRC_rnews =  readnews.c rnews.c
SRC_m2news = mail2news.c m2news.c

OBJ_rnews =  readnews.o rnews.o 
OBJ_m2news = mail2news.o m2news.o

HDR_rnews =  common.h

all:           rnews m2news

rnews:        $(OBJ_rnews) $(LIBS)
	$(CC) $(LDFLAGS) -o rnews $(OBJ_rnews) $(LIBS)

m2news:        $(OBJ_m2news) $(LIBS)
	$(CC) $(LDFLAGS) -o m2news $(OBJ_m2news) $(LIBS)

readnews.o:     readnews.c      ../common.h 
rnews.o:        rnews.c         ../common.h 
m2news.o:       m2news.c        ../common.h 
mail2news.o:    mail2news.c     ../common.h 

install:	rnews m2news
	install -o news -g news -m 755 rnews $(DESTDIR)/rnews
	install -o root -g root -m 755 m2news $(DESTDIR)/m2news
	chmod u+s $(DESTDIR)/rnews
	chmod u+s $(DESTDIR)/m2news
	cp rnews.1 m2news.1 $(DESTMAN)

uninstall:	rnews m2news
	rm -f $(DESTDIR)/rnews
	rm -f $(DESTDIR)/m2news
	rm -f $(DESTMAN)/rnews.1 $(DESTMAN)/m2news.1

save:
	save $(SRC_rnews) $(SRC_m2news) $(HDR_rnews) *.1

clean:
	rm -f $(OBJ_rnews) $(OBJ_m2news) rnews m2news core back.out 0

CFLAGS = -O2 $(CFG) #-DDEBUG
LDFLAGS = -s
LIBS = 
I = /usr/include

SRC_a =  message.c nntplib.c nntputil.c utilmsgid.c diagtime.c getini.c \
	config.c alltrim.c isnum.c

OBJ_a =  message.o nntplib.o nntputil.o utilmsgid.o diagtime.o getini.o \
	config.o alltrim.o isnum.o

nntpftn.a:	$(OBJ_a)
	ar -r nntpftn.a $(OBJ_a)
	ranlib nntpftn.a

message.o:      message.c       ../common.h ../config.h 
nntplib.o:      nntplib.c       ../common.h ../config.h 
nntputil.o:     nntputil.c      ../common.h ../config.h 
utilmsgid.o:    utilmsgid.c     ../common.h ../config.h 
diagtime.o:     diagtime.c
alltrim.o:      alltrim.c
isnum.o:        isnum.c
getini.o:       getini.c        ../common.h ../config.h getini.h
config.o:       config.c        ../common.h ../config.h getini.h

save:
	save $(SRC_a)
	
clean:
	rm -f $(OBJ_a) core back.out 0 nntpftn.a

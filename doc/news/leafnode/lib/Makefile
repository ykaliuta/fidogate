CFLAGS = -O2 $(CFG) #-DDEBUG
LDFLAGS = -s
LIBS = 
I = /usr/include

SRC_a =  message.c nntplib.c nntputil.c utilmsgid.c diagtime.c

OBJ_a =  message.o nntplib.o nntputil.o utilmsgid.o diagtime.o

nntpftn.a:	$(OBJ_a)
	ar -r nntpftn.a $(OBJ_a)
	ranlib nntpftn.a

message.o:      message.c       ../common.h 
nntplib.o:      nntplib.c       ../common.h 
nntputil.o:     nntputil.c      ../common.h 
utilmsgid.o:    utilmsgid.c     ../common.h 
diagtime.o:     diagtime.c

save:
	save $(SRC_a)
	
clean:
	rm -f $(OBJ_a) core back.out 0 nntpftn.a

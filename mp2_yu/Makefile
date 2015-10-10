all: adventure tr mp2photo mp2object

HEADERS=assert.h input.h modex.h photo.h photo_headers.h text.h types.h \
	world.h Makefile
OBJS=adventure.o assert.o modex.o input.o photo.o text.o world.o

CFLAGS=-g -Wall

adventure: ${OBJS}
	gcc -g -o adventure ${OBJS} -lpthread -lrt

tr: modex.c ${HEADERS} text.o
	gcc ${CFLAGS} -DTEXT_RESTORE_PROGRAM=1 -o tr modex.c text.o

mp2photo: ${HEADERS}
	gcc ${CFLAGS} -o mp2photo mp2photo.c

mp2object: ${HEADERS}
	gcc ${CFLAGS} -DWRITE_OBJECT_IMAGE=1 -o mp2object mp2photo.c

%.o: %.c ${HEADERS}
	gcc ${CFLAGS} -c -o $@ $<

%.o: %.s ${HEADERS}
	gcc ${CFLAGS} -c -o $@ $<

clean::
	rm -f *.o *~ a.out

test:
	gcc -o test input.c

clear: clean
	rm -f adventure tr mp2photo mp2object

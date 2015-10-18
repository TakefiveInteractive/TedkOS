CFLAGS += -Wall -nostdlib -ffreestanding
LDFLAGS += -nostdlib -ffreestanding
CC = gcc

ALL: cat grep hello ls pingpong counter shell sigtest testprint syserr

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.S
	$(CC) $(CFLAGS) -c -Wall -o $@ $<

%.exe: ece391%.o ece391syscall.o ece391support.o
	$(CC) $(LDFLAGS) -o $@ $^

%: %.exe
	../elfconvert $<
	mv $<.converted to_fsdir/$@

clean::
	rm -f *~ *.o

clear: clean
	rm -f *.converted
	rm -f *.exe
	rm -f to_fsdir/*

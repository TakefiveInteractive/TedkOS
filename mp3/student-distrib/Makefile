# Makefile for OS project
# To build, first `make dep`, then `make`. Everything should be automatic.
# Will compile all *.c and *.S files in the current directory.

OUTDIR = output/

# Flags to use when compiling, preprocessing, assembling, and linking
CFLAGS 	+= -Wall -fno-stack-protector -nostdlib -fno-exceptions -ffreestanding
ASFLAGS +=
LDFLAGS += -nostdlib -static
CC=`./detect-gcc.sh`

#If you have any .h files in another directory, add -I<dir> to this line
CPPFLAGS +=-g -I.

# This generates the list of source files
SRC =  $(wildcard **/*.S) $(wildcard **/*.c) $(wildcard **/*.cpp) $(wildcard *.S) $(wildcard *.c) $(wildcard *.cpp)

# This generates the list of .o files. The order matters, boot.o must be first
OBJS      = boot.o
OBJS     += $(filter-out boot.o,$(patsubst %.S,%.o,$(filter %.S,$(SRC))))
OBJS     += $(patsubst %.c,%.o,$(filter %.c,$(SRC)))

bootimg: Makefile $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -Ttext=0x400000 -o $(OUTDIR)/bootimg
	(cd $(OUTDIR); sudo ./debug.sh)

dep: Makefile.dep

Makefile.dep: $(SRC)
	$(CC) -MM $(CPPFLAGS) $(SRC) > $@

.PHONY: clean, tidy
clean:
	rm -f $(OBJS) Makefile.dep $(OUTDIR)/bootimg

tidy: clean
	rm -f $(OUTDIR)/mp3.img

ifneq ($(MAKECMDGOALS),dep)
ifneq ($(MAKECMDGOALS),clean)
include Makefile.dep
endif
endif

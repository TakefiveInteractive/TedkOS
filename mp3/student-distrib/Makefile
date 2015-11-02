# Makefile for OS project
# To build, first `make dep`, then `make`. Everything should be automatic.
# Will compile all *.c and *.S files in the current directory.

OUTDIR = output/

release: CFLAGS_EXTRA = -ffat-lto-objects -flto -fno-use-linker-plugin -O2
release: bootimg

# Flags to use when compiling, preprocessing, assembling, and linking
CFLAGS 	+= -g $(CFLAGS_EXTRA) -Wall -fno-stack-protector -nostdlib -fno-exceptions -ffreestanding
ASFLAGS +=
LDFLAGS += -g $(CFLAGS_EXTRA) -nostdlib -static -Wno-attributes
CC=`./detect-gcc.sh`
CXX=`./detect-gcc.sh`

#If you have any .h files in another directory, add -I<dir> to this line
CPPFLAGS += -I. -g
CXXFLAGS += -g $(CFLAGS_EXTRA) -Wall -std=c++11 -nostdlib -fno-exceptions -ffreestanding -ftemplate-depth=4096 -fno-rtti -I.

CRTBEGIN_OBJ:=$(shell $(CC) $(CFLAGS) -print-file-name=crtbegin.o)
CRTEND_OBJ:=$(shell $(CC) $(CFLAGS) -print-file-name=crtend.o)
CRTI_OBJ=crti.o
CRTN_OBJ=crtn.o

# This generates the list of source files
SRC =  $(wildcard **/*.S) $(wildcard **/*.c) $(wildcard *.S) $(wildcard *.c)
SRC_CPP =  $(wildcard **/*.cpp) $(wildcard *.cpp)

# This generates the list of .o files. The order matters, boot.o must be first
OBJS     = $(filter-out crtn.o, $(filter-out crti.o, $(filter-out boot.o,$(patsubst %.S,%.o,$(filter %.S,$(SRC))))))
OBJS     += $(patsubst %.c,%.o,$(filter %.c,$(SRC)))
OBJS     += $(patsubst %.cpp,%.o,$(filter %.cpp,$(SRC_CPP)))

INTERNAL_OBJS:=boot.o $(CRTI_OBJ) $(OBJS) $(CRTN_OBJ)
ALL_OBJS:=boot.o $(CRTI_OBJ) $(CRTBEGIN_OBJ) $(OBJS) $(CRTEND_OBJ) $(CRTN_OBJ)


bootimg: Makefile $(ALL_OBJS)
	$(CC) $(LDFLAGS) $(ALL_OBJS) -Wl,--section-start=.init=400000 -Ttext=0x400010 -o $(OUTDIR)/bootimg
	(cd $(OUTDIR); ./debug.sh)

debug: bootimg

dep: Makefile.dep

Makefile.dep: $(SRC)
	$(CC) -MM $(CFLAGS) $(SRC) > $@

.PHONY: clean, tidy
clean:
	rm -f $(INTERNAL_OBJS) Makefile.dep $(OUTDIR)/bootimg

tidy: clean
	rm -f $(OUTDIR)/mp3.img

ifneq ($(MAKECMDGOALS),dep)
ifneq ($(MAKECMDGOALS),clean)
include Makefile.dep
endif
endif

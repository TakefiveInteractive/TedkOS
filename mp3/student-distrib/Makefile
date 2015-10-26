# Makefile for OS project
# To build, first `make dep`, then `make`. Everything should be automatic.
# Will compile all *.c and *.S files in the current directory.

LIBGTEST_DIR = ../../gtest
LIBGTEST_SRC = $(LIBGTEST_DIR)/src
LIBGTEST_INC = $(LIBGTEST_DIR)/include
GTEST_DEP =	$(LIBGTEST_SRC)/*.cc \
			$(LIBGTEST_SRC)/*.h \
			$(LIBGTEST_INC)/gtest/*.h \
			$(LIBGTEST_INC)/gtest/internal/*.h
GTESTS = gtest-all.o gtest-main.o

# Flags passed to the preprocessor.
CPPFLAGS += -isystem $(LIBGTEST_INC)
# Flags passed to the C++ compiler.
CXXFLAGS += -g -Wall -Wextra -pthread

gtest-all.o: $(GTEST_DEP)
	$(CXX) $(CPPFLAGS) -I $(LIBGTEST_DIR) $(CXXFLAGS) -c \
		$(LIBGTEST_SRC)/gtest-all.cc -o $@

gtest-main.o: $(GTEST_DEP)
	$(CXX) $(CPPFLAGS) -I $(LIBGTEST_DIR) $(CXXFLAGS) -c \
		$(LIBGTEST_SRC)/gtest_main.cc -o $@

#-----------------------------------------------------
#-----------------------------------------------------
#-----------------Define Your Rules-------------------
#-----------------------------------------------------
#-----------------------------------------------------

OUTDIR = output/

# Flags to use when compiling, preprocessing, assembling, and linking
CFLAGS 	+= -Wall -fno-stack-protector -nostdlib -fno-exceptions -ffreestanding
ASFLAGS +=
LDFLAGS += -nostdlib -static
CC=`./detect-gcc.sh`
CXX=`./detect-gcc.sh`

#If you have any .h files in another directory, add -I<dir> to this line
CPPFLAGS += -g -I.
CXXFLAGS += -Wall -std=c++11 -nostdlib -fno-exceptions -ffreestanding -ftemplate-depth=4096 -fno-rtti -I.

# This generates the list of source files
SRC =  $(wildcard **/*.S) $(wildcard **/*.c) $(wildcard *.S) $(wildcard *.c)
SRC_CPP =  $(wildcard **/*.cpp) $(wildcard *.cpp)

# This generates the list of .o files. The order matters, boot.o must be first
OBJS      = boot.o
OBJS     += $(filter-out boot.o,$(patsubst %.S,%.o,$(filter %.S,$(SRC))))
OBJS     += $(patsubst %.c,%.o,$(filter %.c,$(SRC)))
OBJS     += $(patsubst %.cpp,%.o,$(filter %.cpp,$(SRC_CPP)))

TEST_CPP =	pic/list.c
TEST_H =	pic/list.h inc/i8259.h inc/i8259_extra.h
TESTER =	pic/tests.cpp
TEST_SRC = $(TEST_CPP) $(TEST_H)

# Builds a test.  A test should link with either gtest-all.o or "gtest-main.o and gtest-all.o" , depending on whether it defines its own main() function.
pictest: gtest-all.o gtest-main.o $(TESTER) $(TEST_SRC)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -lpthread $< -o $@

bootimg: Makefile $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -Ttext=0x400000 -o $(OUTDIR)/bootimg
	(cd $(OUTDIR); sudo ./debug.sh)

dep: Makefile.dep

Makefile.dep: $(SRC)
	$(CC) -MM $(CFLAGS) $(SRC) > $@

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

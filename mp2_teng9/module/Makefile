# Makefile for tux controller driver
# Created for ECE 391 - Fall 2007
# By Andrew Ofisher

obj-m += tuxctl.o 
tuxctl-objs := tuxctl-ioctl.o tuxctl-ld.o

KERNEL_DIR := /home/user/build

all:
	make -C $(KERNEL_DIR) M=$(PWD) modules
	# for simplicity when using GDB, make a copy in Linux source dir
	cp -f tuxctl.o $(KERNEL_DIR)

clean::
	make -C $(KERNEL_DIR) M=$(PWD) clean

clear: clean
	rm -f Module.symvers

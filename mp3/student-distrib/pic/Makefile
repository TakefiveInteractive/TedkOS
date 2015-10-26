tests: tests.cpp list.h list.c ../klibs/lib.c ../inc/i8259.h ../inc/i8259_extra.h
	gcc -c list.c -I.. -DGTEST_DEBUG
	g++ tests.cpp list.o -lgtest_main -lgtest -I/usr/local/include/ -L/usr/local/lib -o tests

clean:
	rm tests

.PHONY: clean

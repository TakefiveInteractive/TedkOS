tests:
	gcc -c list.c -I.. -DGTEST_DEBUG
	g++ tests.cpp list.o -lgtest_main -lgtest -I/usr/local/include/ -L/usr/local/lib -o tests
	rm list.o

clean:
	rm tests

.PHONY: clean tests

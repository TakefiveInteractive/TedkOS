tests:
	gcc -ggdb -c list.c -I.. -DGTEST_DEBUG
	g++ -ggdb tests.cpp list.o -lgtest_main -lgtest -I/Users/zyu/.local/include/ -L/Users/zyu/.local/lib -o tests -I..
	rm list.o
	valgrind ./tests
	rm -Rf tests.dSYM

clean:
	rm tests

.PHONY: clean tests

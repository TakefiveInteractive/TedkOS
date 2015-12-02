tests:
	gcc -ggdb -c list.c -I.. -DGTEST_DEBUG
	g++ -ggdb -DGTEST_DEBUG tests.cpp list.o -lgtest_main -lgtest -I"${HOME}/.local/include/" -L"${HOME}/.local/lib" -o tests.exe -I..
	rm list.o
	valgrind ./tests.exe
	rm -Rf tests.dSYM

clean:
	rm tests.exe

.PHONY: clean tests

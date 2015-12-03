tests:
	g++ -DGTEST_DEBUG -ggdb tests.cpp -lgtest_main -lgtest -I"${HOME}/.local/include/" -L"${HOME}/.local/lib" -o tests.exe -I..
	valgrind ./tests.exe
	rm -Rf tests.dSYM

clean:
	rm tests.exe

.PHONY: clean tests

all: test

test: test.cpp strutil.h ctstr.h
	clang++ -g -std=c++11 test.cpp -o test
	./test

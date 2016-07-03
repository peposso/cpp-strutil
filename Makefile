
test: test.cpp strutil.h
	clang++ -g -std=c++11 test.cpp -o test
	./test

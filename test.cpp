#include "strutil.h"
#include <iostream>
#include <stdio.h>


struct p_ {
    template<class T>
    void operator<<(const T& s) const {
        std::cout << s << std::endl;
    }
};
const auto p = p_();

int main()
{
    using namespace strutil;

    p << lower("!\"#$%&'()-=^~|AbCdEfGhIjKlMnOpQrStUvWxYz(){}[]+-*/");
    p << upper("!\"#$%&'()-=^~|aBcDeFgHiJkLmNoPqRsTuVwXyZ(){}[]+-*/");
    p << ltrim("  test  ") + "|";
    p << rtrim("  test  ") + "|";
    p << trim("  test  ") + "|";
    for (const auto s: split("this is a pen.", ' ')) {
        p << s;
    }

    // int
    p << format("d=[%012d]", -123);
    printf("d=[%012d]\n", -123);

    // oct
    p << format("o=[%o]", 0777);
    printf("o=[%o]\n", 0777);

    // bin
    p << format("b=[%08b]", 127);

    // str
    p << format("s=[%7s]", "hello");
    p << format("s=[%7s]", std::string("hello"));
    printf("s=[%7s]\n", "hello");

    // hex
    p << format("x=[%04x]", 255);
    printf("x=[%04x]\n", 255);

    // pointer
    auto ptr = new std::string();
    p << format("p=[%p]", ptr);
    printf("p=[%p]\n", ptr);
    delete(ptr);

    // uint
    p << format("u=[%u]", 123);
    printf("u=[%u]\n", 123);

    // float
    p << format("f=[%-8.2f]", -123.123);
    printf("f=[%-8.2f]\n", -123.123);

    return 0;
}

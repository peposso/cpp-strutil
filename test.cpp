#include <iostream>
#include <stdio.h>
#include "strutil.h"
#include "ctstr.h"


struct p_ {
    template<class T>
    p_ const& operator<<(const T& s) const {
        std::cout << s << std::endl;
        return *this;
    }
};
const auto p = p_();


constexpr auto s = ctstr::make("hello, world");
constexpr auto f = s.find(',');
constexpr auto s2 = s.slice(0, f);
constexpr auto s3 = s2.concat("-world.");
constexpr auto s4 = s3 + ctstr::ctstr<5>('!');

constexpr auto sc = ctstr::make("0.1.2.3");
constexpr auto b1 = sc.compare("0.1.3.3");
constexpr auto b2 = sc.compare("0.1.1.3");
constexpr auto b3 = b1 + b2;


int main()
{

    using namespace strutil;

    p << typeid(ctstr::test(true)).name();
    p << typeid(ctstr::test(false)).name();
    p << s2;
    p << s3;
    p << s3;

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
    p << format("f=[%.12f]", 50.12345678);
    printf("f=[%.12f]\n", 50.12345678);

    return 0;
}

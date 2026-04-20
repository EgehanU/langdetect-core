#include <cassert>
#include <iostream>
#include "langdetect/utf8.hpp"

int main(){
    // ASCII
    auto r1 = decode("Hello");
    assert(r1.size() == 5);
    assert(r1[0] == 0x48); // H
    assert(r1[1] == 0x65); // e
    assert(r1[2] == 0x6C); // l
    assert(r1[3] == 0x6C); // l
    assert(r1[4] == 0x6F); // o

    // Russian - Привет
    auto r2 = decode("Привет");
    assert(r2.size() == 6);
    assert(r2[0] == 0x041F); // П
    assert(r2[1] == 0x0440); // р
    assert(r2[2] == 0x0438); // и
    assert(r2[3] == 0x0432); // в
    assert(r2[4] == 0x0435); // е
    assert(r2[5] == 0x0442); // т

    // Empty
    auto r3 = decode("");
    assert(r3.size() == 0);

    // Emoji - 4 byte character
    auto r4 = decode("😀");
    assert(r4.size() == 1);
    assert(r4[0] == 0x1F600);

    std::cout << "All tests passed" << std::endl;
    return 0;
}
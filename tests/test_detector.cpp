#include "langdetect/detector.hpp"
#include <cassert>
#include <iostream>

int main() {
    LanguageDetector detector;

    // English
    auto r1 = detector.detect("The quick brown fox jumps over the lazy dog");
    assert(!r1.empty());
    assert(r1[0].label == "en");

    // German
    auto r2 = detector.detect("Die Katze sitzt auf der Mauer und schaut in den Garten");
    assert(!r2.empty());
    assert(r2[0].label == "de");

    // French
    auto r3 = detector.detect("Le chat est assis sur le mur et regarde dans le jardin");
    assert(!r3.empty());
    assert(r3[0].label == "fr");

    // Cyrillic
    auto r4 = detector.detect("Привет мир как дела сегодня");
    assert(!r4.empty());
    assert(r4[0].label == "cyrillic");

    // Arabic
    auto r5 = detector.detect("مرحبا بالعالم كيف حالك اليوم");
    assert(!r5.empty());
    assert(r5[0].label == "arabic");

    // Japanese
    auto r6 = detector.detect("こんにちは世界、今日はどうですか");
    assert(!r6.empty());
    assert(r6[0].label == "japanese");

    // Too short
    auto r7 = detector.detect("hi");
    assert(!r7.empty());
    assert(r7[0].label == "unknown");

    // Empty
    auto r8 = detector.detect("");
    assert(!r8.empty());
    assert(r8[0].label == "unknown");

    std::cout << "All detector tests passed\n";
    return 0;
}
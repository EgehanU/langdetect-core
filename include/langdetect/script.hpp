#ifndef LANGDETECT_SCRIPT_HPP
#define LANGDETECT_SCRIPT_HPP
#include "langdetect/utf8.hpp"
#include "langdetect/result.hpp"
#include <cstdint>

// Broad script buckets used before language-specific scoring
enum class Script{
    Latin,
    Cyrillic,
    Arabic,
    Greek,
    Devanagari,
    Thai,
    Korean,
    Japanese,
    CjkHan,
    Unknown,

};


// Latin letters used by the trained European language profiles
inline bool is_latin(uint32_t cp){
    return
        (cp >= 0x0041 && cp <= 0x005A) || // A-Z
        (cp >= 0x0061 && cp <= 0x007A) || // a-z
        (cp >= 0x00C0 && cp <= 0x00FF) || // Latin-1
        (cp >= 0x0100 && cp <= 0x024F) || // Extended-A/B
        (cp >= 0x1E00 && cp <= 0x1EFF);   // Extended Additional
}

// Cyrillic blocks cover Russian and related scripts
inline bool is_cyrillic(uint32_t cp){
    return
        (cp >= 0x0400 && cp <= 0x04FF) ||
        (cp >= 0x0500 && cp <= 0x052F);
}

// Greek block check
inline bool is_greek(uint32_t cp){
    return
        (cp >= 0x0370 && cp <= 0x03FF);
}

// Arabic blocks used for script-level detection
inline bool is_arabic(uint32_t cp){
    return
        (cp >= 0x0600 && cp <= 0x06FF) ||
        (cp >= 0x0750 && cp <= 0x077F) ||
        (cp >= 0x08A0 && cp <= 0x08FF);
}

// Devanagari block check
inline bool is_devanagari(uint32_t cp){
    return
        (cp >= 0x0900 && cp <= 0x097F);
}

// Thai block check
inline bool is_thai(uint32_t cp){
    return
        (cp >= 0x0E00 && cp <= 0x0E7F);
}

// Hangul syllables and jamo for Korean
inline bool is_korean(uint32_t cp) {
    return
        (cp >= 0xAC00 && cp <= 0xD7AF) || // syllables
        (cp >= 0x1100 && cp <= 0x11FF);   // jamo
}

// Japanese kana checks
inline bool is_hiragana(uint32_t cp){
    return (cp >= 0x3040 && cp <= 0x309F);
}

inline bool is_katakana(uint32_t cp){
    return (cp >= 0x30A0 && cp <= 0x30FF);
}

inline bool is_japanese(uint32_t cp){
    return is_hiragana(cp) || is_katakana(cp);
}

// Shared Han ideographs used for Chinese and Japanese signals
inline bool is_cjk_han(uint32_t cp){
    return
        (cp >= 0x4E00 && cp <= 0x9FFF) ||
        (cp >= 0x3400 && cp <= 0x4DBF);
}

#endif

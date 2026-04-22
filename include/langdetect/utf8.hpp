#ifndef LANGDETECT_UTF8_HPP
#define LANGDETECT_UTF8_HPP

#include <string_view>
#include <vector>
#include <cstdint>

// Decode UTF-8 input into Unicode code points used by the detector
std::vector<uint32_t> decode(std::string_view data);

#endif

#ifndef LANGDETECT_UTF8_HPP
#define LANGDETECT_UTF8_HPP

#include <string_view>
#include <vector>
#include <cstdint>

std::vector<uint32_t> decode(std::string_view data);

#endif
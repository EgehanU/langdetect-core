#ifndef LANGDETECT_DETECTOR_HPP
#define LANGDETECT_DETECTOR_HPP
#include <vector>
#include <string_view>
#include "langdetect/result.hpp"
#include "langdetect/script.hpp"

class LanguageDetector {
public:
    std::vector<DetectionScore> detect(std::string_view text) const;
};



#endif
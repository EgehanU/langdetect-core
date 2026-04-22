#ifndef LANGDETECT_DETECTOR_HPP
#define LANGDETECT_DETECTOR_HPP

#include "langdetect/result.hpp"
#include <cstdint>
#include <cstddef>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>

using Ngram = std::vector<uint32_t>;

// Lets Unicode n-grams work as unordered_map keys
struct NgramHash {
    size_t operator()(const Ngram& gram) const {
        size_t hash = gram.size();
        for (uint32_t cp : gram)
            hash ^= static_cast<size_t>(cp) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
        return hash;
    }
};

using NgramProfile = std::unordered_map<Ngram, double, NgramHash>;

class LanguageDetector {
public:
    // One trained profile loaded from the profiles directory
    struct Profile {
        std::string label;
        NgramProfile ngrams;
    };

    explicit LanguageDetector(const std::string& profiles_dir = "profiles");

    std::vector<DetectionScore> detect(std::string_view text) const;

private:
    std::vector<Profile> profiles_;
};

#endif

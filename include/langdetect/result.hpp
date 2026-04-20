#ifndef LANGDETECT_RESULT_HPP
#define LANGDETECT_RESULT_HPP

#include <string>

enum class ResultKind {
    Language,
    Script,
    Unknown,
    Ambiguous
};

struct DetectionScore {
    std::string label;
    ResultKind kind;
    double score;
};

#endif
#ifndef LANGDETECT_RESULT_HPP
#define LANGDETECT_RESULT_HPP

#include <string>


// Keeps language, script, and fallback results explicit for callers
enum class ResultKind {
    Language,
    Script,
    Unknown,
    Ambiguous
};

// A single ranked detection result
struct DetectionScore {
    std::string label;
    ResultKind kind;
    double score;
};


#endif

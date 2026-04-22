#include "langdetect/detector.hpp"
#include <iostream>
#include <string>
#include <windows.h>

std::string kind_to_string(ResultKind kind){
    // Keep JSON output stable even if the enum grows later
    switch (kind) {
        case ResultKind::Language:  return "language";
        case ResultKind::Script:    return "script";
        case ResultKind::Unknown:   return "unknown";
        case ResultKind::Ambiguous: return "ambiguous";
    }
    return "unknown";
}

int main(int argc, char* argv[]){
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    if (argc < 2){
        std::cout << "Invalid input" << std::endl;
        return 1;
    }

    std::string input;
    // Treat all command arguments after the executable as one input text
    for (int i = 1; i < argc; ++i){
        if (i > 1) input += " ";
        input += argv[i];
    }

    LanguageDetector detector("profiles");
    auto results = detector.detect(input);

    if (results.empty()) {
        std::cout << "{ \"best\": null }\n";
        return 0;
    }

    std::cout << "{\n";
    const auto& best = results[0];
    // Output is written by hand to keep the CLI dependency free
    std::cout << "  \"best\": { ";
    std::cout << "\"kind\": \"" << kind_to_string(best.kind) << "\", ";
    std::cout << "\"label\": \"" << best.label << "\", ";
    std::cout << "\"score\": " << best.score;
    std::cout << " },\n";

    std::cout << "  \"scores\": [\n";
    for (size_t i = 0; i < results.size(); ++i) {
        const auto& r = results[i];
        std::cout << "    { ";
        std::cout << "\"kind\": \"" << kind_to_string(r.kind) << "\", ";
        std::cout << "\"label\": \"" << r.label << "\", ";
        std::cout << "\"score\": " << r.score;
        std::cout << " }";
        if (i != results.size() - 1) std::cout << ",";
        std::cout << "\n";
    }
    std::cout << "  ]\n";
    std::cout << "}\n";

    return 0;
}

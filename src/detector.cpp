#include "langdetect/detector.hpp"
#include "langdetect/utf8.hpp"
#include "langdetect/script.hpp"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <nlohmann/json.hpp>
#include <string_view>
#include <unordered_map>
#include <vector>

LanguageDetector::LanguageDetector(const std::string& profiles_dir) {
    // load the trained Latin profiles that are available at runtime
    for (const std::string& lang : {"en", "de", "fr", "es", "it"}) {
        std::string path = profiles_dir + "/" + lang + ".json";
        std::ifstream file(path);
        if (!file.is_open()) continue;

        nlohmann::json j = nlohmann::json::parse(file);

        Profile p;
        p.label = lang;
        for (auto& [key, value] : j["ngrams"].items()) {
            Ngram gram = decode(key);
            p.ngrams[gram] = value.get<double>();
        }
        profiles_.push_back(std::move(p));
    }
}

namespace {

// Only handle the Latin ranges used by the current profiles
uint32_t lowercase_latin(uint32_t cp) {
    if (cp >= 0x41 && cp <= 0x5A) return cp + 0x20;
    switch (cp) {
        case 0x00C0: return 0x00E0; case 0x00C1: return 0x00E1;
        case 0x00C2: return 0x00E2; case 0x00C3: return 0x00E3;
        case 0x00C4: return 0x00E4; case 0x00C5: return 0x00E5;
        case 0x00C7: return 0x00E7; case 0x00C8: return 0x00E8;
        case 0x00C9: return 0x00E9; case 0x00CA: return 0x00EA;
        case 0x00CB: return 0x00EB; case 0x00CC: return 0x00EC;
        case 0x00CD: return 0x00ED; case 0x00CE: return 0x00EE;
        case 0x00CF: return 0x00EF; case 0x00D1: return 0x00F1;
        case 0x00D2: return 0x00F2; case 0x00D3: return 0x00F3;
        case 0x00D4: return 0x00F4; case 0x00D5: return 0x00F5;
        case 0x00D6: return 0x00F6; case 0x00D9: return 0x00F9;
        case 0x00DA: return 0x00FA; case 0x00DB: return 0x00FB;
        case 0x00DC: return 0x00FC;
        default: return cp;
    }
}

bool is_ascii_separator(uint32_t cp) {
    return cp <= 0x7F && 
           !((cp >= 'a' && cp <= 'z') || 
             (cp >= 'A' && cp <= 'Z') ||
             cp == '\'');  // keep apostrophes
}

// Normalize Latin text into lowercase letters and single word gaps
std::vector<uint32_t> normalize_latin_text(const std::vector<uint32_t>& data) {
    std::vector<uint32_t> normalized;
    bool last_was_space = true;
    for (uint32_t cp : data) {
        cp = lowercase_latin(cp);
        if (is_latin(cp)) {
            normalized.push_back(cp);
            last_was_space = false;
        } else if (is_ascii_separator(cp) && !last_was_space && !normalized.empty()) {
            normalized.push_back(' ');
            last_was_space = true;
        }
    }
    if (!normalized.empty() && normalized.back() == ' ')
        normalized.pop_back();
    return normalized;
}

size_t latin_letter_count(const std::vector<uint32_t>& normalized) {
    size_t count = 0;
    for (uint32_t cp : normalized)
        if (cp != ' ') count++;
    return count;
}

size_t ngram_count(const std::vector<uint32_t>& normalized, size_t n) {
    return normalized.size() >= n ? normalized.size() - n + 1 : 0;
}

NgramProfile build_ngram_profile(const std::vector<uint32_t>& normalized, size_t n) {
    NgramProfile profile;
    const size_t total = ngram_count(normalized, n);
    if (total == 0) return profile;
    for (size_t i = 0; i + n <= normalized.size(); ++i) {
        Ngram gram;
        gram.reserve(n);
        for (size_t j = 0; j < n; ++j)
            gram.push_back(normalized[i + j]);
        profile[gram] += 1.0;
    }
    for (auto& item : profile)
        item.second /= static_cast<double>(total);
    return profile;
}

void add_weighted_ngrams(NgramProfile& dst, const std::vector<uint32_t>& normalized, size_t n, double weight) {
    for (const auto& item : build_ngram_profile(normalized, n))
        dst[item.first] += item.second * weight;
}

NgramProfile build_weighted_profile(const std::vector<uint32_t>& normalized) {
    NgramProfile profile;
    //Longer n-grams carry more language signal for these profiles
    add_weighted_ngrams(profile, normalized, 1, 0.10);
    add_weighted_ngrams(profile, normalized, 2, 0.25);
    add_weighted_ngrams(profile, normalized, 3, 0.65);
    return profile;
}

double cosine_similarity(const NgramProfile& left, const NgramProfile& right) {
    if (left.empty() || right.empty()) return 0.0;
    double dot = 0.0, left_norm = 0.0, right_norm = 0.0;
    for (const auto& item : left) {
        left_norm += item.second * item.second;
        auto match = right.find(item.first);
        if (match != right.end()) dot += item.second * match->second;
    }
    for (const auto& item : right)
        right_norm += item.second * item.second;
    if (left_norm == 0.0 || right_norm == 0.0) return 0.0;
    return dot / (std::sqrt(left_norm) * std::sqrt(right_norm));
}

DetectionScore unknown_result() {
    return {"unknown", ResultKind::Unknown, 1.0};
}

std::vector<DetectionScore> score_latin_languages(
    const std::vector<uint32_t>& normalized,
    const std::vector<LanguageDetector::Profile>& profiles)
{
    NgramProfile input_profile = build_weighted_profile(normalized);
    std::vector<DetectionScore> scores;
    double total_score = 0.0;

    for (const auto& profile : profiles) {
        const double score = cosine_similarity(input_profile, profile.ngrams);
        total_score += score;
        scores.push_back({profile.label, ResultKind::Language, score});
    }

    if (total_score == 0.0) return {};

    for (auto& s : scores) s.score /= total_score;

    std::sort(scores.begin(), scores.end(), [](const DetectionScore& a, const DetectionScore& b) {
        return a.score > b.score;
    });

    return scores;
}

} // namespace

std::vector<DetectionScore> LanguageDetector::detect(std::string_view text) const {
    size_t number_of_latin = 0, number_of_cyrillic = 0, number_of_arabic = 0;
    size_t number_of_greek = 0, number_of_devanagari = 0, number_of_thai = 0;
    size_t number_of_korean = 0, number_of_japanese = 0, number_of_cjk_han = 0;
    size_t number_of_unknown = 0;

    std::vector<uint32_t> data = decode(text);

    // Count script signals first so non-Latin input can return quickly
    for (uint32_t cp : data) {
        if      (is_japanese(cp))   number_of_japanese++;
        else if (is_korean(cp))     number_of_korean++;
        else if (is_cyrillic(cp))   number_of_cyrillic++;
        else if (is_greek(cp))      number_of_greek++;
        else if (is_arabic(cp))     number_of_arabic++;
        else if (is_devanagari(cp)) number_of_devanagari++;
        else if (is_thai(cp))       number_of_thai++;
        else if (is_cjk_han(cp))    number_of_cjk_han++;
        else if (is_latin(cp))      number_of_latin++;
        else                        number_of_unknown++;
    }

    const size_t detected_total =
        number_of_latin + number_of_cyrillic + number_of_arabic +
        number_of_greek + number_of_devanagari + number_of_thai +
        number_of_korean + number_of_japanese + number_of_cjk_han;

    std::vector<DetectionScore> arr;

    if (detected_total == 0) {
        arr.push_back(unknown_result());
        return arr;
    }

    const double latin_signal_pct      = static_cast<double>(number_of_latin)      / detected_total;
    const double cyrillic_signal_pct   = static_cast<double>(number_of_cyrillic)   / detected_total;
    const double arabic_signal_pct     = static_cast<double>(number_of_arabic)     / detected_total;
    const double greek_signal_pct      = static_cast<double>(number_of_greek)      / detected_total;
    const double devanagari_signal_pct = static_cast<double>(number_of_devanagari) / detected_total;
    const double thai_signal_pct       = static_cast<double>(number_of_thai)       / detected_total;
    const double korean_signal_pct     = static_cast<double>(number_of_korean)     / detected_total;
    const double japanese_signal_pct   = static_cast<double>(number_of_japanese + number_of_cjk_han) / detected_total;
    const double cjk_han_signal_pct    = static_cast<double>(number_of_cjk_han)    / detected_total;

    if (latin_signal_pct >= 0.20) {
        std::vector<uint32_t> normalized = normalize_latin_text(data);

        // Very short Latin text is too noisy for n-gram scoring
        if (latin_letter_count(normalized) < 20 || ngram_count(normalized, 3) < 10) {
            arr.push_back(unknown_result());
            return arr;
        }

        std::vector<DetectionScore> language_scores = score_latin_languages(normalized, profiles_);
        if (language_scores.empty()) {
            arr.push_back(unknown_result());
            return arr;
        }

        if (language_scores.size() > 1 &&
            language_scores[0].score - language_scores[1].score < 0.01) {
            // Keep the ranked scores so callers can inspect close matches
            arr.push_back({"ambiguous", ResultKind::Ambiguous, language_scores[0].score});
            arr.insert(arr.end(), language_scores.begin(), language_scores.end());
            return arr;
        }

        return language_scores;
    }

    if      (number_of_japanese > 0 && japanese_signal_pct >= 0.20)
        arr.push_back({"japanese",   ResultKind::Script, japanese_signal_pct});
    else if (korean_signal_pct     >= 0.20)
        arr.push_back({"korean",     ResultKind::Script, korean_signal_pct});
    else if (cyrillic_signal_pct   >= 0.20)
        arr.push_back({"cyrillic",   ResultKind::Script, cyrillic_signal_pct});
    else if (arabic_signal_pct     >= 0.20)
        arr.push_back({"arabic",     ResultKind::Script, arabic_signal_pct});
    else if (greek_signal_pct      >= 0.20)
        arr.push_back({"greek",      ResultKind::Script, greek_signal_pct});
    else if (devanagari_signal_pct >= 0.20)
        arr.push_back({"devanagari", ResultKind::Script, devanagari_signal_pct});
    else if (thai_signal_pct       >= 0.20)
        arr.push_back({"thai",       ResultKind::Script, thai_signal_pct});
    else if (cjk_han_signal_pct    >= 0.20)
        arr.push_back({"cjk_han",    ResultKind::Script, cjk_han_signal_pct});
    else
        arr.push_back({"ambiguous",  ResultKind::Ambiguous, 1.0});

    return arr;
}

#include "langdetect/detector.hpp"
#include "langdetect/utf8.hpp"
#include "langdetect/script.hpp"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace {

using Ngram = std::vector<uint32_t>;

struct NgramHash{
    size_t operator()(const Ngram& gram) const{
        size_t hash = gram.size();
        for (uint32_t cp : gram) {
            hash ^= static_cast<size_t>(cp) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
        }
        return hash;
    }
};

using NgramProfile = std::unordered_map<Ngram, double, NgramHash>;

struct LanguageProfile{
    std::string_view label;
    NgramProfile profile;
};

uint32_t lowercase_latin(uint32_t cp) {
    if (cp >= 0x41 && cp <= 0x5A) {
        return cp + 0x20;
    }

    switch (cp){
        case 0x00C0: return 0x00E0;
        case 0x00C1: return 0x00E1;
        case 0x00C2: return 0x00E2;
        case 0x00C3: return 0x00E3;
        case 0x00C4: return 0x00E4;
        case 0x00C5: return 0x00E5;
        case 0x00C7: return 0x00E7;
        case 0x00C8: return 0x00E8;
        case 0x00C9: return 0x00E9;
        case 0x00CA: return 0x00EA;
        case 0x00CB: return 0x00EB;
        case 0x00CC: return 0x00EC;
        case 0x00CD: return 0x00ED;
        case 0x00CE: return 0x00EE;
        case 0x00CF: return 0x00EF;
        case 0x00D1: return 0x00F1;
        case 0x00D2: return 0x00F2;
        case 0x00D3: return 0x00F3;
        case 0x00D4: return 0x00F4;
        case 0x00D5: return 0x00F5;
        case 0x00D6: return 0x00F6;
        case 0x00D9: return 0x00F9;
        case 0x00DA: return 0x00FA;
        case 0x00DB: return 0x00FB;
        case 0x00DC: return 0x00FC;
        default: return cp;
    }
}

bool is_ascii_separator(uint32_t cp){
    return cp <= 0x7F && !((cp >= 'a' && cp <= 'z') || (cp >= 'A' && cp <= 'Z'));
}

std::vector<uint32_t> normalize_latin_text(const std::vector<uint32_t>& data){
    std::vector<uint32_t> normalized;
    bool last_was_space = true;

    for (uint32_t cp : data) {
        cp = lowercase_latin(cp);

        if (is_latin(cp)) {
            normalized.push_back(cp);
            last_was_space = false;
        } else if (is_ascii_separator(cp) && !last_was_space && !normalized.empty()){
            normalized.push_back(' ');
            last_was_space = true;
        }
    }

    if (!normalized.empty() && normalized.back() == ' '){
        normalized.pop_back();
    }

    return normalized;
}

size_t latin_letter_count(const std::vector<uint32_t>& normalized){
    size_t count = 0;
    for (uint32_t cp : normalized) {
        if (cp != ' ') {
            count++;
        }
    }
    return count;
}

size_t ngram_count(const std::vector<uint32_t>& normalized, size_t n){
    return normalized.size() >= n ? normalized.size() - n + 1 : 0;
}

NgramProfile build_ngram_profile(const std::vector<uint32_t>& normalized, size_t n){
    NgramProfile profile;
    const size_t total = ngram_count(normalized, n);
    if (total == 0) {
        return profile;
    }

    for (size_t i = 0; i + n <= normalized.size(); ++i) {
        Ngram gram;
        gram.reserve(n);
        for (size_t j = 0; j < n; ++j) {
            gram.push_back(normalized[i + j]);
        }
        profile[gram] += 1.0;
    }

    for (auto& item : profile) {
        item.second /= static_cast<double>(total);
    }

    return profile;
}

void add_weighted_ngrams(NgramProfile& destination, const std::vector<uint32_t>& normalized, size_t n, double weight){
    NgramProfile source = build_ngram_profile(normalized, n);
    for (const auto& item : source) {
        destination[item.first] += item.second * weight;
    }
}

NgramProfile build_weighted_profile(const std::vector<uint32_t>& normalized){
    NgramProfile profile;
    add_weighted_ngrams(profile, normalized, 1, 0.10);
    add_weighted_ngrams(profile, normalized, 2, 0.25);
    add_weighted_ngrams(profile, normalized, 3, 0.65);
    return profile;
}

NgramProfile build_seed_profile(std::string_view sample){
    return build_weighted_profile(normalize_latin_text(decode(sample)));
}

const std::vector<LanguageProfile>& language_profiles(){
    static const std::vector<LanguageProfile> profiles = {
        {"en", build_seed_profile("the and that have for not with you this but from they say her she will one all would there their what about which when make can like time just know take people into year your good some could them see other than then now look only come its over think also back after use two how our work first well way even new want because any these give day most us")},
        {"de", build_seed_profile("der die und in den von zu das mit sich des auf fuer ist im dem nicht ein eine als auch es an werden aus er hat dass sie nach wird bei einer um am sind noch wie einem ueber einen so zum war haben nur oder aber vor zur bis mehr durch man sein wurde ich fuer ueber koennen moechte")},
        {"fr", build_seed_profile("le de un etre et a il avoir ne je son que se qui ce dans en du elle au pour pas vous par sur faire plus dire me on mon lui nous comme mais pouvoir avec tout aller voir bien ou sans tu leur homme quand grand etre tres deja francais apres")},
        {"es", build_seed_profile("de la que el en y a los del se las por un para con no una su al lo es como mas pero sus le ya o este si porque esta entre cuando muy sin sobre tambien me hasta hay donde quien desde todo nos durante todos uno les contra ellos este antes algunos")},
        {"it", build_seed_profile("di e che il la a per un in con non una le si i da come lo al del se dei delle ma ha sono nel alla anche essere gli era tra su cui piu questa lui fare dopo quando molto senza tutto noi bene chi quale cosa pero citta cosi")}
    };

    return profiles;
}

double cosine_similarity(const NgramProfile& left, const NgramProfile& right) {
    if (left.empty() || right.empty()){
        return 0.0;
    }

    double dot = 0.0;
    double left_norm = 0.0;
    double right_norm = 0.0;

    for (const auto& item : left){
        left_norm += item.second * item.second;
        auto match = right.find(item.first);
        if (match != right.end()){
            dot += item.second * match->second;
        }
    }

    for (const auto& item : right){
        right_norm += item.second * item.second;
    }

    if (left_norm == 0.0 || right_norm == 0.0){
        return 0.0;
    }

    return dot / (std::sqrt(left_norm) * std::sqrt(right_norm));
}

std::vector<DetectionScore> score_latin_languages(const std::vector<uint32_t>& normalized){
    NgramProfile input_profile = build_weighted_profile(normalized);
    std::vector<DetectionScore> scores;
    double total_score = 0.0;

    for (const LanguageProfile& profile : language_profiles()) {
        const double score = cosine_similarity(input_profile, profile.profile);
        total_score += score;
        scores.push_back({std::string(profile.label), ResultKind::Language, score});
    }

    if (total_score == 0.0){
        return {};
    }

    for (DetectionScore& score : scores){
        score.score /= total_score;
    }

    std::sort(scores.begin(), scores.end(), [](const DetectionScore& left, const DetectionScore& right){
        return left.score > right.score;
    });

    return scores;
}

DetectionScore unknown_result(){
    return {"unknown", ResultKind::Unknown, 1.0};
}

} // namespace

std::vector<DetectionScore> LanguageDetector::detect(std::string_view text) const{
    size_t number_of_latin = 0;
    size_t number_of_cyrillic = 0;
    size_t number_of_arabic = 0;
    size_t number_of_greek = 0;
    size_t number_of_devanagari = 0;
    size_t number_of_thai = 0;
    size_t number_of_korean = 0;
    size_t number_of_japanese = 0;
    size_t number_of_cjk_han = 0;
    size_t number_of_unknown = 0;
    std::vector<uint32_t> data = decode(text);
    for(auto cp = data.begin(); cp != data.end(); ++cp){
        if (is_japanese(*cp))
        number_of_japanese++;
        else if (is_korean(*cp))
            number_of_korean++;
        else if (is_cyrillic(*cp))
            number_of_cyrillic++;
        else if (is_greek(*cp))
            number_of_greek++;
        else if (is_arabic(*cp))
            number_of_arabic++;
        else if (is_devanagari(*cp))
            number_of_devanagari++;
        else if (is_thai(*cp))
            number_of_thai++;
        else if (is_cjk_han(*cp))
            number_of_cjk_han++;
        else if (is_latin(*cp))
            number_of_latin++;
        else
            number_of_unknown++;
    }


    uint32_t total =
    number_of_latin +
    number_of_cyrillic +
    number_of_arabic +
    number_of_greek +
    number_of_devanagari +
    number_of_thai +
    number_of_korean +
    number_of_japanese +
    number_of_cjk_han+
    number_of_unknown;

    double latin_pct = total ? (double)number_of_latin / total : 0.0;
    double cyrillic_pct = total ? (double)number_of_cyrillic / total : 0.0;
    double arabic_pct = total ? (double)number_of_arabic / total : 0.0;
    double greek_pct = total ? (double)number_of_greek / total : 0.0;
    double devanagari_pct = total ? (double)number_of_devanagari / total : 0.0;
    double thai_pct = total ? (double)number_of_thai/ total : 0.0;
    double korean_pct = total ? (double)number_of_korean / total : 0.0;
    double japanese_pct = total ? (double)number_of_japanese / total : 0.0;
    double cjk_han_pct = total ? (double)number_of_cjk_han / total : 0.0;
    double unknown_pct = total ? (double)number_of_unknown / total : 0.0;


    std::vector<DetectionScore> arr;

    const size_t detected_total =
        number_of_latin +
        number_of_cyrillic +
        number_of_arabic +
        number_of_greek +
        number_of_devanagari +
        number_of_thai +
        number_of_korean +
        number_of_japanese +
        number_of_cjk_han;

    if (detected_total == 0) {
        arr.push_back(unknown_result());
        return arr;
    }

    const double latin_signal_pct = static_cast<double>(number_of_latin) / detected_total;
    const double cyrillic_signal_pct = static_cast<double>(number_of_cyrillic) / detected_total;
    const double arabic_signal_pct = static_cast<double>(number_of_arabic) / detected_total;
    const double greek_signal_pct = static_cast<double>(number_of_greek) / detected_total;
    const double devanagari_signal_pct = static_cast<double>(number_of_devanagari) / detected_total;
    const double thai_signal_pct = static_cast<double>(number_of_thai) / detected_total;
    const double korean_signal_pct = static_cast<double>(number_of_korean) / detected_total;
    const double japanese_signal_pct = static_cast<double>(number_of_japanese + number_of_cjk_han) / detected_total;
    const double cjk_han_signal_pct = static_cast<double>(number_of_cjk_han) / detected_total;

    if (latin_signal_pct >= 0.20){
        std::vector<uint32_t> normalized = normalize_latin_text(data);

        if (latin_letter_count(normalized) < 20 || ngram_count(normalized, 3) < 10) {
            arr.push_back(unknown_result());
            return arr;
        }

        std::vector<DetectionScore> language_scores = score_latin_languages(normalized);
        if (language_scores.empty()) {
            arr.push_back(unknown_result());
            return arr;
        }

        if (language_scores.size() > 1 && language_scores[0].score - language_scores[1].score < 0.05) {
            arr.push_back({"ambiguous", ResultKind::Ambiguous, language_scores[0].score});
            arr.insert(arr.end(), language_scores.begin(), language_scores.end());
            return arr;
        }

        return language_scores;
    }

    if (number_of_japanese > 0 && japanese_signal_pct >= 0.70) {
        arr.push_back({"japanese", ResultKind::Script, japanese_signal_pct});
    } else if (korean_signal_pct >= 0.70) {
        arr.push_back({"korean", ResultKind::Script, korean_signal_pct});
    } else if (cyrillic_signal_pct >= 0.70) {
        arr.push_back({"cyrillic", ResultKind::Script, cyrillic_signal_pct});
    } else if (arabic_signal_pct >= 0.70) {
        arr.push_back({"arabic", ResultKind::Script, arabic_signal_pct});
    } else if (greek_signal_pct >= 0.70) {
        arr.push_back({"greek", ResultKind::Script, greek_signal_pct});
    } else if (devanagari_signal_pct >= 0.70) {
        arr.push_back({"devanagari", ResultKind::Script, devanagari_signal_pct});
    } else if (thai_signal_pct >= 0.70) {
        arr.push_back({"thai", ResultKind::Script, thai_signal_pct});
    } else if (cjk_han_signal_pct >= 0.70) {
        arr.push_back({"cjk_han", ResultKind::Script, cjk_han_signal_pct});
    } else {
        arr.push_back({"ambiguous", ResultKind::Ambiguous, 1.0});
    }

    return arr;
}

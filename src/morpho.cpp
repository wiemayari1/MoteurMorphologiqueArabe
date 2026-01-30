#include "morpho.h"
#include <codecvt>
#include <locale>
#include <unordered_set>
#include <algorithm>

using namespace std;

// --- conversions UTF-8 <-> UTF-32 (std::u32string) ---
std::u32string utf8_to_u32(const std::string& s) {
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
    try { return conv.from_bytes(s); }
    catch(...) { return U""; }
}
std::string u32_to_utf8(const std::u32string& s) {
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
    try { return conv.to_bytes(s); }
    catch(...) { return ""; }
}

// --- Normalisation arabe (basique) ---
static const std::unordered_set<char32_t> DIACRITICS = {
    // ranges and individual points (some common ones)
    0x0610,0x0611,0x0612,0x0613,0x0614,0x0615,0x0616,0x0617,0x0618,0x0619,0x061A,
    0x064B,0x064C,0x064D,0x064E,0x064F,0x0650,0x0651,0x0652,0x0653,0x0654,0x0655,0x0656,0x0657,0x0658,0x0659,0x065A,0x065B,0x065C,0x065D,0x065E,0x065F,
    0x0670,
    0x06D6,0x06D7,0x06D8,0x06D9,0x06DA,0x06DB,0x06DC,0x06DF,0x06E0,0x06E1,0x06E2,0x06E3,0x06E4,0x06E5,0x06E6,0x06E7,0x06E8,0x06EA,0x06EB,0x06EC,0x06ED
};

std::u32string normalize_ar(const std::u32string& s) {
    std::u32string out;
    out.reserve(s.size());
    for (char32_t c : s) {
        // remove diacritics
        if (DIACRITICS.count(c)) continue;
        // tatweel
        if (c == 0x0640) continue;
        // ALEF variants -> ا (U+0627)
        if (c == 0x0623 || c == 0x0625 || c == 0x0622 || c == 0x0671) { out.push_back(0x0627); continue; }
        // ى -> ي
        if (c == 0x0649) { out.push_back(0x064A); continue; }
        // optional: map HAMZA VARIANTS? (not done exhaustively)
        out.push_back(c);
    }
    return out;
}

// placeholders codepoints: 'ف' 'ع' 'ل'
static const char32_t PH_F = U'ف'; // U+0641
static const char32_t PH_A = U'ع'; // U+0639
static const char32_t PH_L = U'ل'; // U+0644

std::u32string apply_template(const std::u32string& root, const std::u32string& templ) {
    auto r = normalize_ar(root);
    if (r.size() != 3) throw std::invalid_argument("root must be 3 letters (after normalization)");
    char32_t r1 = r[0], r2 = r[1], r3 = r[2];
    std::u32string out;
    out.reserve(templ.size() + 3);
    for (char32_t ch : templ) {
        if (ch == PH_F) out.push_back(r1);
        else if (ch == PH_A) out.push_back(r2);
        else if (ch == PH_L) out.push_back(r3);
        else out.push_back(ch);
    }
    return out;
}

std::optional<std::u32string> extract_root_from_word(const std::u32string& word, const std::u32string& templ) {
    auto w = normalize_ar(word);
    size_t i = 0;
    std::u32string letters;
    letters.reserve(3);
    for (char32_t ch : templ) {
        if (ch == PH_F || ch == PH_A || ch == PH_L) {
            if (i >= w.size()) return std::nullopt;
            letters.push_back(w[i]);
            ++i;
        } else {
            if (i >= w.size()) return std::nullopt;
            if (w[i] != ch) return std::nullopt;
            ++i;
        }
    }
    if (i != w.size()) return std::nullopt;
    if (letters.size() != 3) return std::nullopt;
    return letters;
}

std::vector<std::u32string> find_schemes_matching(const std::u32string& word, const std::vector<std::u32string>& scheme_templates) {
    std::vector<std::u32string> out;
    for (const auto &templ : scheme_templates) {
        auto r = extract_root_from_word(word, templ);
        if (r) out.push_back(templ);
    }
    return out;
}

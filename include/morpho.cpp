#include "morpho.h"
#include <codecvt>
#include <locale>
#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <random>
#include <limits>

// Vérification des caractères arabes (U+0600 à U+06FF et U+0750 à U+077F)
bool isValidArabic(const std::u32string& s) {
    for (char32_t c : s) {
        // Lettres arabes de base
        bool isArabic = (c >= 0x0600 && c <= 0x06FF) ||
                        // Lettres arabes supplémentaires
                        (c >= 0x0750 && c <= 0x077F) ||
                        // Hamza et variantes
                        c == 0x0621 || c == 0x0622 || c == 0x0623 || 
                        c == 0x0625 || c == 0x0626 ||
                        // Tatweel (kashida) - optionnel
                        c == 0x0640;
        
        // Ignorer les espaces
        if (c == U' ' || c == U'\t' || c == U'\n' || c == U'\r') continue;
        
        if (!isArabic) return false;
    }
    return !s.empty();
}

// Vérification spécifique pour racine trilitère
bool isValidArabicRoot(const std::u32string& s) {
    if (s.length() != 3) return false;
    return isValidArabic(s);
}

// Conversion UTF-8 -> UTF-32
std::u32string utf8_to_u32(const std::string& s) {
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
    return conv.from_bytes(s);
}

// Conversion UTF-32 -> UTF-8
std::string u32_to_utf8(const std::u32string& s) {
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
    return conv.to_bytes(s);
}

// Normalisation très simple
std::u32string normalize_ar(const std::u32string& in) {
    std::u32string out;
    for (char32_t c : in) {
        // Normaliser les alifs
        if (c == U'أ' || c == U'إ' || c == U'آ') c = U'ا';
        // Supprimer les diacritiques (tashkeel)
        if (c == U'َ' || c == U'ً' || c == U'ُ' || c == U'ٌ' ||
            c == U'ِ' || c == U'ٍ' || c == U'ْ' || c == U'ّ') {
            continue;
        }
        out.push_back(c);
    }
    return out;
}

std::u32string apply_template(const std::u32string& root,
                              const std::u32string& templ) {
    if (root.size() != 3) throw std::invalid_argument("Racine doit être trilittère");
    std::u32string out;
    for (char32_t c : templ) {
        if (c == U'ف') out.push_back(root[0]);
        else if (c == U'ع') out.push_back(root[1]);
        else if (c == U'ل') out.push_back(root[2]);
        else out.push_back(c);
    }
    return out;
}

std::optional<std::u32string> extract_root_from_word(const std::u32string& word,
                                                     const std::u32string& templ) {
    if (word.size() != templ.size()) return std::nullopt;
    std::u32string r(3, U'?');
    for (std::size_t i = 0; i < word.size(); ++i) {
        char32_t tc = templ[i];
        if (tc == U'ف') r[0] = word[i];
        else if (tc == U'ع') r[1] = word[i];
        else if (tc == U'ل') r[2] = word[i];
        else if (word[i] != tc) {
            return std::nullopt;
        }
    }
    if (r[0] == U'?' || r[1] == U'?' || r[2] == U'?') return std::nullopt;
    return r;
}

std::vector<std::u32string> find_schemes_matching(
    const std::u32string& word,
    const std::vector<std::u32string>& templates) {
    std::vector<std::u32string> res;
    for (auto& t : templates) {
        auto r = extract_root_from_word(word, t);
        if (r) res.push_back(t);
    }
    return res;
}

// Génération d'une question de jeu aléatoire
GameQuestion generate_game_question(
    const std::vector<std::u32string>& roots,
    const std::vector<std::u32string>& scheme_names,
    const std::vector<std::u32string>& scheme_templates)
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    
    GameQuestion q;
    q.id = static_cast<int>(gen());
    
    // Choisir un type de question aléatoire
    std::uniform_int_distribution<> type_dist(1, 3);
    int qtype = type_dist(gen);
    
    // Choisir une racine et un schème aléatoires
    std::uniform_int_distribution<> root_dist(0, roots.size() - 1);
    std::uniform_int_distribution<> scheme_dist(0, scheme_templates.size() - 1);
    
    int r_idx = root_dist(gen);
    int s_idx = scheme_dist(gen);
    
    q.root = roots[r_idx];
    q.scheme_name = scheme_names[s_idx];
    q.scheme_template = scheme_templates[s_idx];
    
    try {
        q.word = apply_template(q.root, q.scheme_template);
    } catch (...) {
        // Fallback simple
        q.word = q.root;
    }
    
    // Générer des options (toujours 4 options)
    std::uniform_int_distribution<> diff_dist(0, 2);
    int diff = diff_dist(gen);
    q.difficulty = (diff == 0) ? "easy" : (diff == 1) ? "medium" : "hard";
    
    if (qtype == 1) {
        // Type 1: Donner la racine (word + scheme donnés)
        q.type = "find_root";
        q.correct_answer = q.root;
        
        // Générer 3 mauvaises réponses
        std::set<std::u32string> used = {q.root};
        while (used.size() < 4) {
            int wrong_idx = root_dist(gen);
            if (wrong_idx != r_idx) {
                used.insert(roots[wrong_idx]);
            }
        }
        q.options.assign(used.begin(), used.end());
        
    } else if (qtype == 2) {
        // Type 2: Donner le schème (word + root donnés)
        q.type = "find_scheme";
        q.correct_answer = q.scheme_name;
        
        // Générer 3 mauvais schèmes
        std::set<std::u32string> used = {q.scheme_name};
        while (used.size() < 4) {
            int wrong_idx = scheme_dist(gen);
            if (wrong_idx != s_idx) {
                used.insert(scheme_names[wrong_idx]);
            }
        }
        q.options.assign(used.begin(), used.end());
        
    } else {
        // Type 3: Oui/Non - Le mot appartient-il à la racine?
        q.type = "validate_word";
        
        // 50% de chance que ce soit vrai ou faux
        std::uniform_int_distribution<> bool_dist(0, 1);
        bool is_valid = bool_dist(gen) == 1;
        
        if (!is_valid) {
            // Choisir une autre racine
            std::u32string other_root;
            do {
                other_root = roots[root_dist(gen)];
            } while (other_root == q.root);
            
            try {
                q.word = apply_template(other_root, q.scheme_template);
            } catch (...) {
                // Si échec, garder le mot valide
            }
        }
        
        q.correct_answer = is_valid ? U"نعم" : U"لا";
        q.options = {U"نعم", U"لا"};
    }
    
    // Mélanger les options
    std::shuffle(q.options.begin(), q.options.end(), gen);
    
    return q;
}

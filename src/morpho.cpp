#include "morpho.h"
#include "unicode_utils.h"
#include <algorithm>
#include <cctype>
#include <random>
#include <set>
// Vérification si un caractère est arabe
static bool isArabicChar(char32_t c) {
    return (c >= 0x0600 && c <= 0x06FF) || 
           (c >= 0x0750 && c <= 0x077F) ||
           (c >= 0x08A0 && c <= 0x08FF) ||
           (c >= 0xFB50 && c <= 0xFDFF) ||
           (c >= 0xFE70 && c <= 0xFEFF);
}

bool isValidArabic(const std::u32string& s) {
    if (s.empty()) return false;
    for (char32_t c : s) {
        if (!isArabicChar(c)) return false;
    }
    return true;
}

bool isValidArabicRoot(const std::u32string& s) {
    return s.length() == 3 && isValidArabic(s);
}


// Normalisation simplifiée de l'arabe
std::u32string normalize_ar(const std::u32string& in) {
    std::u32string out;
    for (char32_t c : in) {
        // Supprimer les signes diacritiques (tashkil)
        if (c >= 0x064B && c <= 0x065F) continue;
        if (c == 0x0640) continue; // Tatweel
        
        // Normaliser les formes d'Alif
        if (c == 0x0622 || c == 0x0623 || c == 0x0625) {
            c = 0x0627; // Alif
        }
        
        out.push_back(c);
    }
    return out;
}

// Appliquer un template à une racine
std::u32string apply_template(const std::u32string& root, const std::u32string& templ) {
    if (root.size() != 3) return U"";
    
    std::u32string result;
    for (char32_t c : templ) {
        if (c == U'ف') {
            result.push_back(root[0]);
        } else if (c == U'ع') {
            result.push_back(root[1]);
        } else if (c == U'ل') {
            result.push_back(root[2]);
        } else {
            result.push_back(c);
        }
    }
    return result;
}

// CORRECTION: Extraire la racine d'un mot selon un template
std::optional<std::u32string> extract_root_from_word(const std::u32string& word, 
                                                      const std::u32string& templ) {
    if (word.size() != templ.size()) return std::nullopt;
    
    std::u32string root = U"???";
    bool has_f = false, has_e = false, has_l = false;
    
    for (size_t i = 0; i < templ.size(); i++) {
        char32_t t = templ[i];
        char32_t w = word[i];
        
        if (t == U'ف') {
            if (!has_f) {
                root[0] = w;
                has_f = true;
            } else if (root[0] != w) {
                return std::nullopt; // Incohérence
            }
        } else if (t == U'ع') {
            if (!has_e) {
                root[1] = w;
                has_e = true;
            } else if (root[1] != w) {
                return std::nullopt;
            }
        } else if (t == U'ل') {
            if (!has_l) {
                root[2] = w;
                has_l = true;
            } else if (root[2] != w) {
                return std::nullopt;
            }
        } else {
            // Caractère fixe, doit correspondre exactement
            if (t != w) return std::nullopt;
        }
    }
    
    // Vérifier qu'on a trouvé les 3 lettres de la racine
    if (!has_f || !has_e || !has_l) return std::nullopt;
    
    return root;
}

// Trouver les templates qui matchent un mot
std::vector<std::u32string> find_schemes_matching(const std::u32string& word,
                                                  const std::vector<std::u32string>& templates) {
    std::vector<std::u32string> matches;
    for (const auto& t : templates) {
        auto root = extract_root_from_word(word, t);
        if (root) matches.push_back(t);
    }
    return matches;
}

// CORRECTION: Génération de questions de jeu
GameQuestion generate_game_question(const std::vector<std::u32string>& roots,
                                    const std::vector<std::u32string>& scheme_names,
                                    const std::vector<std::u32string>& scheme_templates) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    
    GameQuestion q;
    q.id = 0;
    
    // Choisir un type de question aléatoire
    std::uniform_int_distribution<> type_dist(0, 2);
    int type = type_dist(gen);
    
    // Choisir une racine et un schème aléatoires
    std::uniform_int_distribution<> root_dist(0, roots.size() - 1);
    std::uniform_int_distribution<> scheme_dist(0, scheme_templates.size() - 1);
    
    int r_idx = root_dist(gen);
    int s_idx = scheme_dist(gen);
    
    q.root = roots[r_idx];
    q.scheme_name = scheme_names[s_idx];
    q.scheme_template = scheme_templates[s_idx];
    
    // Générer le mot dérivé
    q.word = apply_template(q.root, q.scheme_template);
    
    // Difficulté
    std::uniform_int_distribution<> diff_dist(0, 2);
    int diff = diff_dist(gen);
    q.difficulty = (diff == 0) ? "easy" : (diff == 1) ? "medium" : "hard";
    
    switch (type) {
        case 0: { // Trouver la racine
            q.type = "find_root";
            q.correct_answer = q.root;
            
            // Générer 3 mauvaises réponses (autres racines)
            std::set<std::u32string> used = {q.root};
            while (q.options.size() < 3) {
                int idx = root_dist(gen);
                if (used.insert(roots[idx]).second) {
                    q.options.push_back(roots[idx]);
                }
            }
            q.options.push_back(q.root);
            break;
        }
        case 1: { // Trouver le schème
            q.type = "find_scheme";
            q.correct_answer = q.scheme_name;
            
            // Générer 3 mauvaises réponses (autres schèmes)
            std::set<std::u32string> used = {q.scheme_name};
            while (q.options.size() < 3) {
                int idx = scheme_dist(gen);
                if (used.insert(scheme_names[idx]).second) {
                    q.options.push_back(scheme_names[idx]);
                }
            }
            q.options.push_back(q.scheme_name);
            break;
        }
        case 2: { // Valider si le mot appartient à la racine
            q.type = "validate_word";
            
            // 50% de chance que ce soit vrai ou faux
            std::uniform_int_distribution<> bool_dist(0, 1);
            bool is_valid = bool_dist(gen);
            
            if (is_valid) {
                q.correct_answer = U"نعم";
                // Le mot généré est déjà valide
            } else {
                q.correct_answer = U"لا";
                // Choisir une autre racine pour le mot
                std::u32string wrong_root;
                do {
                    wrong_root = roots[root_dist(gen)];
                } while (wrong_root == q.root);
                
                q.word = apply_template(wrong_root, q.scheme_template);
            }
            
            q.options = {U"نعم", U"لا"};
            break;
        }
    }
    
    // Mélanger les options
    std::shuffle(q.options.begin(), q.options.end(), gen);
    
    return q;
}

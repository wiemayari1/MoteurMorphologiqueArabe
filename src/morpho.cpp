#include "morpho.h"
#include <codecvt>
#include <locale>
#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <random>
#include <limits>

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
        if (c == U'أ' || c == U'إ' || c == U'آ') c = U'ا';
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

// Mini-jeu mixte (réponse libre + O/N)
void play_minigame(const std::vector<std::u32string>& roots,
                   const std::vector<std::u32string>& scheme_names,
                   const std::vector<std::u32string>& scheme_templates)
{
    if (roots.empty() || scheme_names.empty() || scheme_templates.empty()) {
        std::cout << "Pas assez de données pour le mini-jeu.\n";
        return;
    }

    std::mt19937 rng(static_cast<unsigned>(std::time(nullptr)));
    std::uniform_int_distribution<int> type_dist(1, 3);
    auto rand_index = [&](int max) {
        std::uniform_int_distribution<int> d(0, max - 1);
        return d(rng);
    };

    const int total_q = 6;
    int score = 0;

    std::cout << "\n===== MINI-JEU MORPHOLOGIQUE =====\n";
    std::cout << "Types de questions :\n"
              << " - Trouver la racine (réponse libre)\n"
              << " - Trouver le schème (réponse libre)\n"
              << " - Dire si un mot appartient à une racine (o/n)\n\n";

    for (int q = 1; q <= total_q; ++q) {
        int qtype = type_dist(rng);

        std::u32string root = roots[rand_index((int)roots.size())];
        int s_idx = rand_index((int)scheme_templates.size());
        std::u32string s_name  = scheme_names[s_idx];
        std::u32string s_templ = scheme_templates[s_idx];

        std::u32string word;
        try {
            word = apply_template(root, s_templ);
        } catch (...) {
            --q;
            continue;
        }

        std::cout << "------------------------------\n";
        std::cout << "Question " << q << " / " << total_q << "\n";

        if (qtype == 1) {
            std::cout << "[Type A] Donnez la racine du mot : "
                      << u32_to_utf8(word) << "\n";
            std::cout << "Votre réponse (en arabe) : ";

            std::string rep_utf8;
            std::getline(std::cin, rep_utf8);
            if (rep_utf8.empty()) {
                std::getline(std::cin, rep_utf8); // si le premier getline lit juste le \n
            }

            auto rep = normalize_ar(utf8_to_u32(rep_utf8));
            auto r_norm = normalize_ar(root);

            if (rep == r_norm) {
                std::cout << "Bonne réponse !\n";
                score++;
            } else {
                std::cout << "Mauvaise réponse. La racine correcte était : "
                          << u32_to_utf8(root) << "\n";
            }
        } else if (qtype == 2) {
            std::cout << "[Type B] Donnez le schème utilisé pour le mot : "
                      << u32_to_utf8(word) << "\n";
            std::cout << "(par ex. مفعول, فاعل, ...)\n";
            std::cout << "Votre réponse : ";

            std::string rep_utf8;
            std::getline(std::cin, rep_utf8);
            if (rep_utf8.empty()) {
                std::getline(std::cin, rep_utf8);
            }

            auto rep = normalize_ar(utf8_to_u32(rep_utf8));
            auto name_norm = normalize_ar(s_name);

            if (rep == name_norm) {
                std::cout << "Bonne réponse !\n";
                score++;
            } else {
                std::cout << "Mauvaise réponse. Le schème correct était : "
                          << u32_to_utf8(s_name) << "\n";
            }
        } else {
            bool cas_vrai = (rand_index(2) == 0);

            std::u32string racine_question = root;
            std::u32string mot_question    = word;

            if (!cas_vrai) {
                std::u32string other_root;
                do {
                    other_root = roots[rand_index((int)roots.size())];
                } while (other_root == root);

                try {
                    mot_question = apply_template(other_root, s_templ);
                    racine_question = root;
                } catch (...) {
                    cas_vrai = true;
                    mot_question = word;
                    racine_question = root;
                }
            }

            std::cout << "[Type C] Est-ce que le mot "
                      << u32_to_utf8(mot_question)
                      << " appartient morphologiquement à la racine "
                      << u32_to_utf8(racine_question) << " ? (o/n) : ";

            char rep_ch;
            std::cin >> rep_ch;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            bool rep_oui = (rep_ch == 'o' || rep_ch == 'O' ||
                            rep_ch == 'y' || rep_ch == 'Y');

            bool vrai = false;
            auto maybe_root = extract_root_from_word(mot_question, s_templ);
            if (maybe_root) {
                auto rn = normalize_ar(*maybe_root);
                auto rq = normalize_ar(racine_question);
                vrai = (rn == rq);
            }

            if (rep_oui == vrai) {
                std::cout << "Bonne réponse !\n";
                score++;
            } else {
                std::cout << "Mauvaise réponse. ";
                if (vrai) {
                    std::cout << "En réalité, OUI, ce mot vient bien de cette racine.\n";
                } else {
                    std::cout << "En réalité, NON, ce mot ne vient pas de cette racine.\n";
                }
            }
        }

        std::cout << "\n";
    }

    std::cout << "===== FIN DU MINI-JEU =====\n";
    int percent = (score * 100) / total_q;
    std::cout << "Score final : " << score << " / " << total_q
              << " (" << percent << "%)\n\n";
}
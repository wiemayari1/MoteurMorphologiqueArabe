#include "morpho.h"
#include "unicode_utils.h"
#include <ctime>
#include <iostream>
#include <limits>


// ============================================================================
// GÉNÉRATEUR ALÉATOIRE MANUEL 
// ============================================================================
static uint64_t lcg_state = 123456789;

static void lcg_seed(uint64_t seed) { lcg_state = seed; }

static uint64_t lcg_rand() {
  lcg_state = (1103515245 * lcg_state + 12345) & 0x7fffffff;
  return lcg_state;
}

// ============================================================================
// FISHER-YATES MANUEL
// ============================================================================
template <typename T> static void fisher_yates_shuffle(std::vector<T> &vec) {
  if (vec.size() <= 1)
    return;

  static bool seeded = false;
  if (!seeded) {
    lcg_seed(static_cast<uint64_t>(time(nullptr)));
    seeded = true;
  }

  for (size_t i = vec.size() - 1; i > 0; --i) {
    uint64_t j = lcg_rand() % (i + 1);
    T temp = vec[i];
    vec[i] = vec[j];
    vec[j] = temp;
  }
}

// ============================================================================
// Validation des caractères arabes
// ============================================================================
static bool isArabicChar(char32_t c) {
  return (c >= 0x0600 && c <= 0x06FF) || (c >= 0x0750 && c <= 0x077F) ||
         (c >= 0x08A0 && c <= 0x08FF) || (c >= 0xFB50 && c <= 0xFDFF) ||
         (c >= 0xFE70 && c <= 0xFEFF);
}

bool isValidArabic(const std::vector<char32_t> &s) {
  if (s.empty())
    return false;
  for (char32_t c : s) {
    if (!isArabicChar(c))
      return false;
  }
  return true;
}

bool isValidArabicRoot(const std::vector<char32_t> &s) {
  return s.size() == 3 && isValidArabic(s);
}

// ============================================================================
// Normalisation arabe
// ============================================================================
std::vector<char32_t> normalize_ar(const std::vector<char32_t> &in) {
  std::vector<char32_t> out;
  out.reserve(in.size());

  for (char32_t c : in) {
    if (c >= 0x064B && c <= 0x065F)
      continue; // Tashkil
    if (c == 0x0640)
      continue; // Tatweel
    if (c == 0x0622 || c == 0x0623 || c == 0x0625)
      c = 0x0627; // Alifs
    out.push_back(c);
  }
  return out;
}

// ============================================================================
// Génération morphologique
// ============================================================================
std::vector<char32_t> apply_template(const std::vector<char32_t> &root,
                                     const std::vector<char32_t> &templ) {
  if (root.size() != 3)
    return {};

  std::vector<char32_t> result;
  result.reserve(templ.size());

  for (char32_t c : templ) {
    if (c == U'ف')
      result.push_back(root[0]);
    else if (c == U'ع')
      result.push_back(root[1]);
    else if (c == U'ل')
      result.push_back(root[2]);
    else
      result.push_back(c);
  }
  return result;
}

std::vector<char32_t>
generate_from_scheme(const std::vector<char32_t> &root,
                     const std::vector<char32_t> &scheme_name, HashTable &ht) {
  SchemeEntry *se = ht.get(scheme_name);
  if (!se)
    return {};
  return apply_template(root, se->templ);
}

// ============================================================================
// Validation morphologique
// ============================================================================
Optional<std::vector<char32_t>>
extract_root_from_word(const std::vector<char32_t> &word,
                       const std::vector<char32_t> &templ) {
  if (word.size() != templ.size())
    return Optional<std::vector<char32_t>>();

  std::vector<char32_t> root(3);
  bool has_f = false, has_e = false, has_l = false;

  for (size_t i = 0; i < templ.size(); i++) {
    char32_t t = templ[i];
    char32_t w = word[i];

    if (t == U'ف') {
      if (!has_f) {
        root[0] = w;
        has_f = true;
      } else if (root[0] != w) {
        return Optional<std::vector<char32_t>>();
      }
    } else if (t == U'ع') {
      if (!has_e) {
        root[1] = w;
        has_e = true;
      } else if (root[1] != w) {
        return Optional<std::vector<char32_t>>();
      }
    } else if (t == U'ل') {
      if (!has_l) {
        root[2] = w;
        has_l = true;
      } else if (root[2] != w) {
        return Optional<std::vector<char32_t>>();
      }
    } else {
      if (t != w)
        return Optional<std::vector<char32_t>>();
    }
  }

  if (!has_f || !has_e || !has_l)
    return Optional<std::vector<char32_t>>();
  return Optional<std::vector<char32_t>>(root);
}

ValidationResult validate_word(const std::vector<char32_t> &word,
                               const std::vector<char32_t> &root,
                               HashTable &ht) {
  for (const auto &scheme : ht.allSchemes()) {
    auto maybe_root = extract_root_from_word(word, scheme.templ);
    if (maybe_root) {
      auto rn = normalize_ar(*maybe_root);
      if (rn == root) {
        return {true, scheme.name};
      }
    }
  }
  return {false, {}};
}

// ============================================================================
// Utilitaires
// ============================================================================
std::vector<std::vector<char32_t>>
find_schemes_matching(const std::vector<char32_t> &word,
                      const std::vector<std::vector<char32_t>> &templates) {
  std::vector<std::vector<char32_t>> matches;
  for (const auto &t : templates) {
    if (extract_root_from_word(word, t)) {
      matches.push_back(t);
    }
  }
  return matches;
}

// ============================================================================
// Mini-jeu morphologique
// ============================================================================
void play_minigame(const std::vector<std::vector<char32_t>> &roots,
                   const std::vector<std::vector<char32_t>> &scheme_names,
                   const std::vector<std::vector<char32_t>> &scheme_templates) {
  if (roots.size() < 2 || scheme_templates.empty()) {
    std::cout << "Données insuffisantes pour le jeu\\n";
    return;
  }

  const int NB_QUESTIONS = 5;
  int score = 0;

  std::cout << "=== Mini-jeu morphologique ===\\n";
  std::cout << "Répondez à " << NB_QUESTIONS << " questions.\\n\\n";

  for (int q = 1; q <= NB_QUESTIONS; q++) {
    size_t r_idx = lcg_rand() % roots.size();
    size_t s_idx = lcg_rand() % scheme_templates.size();

    const auto &root = roots[r_idx];
    const auto &sname = scheme_names[s_idx];
    const auto &stempl = scheme_templates[s_idx];

    auto word = apply_template(root, stempl);
    int type = lcg_rand() % 2;

    std::cout << "Question " << q << "/" << NB_QUESTIONS << "\\n";

    if (type == 0) {
      std::cout << "Mot: " << unicode::u32_to_utf8(word) << "\\n";
      std::cout << "Schème: " << unicode::u32_to_utf8(sname) << "\\n";
      std::cout << "Quelle est la racine? ";

      std::string answer;
      std::getline(std::cin, answer);
      auto ans_u32 = normalize_ar(unicode::utf8_to_u32(answer));

      if (ans_u32 == normalize_ar(root)) {
        std::cout << "Correct!\\n";
        score++;
      } else {
        std::cout << "Incorrect. Réponse: " << unicode::u32_to_utf8(root)
                  << "\\n";
      }
    } else {
      std::cout << "Mot: " << unicode::u32_to_utf8(word) << "\\n";
      std::cout << "Racine: " << unicode::u32_to_utf8(root) << "\\n";
      std::cout << "Quel est le schème? ";

      std::string answer;
      std::getline(std::cin, answer);
      auto ans_u32 = normalize_ar(unicode::utf8_to_u32(answer));

      if (ans_u32 == normalize_ar(sname)) {
        std::cout << "Correct!\\n";
        score++;
      } else {
        std::cout << "Incorrect. Réponse: " << unicode::u32_to_utf8(sname)
                  << "\\n";
      }
    }
    std::cout << "\\n";
  }

  std::cout << "=== Résultat: " << score << "/" << NB_QUESTIONS << " ===\\n";
}

// ============================================================================
// Génération de questions pour le jeu API
// ============================================================================
GameQuestion generate_game_question(
    const std::vector<std::vector<char32_t>> &roots,
    const std::vector<std::vector<char32_t>> &scheme_names,
    const std::vector<std::vector<char32_t>> &scheme_templates) {
  GameQuestion q;
  q.id = 0;

  size_t r_idx = lcg_rand() % roots.size();
  size_t s_idx = lcg_rand() % scheme_templates.size();

  q.root = roots[r_idx];
  q.scheme_name = scheme_names[s_idx];
  q.scheme_template = scheme_templates[s_idx];
  q.word = apply_template(q.root, q.scheme_template);

  int diff = lcg_rand() % 3;
  q.difficulty = (diff == 0) ? "easy" : (diff == 1) ? "medium" : "hard";

  int type = lcg_rand() % 3;

  if (type == 0) {
    q.type = "find_root";
    q.correct_answer = q.root;

    std::vector<bool> used(roots.size(), false);
    used[r_idx] = true;

    while (q.options.size() < 3) {
      size_t idx = lcg_rand() % roots.size();
      if (!used[idx]) {
        used[idx] = true;
        q.options.push_back(roots[idx]);
      }
    }
    q.options.push_back(q.root);

  } else if (type == 1) {
    q.type = "find_scheme";
    q.correct_answer = q.scheme_name;

    std::vector<bool> used(scheme_names.size(), false);
    used[s_idx] = true;

    while (q.options.size() < 3) {
      size_t idx = lcg_rand() % scheme_names.size();
      if (!used[idx]) {
        used[idx] = true;
        q.options.push_back(scheme_names[idx]);
      }
    }
    q.options.push_back(q.scheme_name);

  } else {
    q.type = "validate_word";

    bool is_valid = (lcg_rand() % 2) == 1;

    if (!is_valid && roots.size() > 1) {
      std::vector<char32_t> wrong_root;
      do {
        wrong_root = roots[lcg_rand() % roots.size()];
      } while (wrong_root == q.root);
      q.word = apply_template(wrong_root, q.scheme_template);
    }

    q.correct_answer = is_valid ? std::vector<char32_t>{U'ن', U'ع', U'م'}
                                :                             // "نعم"
                           std::vector<char32_t>{U'ل', U'ا'}; // "لا"
    q.options = {std::vector<char32_t>{U'ن', U'ع', U'م'},
                 std::vector<char32_t>{U'ل', U'ا'}};
  }

  fisher_yates_shuffle(q.options);
  return q;
}

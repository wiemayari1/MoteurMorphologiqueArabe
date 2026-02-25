#ifndef MORPHO_H
#define MORPHO_H

#include "hash_table.h"
#include "unicode_utils.h"
#include <string>
#include <vector>

// ============================================================================
// OPTIONAL MANUEL 
// ============================================================================
template <typename T> struct Optional {
  bool has_value;
  T value;

  Optional() : has_value(false) {}
  Optional(const T &v) : has_value(true), value(v) {}

  bool hasValue() const { return has_value; }
  T &getValue() { return value; }
  const T &getValue() const { return value; }

  operator bool() const { return has_value; }
  T &operator*() { return value; }
  const T &operator*() const { return value; }
};

// ============================================================================
// Validation des caractères arabes
// ============================================================================
bool isValidArabic(const std::vector<char32_t> &s);
bool isValidArabicRoot(const std::vector<char32_t> &s);

// ============================================================================
// Normalisation (suppression diacritiques, unification alifs)
// ============================================================================
std::vector<char32_t> normalize_ar(const std::vector<char32_t> &in);

// ============================================================================
// Génération morphologique
// Convention : ف = 1ère lettre, ع = 2ème, ل = 3ème
// ============================================================================
std::vector<char32_t> apply_template(const std::vector<char32_t> &root,
                                     const std::vector<char32_t> &templ);

std::vector<char32_t>
generate_from_scheme(const std::vector<char32_t> &root,
                     const std::vector<char32_t> &scheme_name, HashTable &ht);

// ============================================================================
// Validation morphologique
// ============================================================================
Optional<std::vector<char32_t>>
extract_root_from_word(const std::vector<char32_t> &word,
                       const std::vector<char32_t> &templ);

struct ValidationResult {
  bool valid;
  std::vector<char32_t> scheme;
};

ValidationResult validate_word(const std::vector<char32_t> &word,
                               const std::vector<char32_t> &root,
                               HashTable &ht);

// ============================================================================
// Utilitaires
// ============================================================================
std::vector<std::vector<char32_t>>
find_schemes_matching(const std::vector<char32_t> &word,
                      const std::vector<std::vector<char32_t>> &templates);

// ============================================================================
// Mini-jeu morphologique
// ============================================================================
void play_minigame(const std::vector<std::vector<char32_t>> &roots,
                   const std::vector<std::vector<char32_t>> &scheme_names,
                   const std::vector<std::vector<char32_t>> &scheme_templates);

// ============================================================================
// Structure pour le jeu API
// ============================================================================
struct GameQuestion {
  int id;
  std::string type;
  std::vector<char32_t> word;
  std::vector<char32_t> root;
  std::vector<char32_t> scheme_name;
  std::vector<char32_t> scheme_template;
  std::vector<std::vector<char32_t>> options;
  std::vector<char32_t> correct_answer;
  std::string difficulty;
};

GameQuestion generate_game_question(
    const std::vector<std::vector<char32_t>> &roots,
    const std::vector<std::vector<char32_t>> &scheme_names,
    const std::vector<std::vector<char32_t>> &scheme_templates);

#endif

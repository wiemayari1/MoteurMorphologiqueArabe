#ifndef MORPHO_H
#define MORPHO_H

#include <string>
#include <vector>
#include <optional>

// Conversion UTF-8 <-> UTF-32
std::u32string utf8_to_u32(const std::string& s);
std::string u32_to_utf8(const std::u32string& s);

// Normalisation de texte arabe (simplifiée)
std::u32string normalize_ar(const std::u32string& in);

// Appliquer un schème (template) à une racine
// Convention : le template contient ف / ع / ل comme slots de la racine
std::u32string apply_template(const std::u32string& root,
                              const std::u32string& templ);

// Extraire une racine à partir d'un mot et d'un schème
// Retourne std::nullopt si impossible.
std::optional<std::u32string> extract_root_from_word(const std::u32string& word,
                                                     const std::u32string& templ);

// À partir d'un mot et d'une liste de templates, retourne ceux qui matchent
std::vector<std::u32string> find_schemes_matching(
    const std::u32string& word,
    const std::vector<std::u32string>& templates);

// Mini-jeu : version à 3 paramètres
void play_minigame(const std::vector<std::u32string>& roots,
                   const std::vector<std::u32string>& scheme_names,
                   const std::vector<std::u32string>& scheme_templates);

#endif
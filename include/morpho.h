#pragma once
#include <string>
#include <vector>
#include <optional>

// Conversion helpers (UTF-8 <-> UTF-32)
std::u32string utf8_to_u32(const std::string& s);
std::string u32_to_utf8(const std::u32string& s);

// Normalisation de base
std::u32string normalize_ar(const std::u32string& s);

// Génération : root (u32, 3 lettres) + template (u32, avec ف ع ل) -> mot u32
std::u32string apply_template(const std::u32string& root, const std::u32string& templ);

// Extraction : tenter d'extraire une racine à partir d'un mot et d'un template.
// Retourne racine (3 lettres) si template correspond, sinon empty optional.
std::optional<std::u32string> extract_root_from_word(const std::u32string& word, const std::u32string& templ);

// Validation (test tous les schèmes fournis) : retourne noms des schèmes qui valident
std::vector<std::u32string> find_schemes_matching(const std::u32string& word, const std::vector<std::u32string>& scheme_templates);

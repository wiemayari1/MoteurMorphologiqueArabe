#ifndef ARABIC_DISPLAY_H
#define ARABIC_DISPLAY_H

#include "unicode_utils.h"
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

extern const std::string RESET;
extern const std::string BOLD;
extern const std::string RED;
extern const std::string GREEN;
extern const std::string YELLOW;
extern const std::string BLUE;
extern const std::string MAGENTA;
extern const std::string CYAN;
extern const std::string WHITE;

inline void print_ar(const std::string &utf8_str,
                     const std::string &color = "") {
  if (utf8_str.empty())
    return;

  if (!color.empty())
    std::cout << color;

  // Convertir en UTF-32
  auto u32 = unicode::utf8_to_u32(utf8_str);

  // Afficher en ordre inverse 
  for (int i = (int)u32.size() - 1; i >= 0; --i) {
    std::cout << unicode::u32_to_utf8(std::vector<char32_t>{u32[i]});
  }

  if (!color.empty())
    std::cout << RESET;
}

/**
 * avec retour à la ligne
 */
inline void print_ar_ln(const std::string &utf8_str,
                        const std::string &color = "") {
  print_ar(utf8_str, color);
  std::cout << "\n";
}

/**
 * Pour les vecteurs UTF-32 
 */
inline void print_ar_u32(const std::vector<char32_t> &u32_str,
                         const std::string &color = "") {
  print_ar(unicode::u32_to_utf8(u32_str), color);
}

/**
 * Formaté : numéro + texte arabe
 */
inline void print_numbered_ar(int num, const std::string &utf8_str,
                              const std::string &num_color = CYAN,
                              const std::string &text_color = "") {
  std::cout << num_color << std::setw(2) << num << "." << RESET << " ";
  print_ar(utf8_str, text_color);
}

/**
 * Deux colonnes arabe (ex: nom | pattern)
 */
inline void print_ar_two_cols(const std::string &left, const std::string &right,
                              const std::string &left_color = "",
                              const std::string &sep_color = YELLOW,
                              const std::string &right_color = "") {
  print_ar(left, left_color);
  std::cout << " " << sep_color << "|" << RESET << " ";
  print_ar(right, right_color);
}

#endif // ARABIC_DISPLAY_H

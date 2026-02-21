#ifndef UNICODE_UTILS_H
#define UNICODE_UTILS_H

#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>


// ============================================================================
// Utilitaires de conversion UTF-8 <-> UTF-32
// ============================================================================
namespace unicode {

inline std::string codepoint_to_utf8(char32_t cp) {
  if (cp > 0x10FFFF) {
    throw std::invalid_argument("Codepoint hors plage");
  }
  if (cp >= 0xD800 && cp <= 0xDFFF) {
    throw std::invalid_argument("Codepoint surrogate interdit");
  }

  if (cp <= 0x7F) {
    return std::string(1, static_cast<char>(cp));
  }
  if (cp <= 0x7FF) {
    return {static_cast<char>(0xC0 | (cp >> 6)),
            static_cast<char>(0x80 | (cp & 0x3F))};
  }
  if (cp <= 0xFFFF) {
    return {static_cast<char>(0xE0 | (cp >> 12)),
            static_cast<char>(0x80 | ((cp >> 6) & 0x3F)),
            static_cast<char>(0x80 | (cp & 0x3F))};
  }
  return {static_cast<char>(0xF0 | (cp >> 18)),
          static_cast<char>(0x80 | ((cp >> 12) & 0x3F)),
          static_cast<char>(0x80 | ((cp >> 6) & 0x3F)),
          static_cast<char>(0x80 | (cp & 0x3F))};
}

inline std::string u32_to_utf8(const std::vector<char32_t> &vec) {
  std::string result;
  result.reserve(vec.size() * 4);
  for (char32_t cp : vec) {
    result += codepoint_to_utf8(cp);
  }
  return result;
}

inline std::vector<char32_t> utf8_to_u32(const std::string &utf8) {
  std::vector<char32_t> result;
  result.reserve(utf8.size());

  size_t i = 0;
  while (i < utf8.size()) {
    unsigned char c = static_cast<unsigned char>(utf8[i]);
    char32_t cp;

    if (c <= 0x7F) {
      cp = c;
      i++;
    } else if ((c & 0xE0) == 0xC0) {
      if (i + 1 >= utf8.size())
        throw std::invalid_argument("UTF-8 tronqué");
      unsigned char b2 = static_cast<unsigned char>(utf8[++i]);
      cp = ((c & 0x1F) << 6) | (b2 & 0x3F);
      i++;
    } else if ((c & 0xF0) == 0xE0) {
      if (i + 2 >= utf8.size())
        throw std::invalid_argument("UTF-8 tronqué");
      unsigned char b2 = static_cast<unsigned char>(utf8[++i]);
      unsigned char b3 = static_cast<unsigned char>(utf8[++i]);
      cp = ((c & 0x0F) << 12) | ((b2 & 0x3F) << 6) | (b3 & 0x3F);
      i++;
    } else if ((c & 0xF8) == 0xF0) {
      if (i + 3 >= utf8.size())
        throw std::invalid_argument("UTF-8 tronqué");
      unsigned char b2 = static_cast<unsigned char>(utf8[++i]);
      unsigned char b3 = static_cast<unsigned char>(utf8[++i]);
      unsigned char b4 = static_cast<unsigned char>(utf8[++i]);
      cp = ((c & 0x07) << 18) | ((b2 & 0x3F) << 12) | ((b3 & 0x3F) << 6) |
           (b4 & 0x3F);
      i++;
    } else {
      throw std::invalid_argument("Octet UTF-8 invalide");
    }

    if (cp > 0x10FFFF)
      throw std::invalid_argument("Codepoint trop grand");
    if (cp >= 0xD800 && cp <= 0xDFFF)
      throw std::invalid_argument("Surrogate");

    result.push_back(cp);
  }
  return result;
}

} // namespace unicode

#endif
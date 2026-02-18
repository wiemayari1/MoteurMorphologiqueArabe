#ifndef UNICODE_UTILS_H
#define UNICODE_UTILS_H

#include <string>
#include <stdexcept>
#include <cstdint>    // pour uint32_t si besoin (optionnel)

namespace unicode {

/**
 * Convertit un seul codepoint UTF-32 en séquence UTF-8 (1 à 4 octets)
 * @throws std::invalid_argument si le codepoint est invalide
 */
inline std::string codepoint_to_utf8(char32_t cp) {
    if (cp > 0x10FFFF) {
        throw std::invalid_argument("Codepoint hors plage Unicode (max 0x10FFFF)");
    }
    if (cp >= 0xD800 && cp <= 0xDFFF) {
        throw std::invalid_argument("Codepoint surrogate interdit dans UTF-32");
    }

    if (cp <= 0x7F) {
        return std::string(1, static_cast<char>(cp));
    }
    if (cp <= 0x7FF) {
        return {
            static_cast<char>(0xC0 | (cp >> 6)),
            static_cast<char>(0x80 | (cp & 0x3F))
        };
    }
    if (cp <= 0xFFFF) {
        return {
            static_cast<char>(0xE0 | (cp >> 12)),
            static_cast<char>(0x80 | ((cp >> 6) & 0x3F)),
            static_cast<char>(0x80 | (cp & 0x3F))
        };
    }
    // 4 octets
    return {
        static_cast<char>(0xF0 | (cp >> 18)),
        static_cast<char>(0x80 | ((cp >> 12) & 0x3F)),
        static_cast<char>(0x80 | ((cp >> 6)  & 0x3F)),
        static_cast<char>(0x80 | (cp & 0x3F))
    };
}

/**
 * Convertit une std::u32string (UTF-32) en std::string (UTF-8)
 */
inline std::string u32_to_utf8(const std::u32string& str) {
    std::string result;
    // Estimation conservatrice : 4 octets max par codepoint
    result.reserve(str.size() * 4);

    for (char32_t cp : str) {
        result += codepoint_to_utf8(cp);
    }
    return result;
}

/**
 * Convertit une std::string (UTF-8) en std::u32string (UTF-32)
 * @throws std::invalid_argument en cas de séquence UTF-8 invalide ou tronquée
 */
inline std::u32string utf8_to_u32(const std::string& utf8) {
    std::u32string result;
    result.reserve(utf8.size());  // estimation basse → on agrandira si besoin

    size_t i = 0;
    while (i < utf8.size()) {
        unsigned char c = static_cast<unsigned char>(utf8[i]);

        char32_t cp;

        if (c <= 0x7F) {
            // 1 octet (ASCII)
            cp = c;
            ++i;
        }
        else if ((c & 0xE0) == 0xC0) {
            // 2 octets
            if (i + 1 >= utf8.size()) {
                throw std::invalid_argument("Séquence UTF-8 tronquée (2 octets attendus)");
            }
            unsigned char b2 = static_cast<unsigned char>(utf8[++i]);
            if ((b2 & 0xC0) != 0x80) {
                throw std::invalid_argument("Octet de continuation invalide dans séquence 2 octets");
            }
            cp = ((c & 0x1F) << 6) | (b2 & 0x3F);
            ++i;
        }
        else if ((c & 0xF0) == 0xE0) {
            // 3 octets
            if (i + 2 >= utf8.size()) {
                throw std::invalid_argument("Séquence UTF-8 tronquée (3 octets attendus)");
            }
            unsigned char b2 = static_cast<unsigned char>(utf8[++i]);
            unsigned char b3 = static_cast<unsigned char>(utf8[++i]);
            if ((b2 & 0xC0) != 0x80 || (b3 & 0xC0) != 0x80) {
                throw std::invalid_argument("Octet de continuation invalide dans séquence 3 octets");
            }
            cp = ((c & 0x0F) << 12) | ((b2 & 0x3F) << 6) | (b3 & 0x3F);
            ++i;
        }
        else if ((c & 0xF8) == 0xF0) {
            // 4 octets
            if (i + 3 >= utf8.size()) {
                throw std::invalid_argument("Séquence UTF-8 tronquée (4 octets attendus)");
            }
            unsigned char b2 = static_cast<unsigned char>(utf8[++i]);
            unsigned char b3 = static_cast<unsigned char>(utf8[++i]);
            unsigned char b4 = static_cast<unsigned char>(utf8[++i]);
            if ((b2 & 0xC0) != 0x80 || (b3 & 0xC0) != 0x80 || (b4 & 0xC0) != 0x80) {
                throw std::invalid_argument("Octet de continuation invalide dans séquence 4 octets");
            }
            cp = ((c & 0x07) << 18) | ((b2 & 0x3F) << 12) | ((b3 & 0x3F) << 6) | (b4 & 0x3F);
            ++i;
        }
        else {
            throw std::invalid_argument("Octet de départ UTF-8 invalide");
        }

        // Vérifications supplémentaires
        if (cp > 0x10FFFF) {
            throw std::invalid_argument("Codepoint UTF-8 dépasse 0x10FFFF");
        }
        if (cp >= 0xD800 && cp <= 0xDFFF) {
            throw std::invalid_argument("Surrogate UTF-16 encodé dans UTF-8 (invalide)");
        }

        result.push_back(cp);
    }

    return result;
}

} // namespace unicode

// Pour compatibilité avec ton code existant (sans namespace)
#endif

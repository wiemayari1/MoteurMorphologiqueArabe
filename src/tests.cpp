#include <iostream>
#include <cassert>
#include "morpho.h"

int main() {
    auto r = normalize_ar(unicode::utf8_to_u32("كتب"));
    assert(r.size() == 3);

    auto gen = apply_template(unicode::utf8_to_u32("كتب"), unicode::utf8_to_u32("مفعول"));
    std::string gen_utf8 = unicode::u32_to_utf8(gen);
    std::cout << "Généré: " << gen_utf8 << std::endl;

    auto extracted = extract_root_from_word(
        unicode::utf8_to_u32("مكتوب"),
        unicode::utf8_to_u32("مفعول")
    );
    assert(extracted && normalize_ar(*extracted) == normalize_ar(unicode::utf8_to_u32("كتب")));

    std::cout << "Tests unitaires passés.\n";
    return 0;
}
#include <iostream>
#include <cassert>
#include "morpho.h"

int main() {
    // test conversion & normalisation basics
    auto r = normalize_ar(utf8_to_u32("كتب"));
    assert(r.size() == 3);
    // test generation
    auto gen = apply_template(utf8_to_u32("كتب"), utf8_to_u32("مفعول"));
    std::string gen_utf8 = u32_to_utf8(gen);
    std::cout << "Généré: " << gen_utf8 << std::endl;
    assert(gen_utf8 == "مكتوب");
    // test extraction
    auto extracted = extract_root_from_word(utf8_to_u32("مكتوب"), utf8_to_u32("مفعول"));
    assert(extracted && normalize_ar(*extracted) == normalize_ar(utf8_to_u32("كتب")));
    std::cout << "Tests unitaires passés.\n";
    return 0;
}

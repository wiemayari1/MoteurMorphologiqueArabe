#include "hash_table.h"

HashTable::HashTable(std::size_t capacity)
    : table(capacity) {}

std::size_t HashTable::hash(const std::u32string& s) const {
    std::size_t h = 1469598103934665603ull;
    for (char32_t c : s) {
        h ^= static_cast<std::size_t>(c);
        h *= 1099511628211ull;
    }
    return h;
}

void HashTable::put(const std::u32string& name, const std::u32string& templ) {
    if (table.empty()) return;
    std::size_t idx = hash(name) % table.size();
    for (std::size_t i = 0; i < table.size(); ++i) {
        std::size_t j = (idx + i) % table.size();
        if (!table[j].used || table[j].deleted || table[j].entry.name == name) {
            table[j].used = true;
            table[j].deleted = false;
            table[j].entry.name = name;
            table[j].entry.templ = templ;
            return;
        }
    }
}

SchemeEntry* HashTable::get(const std::u32string& name) {
    if (table.empty()) return nullptr;
    std::size_t idx = hash(name) % table.size();
    for (std::size_t i = 0; i < table.size(); ++i) {
        std::size_t j = (idx + i) % table.size();
        if (!table[j].used && !table[j].deleted) return nullptr;
        if (table[j].used && !table[j].deleted && table[j].entry.name == name)
            return &table[j].entry;
    }
    return nullptr;
}

void HashTable::remove(const std::u32string& name) {
    if (table.empty()) return;
    std::size_t idx = hash(name) % table.size();
    for (std::size_t i = 0; i < table.size(); ++i) {
        std::size_t j = (idx + i) % table.size();
        if (!table[j].used && !table[j].deleted) return;
        if (table[j].used && !table[j].deleted && table[j].entry.name == name) {
            table[j].deleted = true;
            return;
        }
    }
}

std::vector<SchemeEntry> HashTable::allSchemes() const {
    std::vector<SchemeEntry> res;
    for (const auto& b : table) {
        if (b.used && !b.deleted) res.push_back(b.entry);
    }
    return res;
}
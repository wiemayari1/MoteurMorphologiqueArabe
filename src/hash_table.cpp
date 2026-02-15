#include "hash_table.h"
#include <iostream>

// Implémentation de TransformRule
std::u32string TransformRule::apply(const std::u32string& root) const {
    if (root.size() != 3) return U"";
    std::u32string result;
    for (char32_t c : pattern) {
        if (c == U'ف') result.push_back(root[0]);
        else if (c == U'ع') result.push_back(root[1]);
        else if (c == U'ل') result.push_back(root[2]);
        else result.push_back(c);
    }
    return result;
}

bool TransformRule::matches(const std::u32string& word, const std::u32string& root) const {
    if (root.size() != 3) return false;
    if (word.size() != pattern.size()) return false;
    
    for (size_t i = 0; i < word.size(); ++i) {
        char32_t pc = pattern[i];
        char32_t expected;
        if (pc == U'ف') expected = root[0];
        else if (pc == U'ع') expected = root[1];
        else if (pc == U'ل') expected = root[2];
        else expected = pc;
        
        if (word[i] != expected) return false;
    }
    return true;
}

// Implémentation de HashTable
HashTable::HashTable(std::size_t capacity)
    : table(capacity), count(0), deleted_count(0) {}

std::size_t HashTable::hash(const std::u32string& s) const {
    std::size_t h = 1469598103934665603ull;
    for (char32_t c : s) {
        h ^= static_cast<std::size_t>(c);
        h *= 1099511628211ull;
    }
    return h;
}

void HashTable::resize() {
    std::size_t new_capacity = table.size() * 2;
    std::vector<Bucket> old_table = table;

    table.assign(new_capacity, Bucket{});
    count = 0;
    deleted_count = 0;

    for (const auto& bucket : old_table) {
        if (bucket.used && !bucket.deleted) {
            put(bucket.entry.name, bucket.entry.templ, 
                u32_to_utf8(bucket.entry.rule.description));
        }
    }
}

void HashTable::put(const std::u32string& name, const std::u32string& templ,
                    const std::string& description) {
    if (count >= table.size() * 0.7) {
        resize();
    }

    std::size_t idx = hash(name) % table.size();
    for (std::size_t i = 0; i < table.size(); ++i) {
        std::size_t j = (idx + i) % table.size();
        
        if (!table[j].used || table[j].deleted) {
            table[j].used = true;
            table[j].deleted = false;
            table[j].entry.name = name;
            table[j].entry.templ = templ;
            table[j].entry.rule.name = name;
            table[j].entry.rule.pattern = templ;
            table[j].entry.rule.description = utf8_to_u32(description.empty() ? 
                u32_to_utf8(templ) : description);
            count++;
            if (table[j].deleted) deleted_count--;
            return;
        }
        
        if (table[j].used && !table[j].deleted && table[j].entry.name == name) {
            table[j].entry.templ = templ;
            table[j].entry.rule.pattern = templ;
            table[j].entry.rule.description = utf8_to_u32(description.empty() ? 
                u32_to_utf8(templ) : description);
            return;
        }
    }
}

SchemeEntry* HashTable::get(const std::u32string& name) {
    if (table.empty()) return nullptr;
    std::size_t idx = hash(name) % table.size();
    for (std::size_t i = 0; i < table.size(); ++i) {
        std::size_t j = (idx + i) % table.size();
        
        if (!table[j].used && !table[j].deleted) {
            return nullptr;
        }
        
        if (table[j].used && !table[j].deleted && table[j].entry.name == name) {
            return &table[j].entry;
        }
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
            deleted_count++;
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

std::vector<std::pair<std::u32string, std::u32string>> 
HashTable::generateAllDerivatives(const std::u32string& root) const {
    std::vector<std::pair<std::u32string, std::u32string>> results;
    
    if (root.size() != 3) return results;
    
    for (const auto& b : table) {
        if (b.used && !b.deleted) {
            std::u32string derived = b.entry.rule.apply(root);
            if (!derived.empty()) {
                results.push_back({b.entry.name, derived});
            }
        }
    }
    return results;
}

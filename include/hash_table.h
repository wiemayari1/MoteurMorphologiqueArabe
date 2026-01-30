// Table de hachage simple par chaînage pour schèmes.
// clé : std::u32string (nom du schème), valeur : template std::u32string
#pragma once
#include <vector>
#include <string>
#include <utility>

struct Scheme {
    std::u32string name;
    std::u32string templ;
    std::string desc;
};

class HashTable {
    std::vector<std::vector<Scheme>> buckets;
    size_t capacity;
    size_t size_;

    static uint64_t hash_u32(const std::u32string& s) {
        uint64_t h = 1469598103934665603ULL; // FNV offset
        for (char32_t c : s) {
            h ^= static_cast<uint64_t>(c);
            h *= 1099511628211ULL;
        }
        return h;
    }

public:
    HashTable(size_t cap = 1024) : capacity(cap), buckets(cap), size_(0) {}

    void put(const std::u32string& key, const std::u32string& templ, const std::string& desc = "") {
        uint64_t h = hash_u32(key) % capacity;
        for (auto &s : buckets[h]) {
            if (s.name == key) {
                s.templ = templ;
                s.desc = desc;
                return;
            }
        }
        buckets[h].push_back({key, templ, desc});
        ++size_;
    }

    Scheme* get(const std::u32string& key) {
        uint64_t h = hash_u32(key) % capacity;
        for (auto &s : buckets[h]) {
            if (s.name == key) return &s;
        }
        return nullptr;
    }

    std::vector<Scheme> allSchemes() const {
        std::vector<Scheme> out;
        for (const auto &b : buckets)
            for (const auto &s : b) out.push_back(s);
        return out;
    }
};

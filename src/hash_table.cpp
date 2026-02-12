#include "hash_table.h"
#include <iostream>

HashTable::HashTable(std::size_t capacity)
    : table(capacity), count(0) {}

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

    for (const auto& bucket : old_table) {
        if (bucket.used && !bucket.deleted) {
            put(bucket.entry.name, bucket.entry.templ);
        }
    }
}

void HashTable::put(const std::u32string& name, const std::u32string& templ) {
    if (count >= table.size() * 0.7) {
        resize();
    }

    std::size_t idx = hash(name) % table.size();
    for (std::size_t i = 0; i < table.size(); ++i) {
        std::size_t j = (idx + i) % table.size();
        
        // Slot vide ou supprimé -> insertion
        if (!table[j].used || table[j].deleted) {
            table[j].used = true;
            table[j].deleted = false;
            table[j].entry.name = name;
            table[j].entry.templ = templ;
            count++;
            return;
        }
        
        // Mise à jour existante
        if (table[j].used && !table[j].deleted && table[j].entry.name == name) {
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
        
        if (!table[j].used && !table[j].deleted) {
            // Rencontré une case vide "jamais utilisée" -> fin de recherche
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
        
        if (!table[j].used && !table[j].deleted) return; // Pas trouvé
        
        if (table[j].used && !table[j].deleted && table[j].entry.name == name) {
            table[j].deleted = true;
            // On ne décrémente pas count pour simplifier (la case reste "occupée" pour le probing)
            // Ou on pourrait, mais la condition de resize est basée sur `count`.
            // Pour être précis, on pourrait compter les deleted séparément.
            // Ici on simplifie.
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

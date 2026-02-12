#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <string>
#include <vector>

struct SchemeEntry {
    std::u32string name;   // ex: "مفعول"
    std::u32string templ;  // ex: "مفعول" (avec ف/ع/ل)
};

class HashTable {
public:
    explicit HashTable(std::size_t capacity);

    void put(const std::u32string& name, const std::u32string& templ);
    SchemeEntry* get(const std::u32string& name);
    void remove(const std::u32string& name);      // optionnel mais défini
    std::vector<SchemeEntry> allSchemes() const;

private:
    struct Bucket {
        bool used = false;
        bool deleted = false;
        SchemeEntry entry;
    };

    std::vector<Bucket> table;
    std::size_t count;

    void resize();

    std::size_t hash(const std::u32string& s) const;
};

#endif
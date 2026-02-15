#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <string>
#include <vector>

// Représentation d'une règle de transformation algorithmique
struct TransformRule {
    std::u32string name;        // Nom de la règle (ex: "فاعل")
    std::u32string description; // Description (ex: "ف + ا + ع + ل")
    std::u32string pattern;     // Pattern avec ف/ع/ل (ex: "فاعل")
    
    // Applique la règle à une racine trilitère
    std::u32string apply(const std::u32string& root) const;
    
    // Vérifie si un mot correspond à cette règle avec une racine donnée
    bool matches(const std::u32string& word, const std::u32string& root) const;
};

struct SchemeEntry {
    std::u32string name;        // ex: "مفعول"
    std::u32string templ;       // ex: "مفعول" (avec ف/ع/ل)
    TransformRule rule;         // Règle de transformation associée
};

class HashTable {
public:
    explicit HashTable(std::size_t capacity = 16);

    void put(const std::u32string& name, const std::u32string& templ, 
             const std::string& description = "");
    SchemeEntry* get(const std::u32string& name);
    void remove(const std::u32string& name);
    std::vector<SchemeEntry> allSchemes() const;
    
    // Génère tous les dérivés d'une racine avec tous les schèmes
    std::vector<std::pair<std::u32string, std::u32string>> generateAllDerivatives(
        const std::u32string& root) const;

private:
    struct Bucket {
        bool used = false;
        bool deleted = false;
        SchemeEntry entry;
    };

    std::vector<Bucket> table;
    std::size_t count;
    std::size_t deleted_count;

    void resize();
    std::size_t hash(const std::u32string& s) const;
};

#endif

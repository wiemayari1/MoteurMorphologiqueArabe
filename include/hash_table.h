#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <string>
#include <vector>

// ============================================================================
// Règle de transformation algorithmique
// ============================================================================
struct TransformRule {
  std::vector<char32_t> name;
  std::vector<char32_t> description;
  std::vector<char32_t> pattern;

  std::vector<char32_t> apply(const std::vector<char32_t> &root) const;
  bool matches(const std::vector<char32_t> &word,
               const std::vector<char32_t> &root) const;
};

struct SchemeEntry {
  std::vector<char32_t> name;
  std::vector<char32_t> templ;
  TransformRule rule;
};

// ============================================================================
// Table de hachage manuelle avec sondage linéaire
// ============================================================================
class HashTable {
public:
  explicit HashTable(std::size_t capacity = 16);

  void put(const std::vector<char32_t> &name,
           const std::vector<char32_t> &templ,
           const std::string &description = "");

  SchemeEntry *get(const std::vector<char32_t> &name);
  void remove(const std::vector<char32_t> &name);

  std::vector<SchemeEntry> allSchemes() const;

  std::vector<std::pair<std::vector<char32_t>, std::vector<char32_t>>>
  generateAllDerivatives(const std::vector<char32_t> &root) const;

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
  std::size_t hash(const std::vector<char32_t> &s) const;
};

#endif
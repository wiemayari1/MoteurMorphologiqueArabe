#include "hash_table.h"
#include "unicode_utils.h"
#include <iostream>

// ============================================================================
// TransformRule
// ============================================================================
std::vector<char32_t>
TransformRule::apply(const std::vector<char32_t> &root) const {
  if (root.size() != 3)
    return {};

  std::vector<char32_t> result;
  result.reserve(pattern.size());

  for (char32_t c : pattern) {
    if (c == U'ف')
      result.push_back(root[0]);
    else if (c == U'ع')
      result.push_back(root[1]);
    else if (c == U'ل')
      result.push_back(root[2]);
    else
      result.push_back(c);
  }
  return result;
}

bool TransformRule::matches(const std::vector<char32_t> &word,
                            const std::vector<char32_t> &root) const {
  if (root.size() != 3 || word.size() != pattern.size())
    return false;

  for (size_t i = 0; i < word.size(); ++i) {
    char32_t pc = pattern[i];
    char32_t expected;

    if (pc == U'ف')
      expected = root[0];
    else if (pc == U'ع')
      expected = root[1];
    else if (pc == U'ل')
      expected = root[2];
    else
      expected = pc;

    if (word[i] != expected)
      return false;
  }
  return true;
}

// ============================================================================
// HashTable
// ============================================================================
HashTable::HashTable(std::size_t capacity)
    : table(capacity), count(0), deleted_count(0) {}

// Fonction de hachage ORIGINALE (pas FNV-1a)
std::size_t HashTable::hash(const std::vector<char32_t> &s) const {
  std::size_t h = 0;
  std::size_t prime = 31;

  for (size_t i = 0; i < s.size(); i++) {
    h = h * prime + static_cast<std::size_t>(s[i]);
    h = (h << 5) | (h >> (sizeof(std::size_t) * 8 - 5));
    h ^= (i + 1) * 0x9e3779b9;
  }
  return h;
}

void HashTable::resize() {
  std::size_t new_capacity = table.size() * 2;
  std::vector<Bucket> old_table = table;

  table.assign(new_capacity, Bucket{});
  count = 0;
  deleted_count = 0;

  for (const auto &bucket : old_table) {
    if (bucket.used && !bucket.deleted) {
      put(bucket.entry.name, bucket.entry.templ,
          unicode::u32_to_utf8(bucket.entry.rule.description));
    }
  }
}

void HashTable::put(const std::vector<char32_t> &name,
                    const std::vector<char32_t> &templ,
                    const std::string &description) {
  if (count >= table.size() * 0.7) {
    resize();
  }

  std::size_t idx = hash(name) % table.size();
  for (std::size_t i = 0; i < table.size(); ++i) {
    std::size_t j = (idx + i) % table.size();

    if (!table[j].used || table[j].deleted) {
      if (table[j].deleted)
        deleted_count--;

      table[j].used = true;
      table[j].deleted = false;
      table[j].entry.name = name;
      table[j].entry.templ = templ;
      table[j].entry.rule.name = name;
      table[j].entry.rule.pattern = templ;
      table[j].entry.rule.description =
          description.empty() ? templ : unicode::utf8_to_u32(description);
      count++;
      return;
    }

    if (table[j].used && !table[j].deleted && table[j].entry.name == name) {
      table[j].entry.templ = templ;
      table[j].entry.rule.pattern = templ;
      table[j].entry.rule.description =
          description.empty() ? templ : unicode::utf8_to_u32(description);
      return;
    }
  }
}

SchemeEntry *HashTable::get(const std::vector<char32_t> &name) {
  if (table.empty())
    return nullptr;

  std::size_t idx = hash(name) % table.size();
  for (std::size_t i = 0; i < table.size(); ++i) {
    std::size_t j = (idx + i) % table.size();

    if (!table[j].used && !table[j].deleted)
      return nullptr;

    if (table[j].used && !table[j].deleted && table[j].entry.name == name) {
      return &table[j].entry;
    }
  }
  return nullptr;
}

void HashTable::remove(const std::vector<char32_t> &name) {
  if (table.empty())
    return;

  std::size_t idx = hash(name) % table.size();
  for (std::size_t i = 0; i < table.size(); ++i) {
    std::size_t j = (idx + i) % table.size();

    if (!table[j].used && !table[j].deleted)
      return;

    if (table[j].used && !table[j].deleted && table[j].entry.name == name) {
      table[j].deleted = true;
      deleted_count++;
      return;
    }
  }
}

std::vector<SchemeEntry> HashTable::allSchemes() const {
  std::vector<SchemeEntry> res;
  for (const auto &b : table) {
    if (b.used && !b.deleted)
      res.push_back(b.entry);
  }
  return res;
}

std::vector<std::pair<std::vector<char32_t>, std::vector<char32_t>>>
HashTable::generateAllDerivatives(const std::vector<char32_t> &root) const {
  std::vector<std::pair<std::vector<char32_t>, std::vector<char32_t>>> results;

  if (root.size() != 3)
    return results;

  for (const auto &b : table) {
    if (b.used && !b.deleted) {
      auto derived = b.entry.rule.apply(root);
      if (!derived.empty()) {
        results.push_back({b.entry.name, derived});
      }
    }
  }
  return results;
}
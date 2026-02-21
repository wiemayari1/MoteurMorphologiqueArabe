#ifndef AVL_H
#define AVL_H

#include <functional>
#include <string>
#include <vector>

// ============================================================================
// Nœud AVL avec gestion des dérivés
// ============================================================================
struct AVLNode {
  std::vector<char32_t> key;
  std::vector<std::vector<char32_t>> derived;
  int frequency;
  AVLNode *left;
  AVLNode *right;
  int height;

  AVLNode(const std::vector<char32_t> &k)
      : key(k), frequency(0), left(nullptr), right(nullptr), height(1) {}
};

// ============================================================================
// Arbre AVL équilibré
// ============================================================================
class AVLTree {
public:
  AVLTree() : root(nullptr) {}
  ~AVLTree();

  void insert(const std::vector<char32_t> &root);
  void remove(const std::vector<char32_t> &root);
  bool contains(const std::vector<char32_t> &root) const;

  void incrementFrequency(const std::vector<char32_t> &key);
  void addDerived(const std::vector<char32_t> &root_key,
                  const std::vector<char32_t> &derived);

  void forEach(std::function<void(const AVLNode *)> callback) const;
  std::vector<std::vector<char32_t>> getAllKeys() const;
  void getAllKeys(const std::function<void(const AVLNode *)> &callback) const;

private:
  AVLNode *root;

  AVLNode *insert(AVLNode *node, const std::vector<char32_t> &key);
  AVLNode *remove(AVLNode *node, const std::vector<char32_t> &key,
                  bool &deleted);
  AVLNode *find(AVLNode *node, const std::vector<char32_t> &key) const;

  int height(AVLNode *node) const;
  int getBalance(AVLNode *node) const;
  AVLNode *rotateLeft(AVLNode *x);
  AVLNode *rotateRight(AVLNode *y);
};

#endif
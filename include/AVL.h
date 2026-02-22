#ifndef AVL_H
#define AVL_H

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

struct AVLNode {
  std::vector<char32_t> key;
  int height;
  int frequency;
  std::vector<std::vector<char32_t>> derived;
  AVLNode *left;
  AVLNode *right;

  AVLNode(const std::vector<char32_t> &k)
      : key(k), height(1), frequency(1), left(nullptr), right(nullptr) {}
};

class AVLTree {
private:
  AVLNode *root;

  AVLNode *insert(AVLNode *node, const std::vector<char32_t> &key);
  AVLNode *remove(AVLNode *node, const std::vector<char32_t> &key,
                  bool &deleted);
  AVLNode *find(AVLNode *node, const std::vector<char32_t> &key) const;
  AVLNode *rotateRight(AVLNode *y);
  AVLNode *rotateLeft(AVLNode *x);
  int heightNode(AVLNode *node) const;
  int getBalance(AVLNode *node) const;
  void forEachImpl(AVLNode *node,
                   std::function<void(const AVLNode *)> callback) const;

public:
  AVLTree() : root(nullptr) {}
  ~AVLTree();

  void insert(const std::vector<char32_t> &key);
  void remove(const std::vector<char32_t> &key);
  bool contains(const std::vector<char32_t> &key) const;
  void incrementFrequency(const std::vector<char32_t> &key);
  void addDerived(const std::vector<char32_t> &root_key,
                  const std::vector<char32_t> &d);

  int height() const;
  void forEach(std::function<void(const AVLNode *)> callback) const;

  std::vector<std::vector<char32_t>> getAllKeys() const;
};

#endif
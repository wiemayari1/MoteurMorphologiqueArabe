#ifndef AVL_H
#define AVL_H

#include <functional>
#include <string>
#include <vector>

struct AVLNode {
  std::u32string key;                  // racine (ex: "كتب")
  std::vector<std::u32string> derived; // dérivés validés
  int frequency; // fréquence (nombre de validations/générations)
  AVLNode *left;
  AVLNode *right;
  int height;

  AVLNode(const std::u32string &k)
      : key(k), frequency(0), left(nullptr), right(nullptr), height(1) {}
};

class AVLTree {
public:
  AVLTree() : root(nullptr) {}
  ~AVLTree(); // Destructeur pour libérer la mémoire

  void insert(const std::u32string &root);
  void remove(const std::u32string &root); // ✅ Added remove
  bool contains(const std::u32string &root) const;

  // Incrémente la fréquence d'une racine (utilisé lors d'une
  // validation/génération)
  void incrementFrequency(const std::u32string &key);

  // Ajoute un dérivé validé à une racine
  void addDerived(const std::u32string &root_key,
                  const std::u32string &derived);

  // Parcours infixe et callback sur chaque nœud
  void forEach(std::function<void(const AVLNode *)> callback) const;

  // Retourne toutes les racines
  std::vector<std::u32string> getAllKeys() const;

  // Même chose, mais avec callback direct
  void getAllKeys(const std::function<void(const AVLNode *)> &callback) const;

private:
  AVLNode *root;

  AVLNode *insert(AVLNode *node, const std::u32string &key);
  AVLNode *remove(AVLNode *node, const std::u32string &key,
                  bool &deleted); // ✅ Added helper
  AVLNode *find(AVLNode *node, const std::u32string &key) const;

  int height(AVLNode *node) const;
  int getBalance(AVLNode *node) const;
  AVLNode *rotateLeft(AVLNode *x);
  AVLNode *rotateRight(AVLNode *y);
};

#endif
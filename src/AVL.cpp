#include "AVL.h"
#include <algorithm>

// ============================================================================
// Destructeur récursif
// ============================================================================
static void destroyTree(AVLNode *n) {
  if (!n)
    return;
  destroyTree(n->left);
  destroyTree(n->right);
  delete n;
}

AVLTree::~AVLTree() { destroyTree(root); }

// ============================================================================
// Helpers de hauteur et balance
// ============================================================================
static int nodeHeight(AVLNode *n) { return n ? n->height : 0; }

int AVLTree::height(AVLNode *node) const { return nodeHeight(node); }

int AVLTree::getBalance(AVLNode *node) const {
  if (!node)
    return 0;
  return nodeHeight(node->left) - nodeHeight(node->right);
}

// ============================================================================
// Rotations
// ============================================================================
AVLNode *AVLTree::rotateRight(AVLNode *y) {
  AVLNode *x = y->left;
  AVLNode *T2 = x->right;

  x->right = y;
  y->left = T2;

  y->height = 1 + std::max(nodeHeight(y->left), nodeHeight(y->right));
  x->height = 1 + std::max(nodeHeight(x->left), nodeHeight(x->right));

  return x;
}

AVLNode *AVLTree::rotateLeft(AVLNode *x) {
  AVLNode *y = x->right;
  AVLNode *T2 = y->left;

  y->left = x;
  x->right = T2;

  x->height = 1 + std::max(nodeHeight(x->left), nodeHeight(x->right));
  y->height = 1 + std::max(nodeHeight(y->left), nodeHeight(y->right));

  return y;
}

// ============================================================================
// Insertion
// ============================================================================
AVLNode *AVLTree::insert(AVLNode *node, const std::vector<char32_t> &key) {
  if (!node)
    return new AVLNode(key);

  if (key < node->key)
    node->left = insert(node->left, key);
  else if (key > node->key)
    node->right = insert(node->right, key);
  else
    return node;

  node->height = 1 + std::max(nodeHeight(node->left), nodeHeight(node->right));
  int balance = getBalance(node);

  // Left Left
  if (balance > 1 && key < node->left->key)
    return rotateRight(node);

  // Right Right
  if (balance < -1 && key > node->right->key)
    return rotateLeft(node);

  // Left Right
  if (balance > 1 && key > node->left->key) {
    node->left = rotateLeft(node->left);
    return rotateRight(node);
  }

  // Right Left
  if (balance < -1 && key < node->right->key) {
    node->right = rotateRight(node->right);
    return rotateLeft(node);
  }

  return node;
}

void AVLTree::insert(const std::vector<char32_t> &key) {
  root = insert(root, key);
}

// ============================================================================
// Recherche
// ============================================================================
AVLNode *AVLTree::find(AVLNode *node, const std::vector<char32_t> &key) const {
  if (!node)
    return nullptr;
  if (key == node->key)
    return node;
  if (key < node->key)
    return find(node->left, key);
  return find(node->right, key);
}

bool AVLTree::contains(const std::vector<char32_t> &key) const {
  return find(root, key) != nullptr;
}

// ============================================================================
// Gestion des dérivés et fréquences
// ============================================================================
void AVLTree::incrementFrequency(const std::vector<char32_t> &key) {
  AVLNode *n = find(root, key);
  if (n)
    n->frequency++;
}

void AVLTree::addDerived(const std::vector<char32_t> &root_key,
                         const std::vector<char32_t> &d) {
  AVLNode *n = find(root, root_key);
  if (!n)
    return;

  for (const auto &x : n->derived)
    if (x == d)
      return;

  n->derived.push_back(d);
}

// ============================================================================
// Parcours infixe
// ============================================================================
static void inorder(const AVLNode *n,
                    const std::function<void(const AVLNode *)> &cb) {
  if (!n)
    return;
  inorder(n->left, cb);
  cb(n);
  inorder(n->right, cb);
}

void AVLTree::forEach(std::function<void(const AVLNode *)> callback) const {
  inorder(root, callback);
}

std::vector<std::vector<char32_t>> AVLTree::getAllKeys() const {
  std::vector<std::vector<char32_t>> res;
  forEach([&](const AVLNode *n) { res.push_back(n->key); });
  return res;
}

void AVLTree::getAllKeys(
    const std::function<void(const AVLNode *)> &callback) const {
  forEach(callback);
}

// ============================================================================
// Suppression avec rééquilibrage
// ============================================================================
static AVLNode *minValueNode(AVLNode *node) {
  while (node && node->left)
    node = node->left;
  return node;
}

AVLNode *AVLTree::remove(AVLNode *node, const std::vector<char32_t> &key,
                         bool &deleted) {
  if (!node)
    return nullptr;

  if (key < node->key) {
    node->left = remove(node->left, key, deleted);
  } else if (key > node->key) {
    node->right = remove(node->right, key, deleted);
  } else {
    deleted = true;

    if (!node->left || !node->right) {
      AVLNode *child = node->left ? node->left : node->right;
      delete node;
      return child;
    } else {
      AVLNode *succ = minValueNode(node->right);
      node->key = succ->key;
      node->frequency = succ->frequency;
      node->derived = succ->derived;
      bool dummy = false;
      node->right = remove(node->right, succ->key, dummy);
    }
  }

  if (!node)
    return nullptr;

  node->height = 1 + std::max(height(node->left), height(node->right));
  int balance = getBalance(node);

  // Left Left
  if (balance > 1 && getBalance(node->left) >= 0)
    return rotateRight(node);

  // Left Right
  if (balance > 1 && getBalance(node->left) < 0) {
    node->left = rotateLeft(node->left);
    return rotateRight(node);
  }

  // Right Right
  if (balance < -1 && getBalance(node->right) <= 0)
    return rotateLeft(node);

  // Right Left
  if (balance < -1 && getBalance(node->right) > 0) {
    node->right = rotateRight(node->right);
    return rotateLeft(node);
  }

  return node;
}

void AVLTree::remove(const std::vector<char32_t> &key) {
  bool deleted = false;
  root = remove(root, key, deleted);
}
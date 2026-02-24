#include "AVL.h"

// ============================================================================
// UTILITAIRES ALGORITHMIQUES MANUELS
// ============================================================================

static int max_int(int a, int b) { return (a > b) ? a : b; }

static bool vector_equals(const std::vector<char32_t> &a,
                          const std::vector<char32_t> &b) {
  if (a.size() != b.size())
    return false;
  for (size_t i = 0; i < a.size(); i++) {
    if (a[i] != b[i])
      return false;
  }
  return true;
}

static bool vector_less(const std::vector<char32_t> &a,
                        const std::vector<char32_t> &b) {
  size_t min_len = (a.size() < b.size()) ? a.size() : b.size();
  for (size_t i = 0; i < min_len; i++) {
    if (a[i] < b[i])
      return true;
    if (a[i] > b[i])
      return false;
  }
  return a.size() < b.size();
}

// ============================================================================
// DESTRUCTEUR
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
// HELPERS DE HAUTEUR ET BALANCE
// ============================================================================

static int nodeHeight(AVLNode *n) { return n ? n->height : 0; }

int AVLTree::heightNode(AVLNode *node) const { return nodeHeight(node); }

int AVLTree::getBalance(AVLNode *node) const {
  if (!node)
    return 0;
  return nodeHeight(node->left) - nodeHeight(node->right);
}

// ============================================================================
// ROTATIONS
// ============================================================================

AVLNode *AVLTree::rotateRight(AVLNode *y) {
  AVLNode *x = y->left;
  AVLNode *T2 = x->right;

  x->right = y;
  y->left = T2;

  y->height = 1 + max_int(nodeHeight(y->left), nodeHeight(y->right));
  x->height = 1 + max_int(nodeHeight(x->left), nodeHeight(x->right));

  return x;
}

AVLNode *AVLTree::rotateLeft(AVLNode *x) {
  AVLNode *y = x->right;
  AVLNode *T2 = y->left;

  y->left = x;
  x->right = T2;

  x->height = 1 + max_int(nodeHeight(x->left), nodeHeight(x->right));
  y->height = 1 + max_int(nodeHeight(y->left), nodeHeight(y->right));

  return y;
}

// ============================================================================
// INSERTION
// ============================================================================

AVLNode *AVLTree::insert(AVLNode *node, const std::vector<char32_t> &key) {
  if (!node)
    return new AVLNode(key);

  if (vector_less(key, node->key))
    node->left = insert(node->left, key);
  else if (vector_less(node->key, key))
    node->right = insert(node->right, key);
  else
    return node;

  node->height = 1 + max_int(nodeHeight(node->left), nodeHeight(node->right));

  int balance = getBalance(node);

  if (balance > 1 && vector_less(key, node->left->key))
    return rotateRight(node);

  if (balance < -1 && vector_less(node->right->key, key))
    return rotateLeft(node);

  if (balance > 1 && vector_less(node->left->key, key)) {
    node->left = rotateLeft(node->left);
    return rotateRight(node);
  }

  if (balance < -1 && vector_less(key, node->right->key)) {
    node->right = rotateRight(node->right);
    return rotateLeft(node);
  }

  return node;
}

void AVLTree::insert(const std::vector<char32_t> &key) {
  root = insert(root, key);
}

// ============================================================================
// RECHERCHE
// ============================================================================

AVLNode *AVLTree::find(AVLNode *node, const std::vector<char32_t> &key) const {
  if (!node)
    return nullptr;

  if (vector_equals(key, node->key))
    return node;

  if (vector_less(key, node->key))
    return find(node->left, key);
  else
    return find(node->right, key);
}

bool AVLTree::contains(const std::vector<char32_t> &key) const {
  return find(root, key) != nullptr;
}

// ============================================================================
// GESTION DES DÉRIVÉS ET FRÉQUENCES
// ============================================================================

void AVLTree::incrementFrequency(const std::vector<char32_t> &key) {
  AVLNode *n = find(root, key);
  if (n)
    n->frequency++;
}

static bool derived_contains(const std::vector<std::vector<char32_t>> &derived,
                             const std::vector<char32_t> &d) {
  for (size_t i = 0; i < derived.size(); i++) {
    if (vector_equals(derived[i], d))
      return true;
  }
  return false;
}

void AVLTree::addDerived(const std::vector<char32_t> &root_key,
                         const std::vector<char32_t> &d) {
  AVLNode *n = find(root, root_key);
  if (!n)
    return;

  if (!derived_contains(n->derived, d)) {
    n->derived.push_back(d);
  }
}

// ============================================================================
// PARCOURS AVEC std::function
// ============================================================================

void AVLTree::forEachImpl(AVLNode *node,
                          std::function<void(const AVLNode *)> callback) const {
  if (!node)
    return;

  forEachImpl(node->left, callback);
  callback(node);
  forEachImpl(node->right, callback);
}

void AVLTree::forEach(std::function<void(const AVLNode *)> callback) const {
  forEachImpl(root, callback);
}

std::vector<std::vector<char32_t>> AVLTree::getAllKeys() const {
  std::vector<std::vector<char32_t>> res;

  forEach([&](const AVLNode *n) { res.push_back(n->key); });

  return res;
}

// ============================================================================
// TROUVER LE MINIMUM (successeur pour la suppression)
// ============================================================================
static AVLNode *minValueNode(AVLNode *node) {
  AVLNode *current = node;
  while (current && current->left != nullptr) {
    current = current->left;
  }
  return current;
}

// ============================================================================
// SUPPRESSION
// ============================================================================

AVLNode *AVLTree::remove(AVLNode *node, const std::vector<char32_t> &key,
                         bool &deleted) {
  if (!node)
    return nullptr;

  if (vector_less(key, node->key)) {
    node->left = remove(node->left, key, deleted);
  } else if (vector_less(node->key, key)) {
    node->right = remove(node->right, key, deleted);
  } else {
    // Nœud trouvé
    deleted = true;

    // Cas 1 ou 2 enfants
    if (!node->left || !node->right) {
      AVLNode *child = node->left ? node->left : node->right;

      if (!child) {
        // Pas d'enfant - suppression simple
        delete node;
        return nullptr;
      } else {
        // Un enfant - remplacer par l'enfant
        // Copier les données de l'enfant vers le node courant
        node->key = child->key;
        node->frequency = child->frequency;
        node->derived = child->derived;
        node->left = child->left;
        node->right = child->right;
        node->height = child->height;

        delete child; // Supprimer l'enfant (pas le node)
        return node;  // Retourner le node modifié
      }
    } else {
      // Cas 2 enfants: trouver successeur (minimum du sous-arbre droit)
      AVLNode *succ = minValueNode(node->right);

      // Copier les données du successeur
      node->key = succ->key;
      node->frequency = succ->frequency;
      node->derived = succ->derived;

      // Supprimer le successeur du sous-arbre droit
      bool dummy = false;
      node->right = remove(node->right, succ->key, dummy);
    }
  }

  // Si l'arbre n'avait qu'un seul nœud
  if (!node)
    return nullptr;

  // 2. Mise à jour hauteur
  node->height = 1 + max_int(nodeHeight(node->left), nodeHeight(node->right));

  // 3. Rééquilibrage
  int balance = getBalance(node);

  // Gauche Gauche
  if (balance > 1 && getBalance(node->left) >= 0)
    return rotateRight(node);

  // Gauche Droite
  if (balance > 1 && getBalance(node->left) < 0) {
    node->left = rotateLeft(node->left);
    return rotateRight(node);
  }

  // Droite Droite
  if (balance < -1 && getBalance(node->right) <= 0)
    return rotateLeft(node);

  // Droite Gauche
  if (balance < -1 && getBalance(node->right) > 0) {
    node->right = rotateRight(node->right);
    return rotateLeft(node);
  }

  return node;
}

// ============================================================================
// SUPPRESSION PUBLIQUE (wrapper)
// ============================================================================

void AVLTree::remove(const std::vector<char32_t> &key) {
  bool dummy = false;
  root = remove(root, key, dummy);
}

// ============================================================================
// HAUTEUR PUBLIQUE
// ============================================================================

int AVLTree::height() const { return nodeHeight(root); }
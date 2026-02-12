#include "AVL.h"

#include <algorithm>
#include <utility>

static void destroyTree(AVLNode *n) {
  if (!n)
    return;
  destroyTree(n->left);
  destroyTree(n->right);
  delete n;
}

AVLTree::~AVLTree() { destroyTree(root); }

static int nodeHeight(AVLNode *n) { return n ? n->height : 0; }

int AVLTree::height(AVLNode *node) const { return nodeHeight(node); }

int AVLTree::getBalance(AVLNode *node) const {
  if (!node)
    return 0;
  return nodeHeight(node->left) - nodeHeight(node->right);
}

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

AVLNode *AVLTree::insert(AVLNode *node, const std::u32string &key) {
  if (!node)
    return new AVLNode(key);

  if (key < node->key) {
    node->left = insert(node->left, key);
  } else if (key > node->key) {
    node->right = insert(node->right, key);
  } else {
    // déjà présent
    return node;
  }

  node->height = 1 + std::max(nodeHeight(node->left), nodeHeight(node->right));
  int balance = getBalance(node);

  // Left Left
  if (balance > 1 && key < node->left->key) {
    return rotateRight(node);
  }
  // Right Right
  if (balance < -1 && key > node->right->key) {
    return rotateLeft(node);
  }
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

void AVLTree::insert(const std::u32string &key) { root = insert(root, key); }

AVLNode *AVLTree::find(AVLNode *node, const std::u32string &key) const {
  if (!node)
    return nullptr;
  if (key == node->key)
    return node;
  if (key < node->key)
    return find(node->left, key);
  return find(node->right, key);
}

bool AVLTree::contains(const std::u32string &key) const {
  return find(root, key) != nullptr;
}

void AVLTree::incrementFrequency(const std::u32string &key) {
  AVLNode *n = find(root, key);
  if (!n)
    return;
  n->frequency += 1;
}

void AVLTree::addDerived(const std::u32string &root_key,
                         const std::u32string &d) {
  AVLNode *n = find(root, root_key);
  if (!n)
    return;

  // éviter doublons
  for (const auto &x : n->derived) {
    if (x == d)
      return;
  }
  n->derived.push_back(d);
}

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

std::vector<std::u32string> AVLTree::getAllKeys() const {
  std::vector<std::u32string> res;
  forEach([&](const AVLNode *n) { res.push_back(n->key); });
  return res;
}

void AVLTree::getAllKeys(
    const std::function<void(const AVLNode *)> &callback) const {
  forEach(callback);
}

static AVLNode *minValueNode(AVLNode *node) {
  AVLNode *current = node;
  while (current && current->left != nullptr)
    current = current->left;
  return current;
}

AVLNode *deleteNode(AVLNode *root, const std::u32string &key, bool &deleted) {
  if (!root)
    return root;

  if (key < root->key) {
    root->left = deleteNode(root->left, key, deleted);
  } else if (key > root->key) {
    root->right = deleteNode(root->right, key, deleted);
  } else {
    // Trouvé
    deleted = true;
    if (!root->left || !root->right) {
      AVLNode *temp = root->left ? root->left : root->right;
      if (!temp) {
        temp = root;
        root = nullptr;
      } else {
        *root = *temp; // copie contenu
        // On ne delete pas temp ici car *root pointe dessus? Non, structure
        // standard AVL delete Correction standard: Copier le contenu n'est pas
        // safe avec les pointeurs left/right si on ne fait pas gaffe Refaisons
        // plus propre:
      }
      delete temp; // Attention: ci-dessus logic flaw si root=temp.
    } else {
      // 2 enfants
      AVLNode *temp = minValueNode(root->right);
      root->key = temp->key;
      root->frequency = temp->frequency;
      root->derived = temp->derived; // coûteux mais ok
      root->right = deleteNode(root->right, temp->key, deleted);
    }
  }

  if (!root)
    return root;

  // Mise à jour hauteur
  root->height = 1 + std::max(nodeHeight(root->left), nodeHeight(root->right));
  int balance = 0; // Calcul balance
  if (root)
    balance =
        nodeHeight(root->left) -
        nodeHeight(
            root->right); // Recopie logic getBalance locale ou appel méthode

  // Rotations (copie de insert logic mais adaptée)
  // On doit avoir accès à rotateLeft/Right qui sont membres...
  // Problème: deleteNode est statique ici ou membre ?
  // Il vaut mieux le faire membre pour accéder aux rotations.
  return root;
}

// Réimplémentation propre en membre de la classe AVLTree pour accéder aux
// helpers
AVLNode *AVLTree::remove(AVLNode *root, const std::u32string &key,
                         bool &deleted) {
  if (!root)
    return root;

  if (key < root->key) {
    root->left = remove(root->left, key, deleted);
  } else if (key > root->key) {
    root->right = remove(root->right, key, deleted);
  } else {
    deleted = true;
    if ((!root->left) || (!root->right)) {
      AVLNode *temp = root->left ? root->left : root->right;
      if (!temp) {
        temp = root;
        root = nullptr;
      } else {
        *root = *temp;
        // Attention ici: écraser root par temp copie les pointeurs de temp.
        // Il faut s'assurer que l'ancien `root` (qui est `this` dans ce
        // contexte de node) est bien géré. L'idiome standard: AVLNode* temp =
        // root->left ? root->left : root->right; if(!temp) { temp = root; root
        // = NULL; } else *root = *temp; delete temp;
      }
      delete temp;
    } else {
      AVLNode *temp = minValueNode(root->right);
      root->key = temp->key;
      root->frequency = temp->frequency;
      root->derived = temp->derived;
      root->right = remove(root->right, temp->key, deleted);
    }
  }

  if (!root)
    return root;

  root->height = 1 + std::max(height(root->left), height(root->right));
  int balance = getBalance(root);

  // Left Left
  if (balance > 1 && getBalance(root->left) >= 0)
    return rotateRight(root);

  // Left Right
  if (balance > 1 && getBalance(root->left) < 0) {
    root->left = rotateLeft(root->left);
    return rotateRight(root);
  }

  // Right Right
  if (balance < -1 && getBalance(root->right) <= 0)
    return rotateLeft(root);

  // Right Left
  if (balance < -1 && getBalance(root->right) > 0) {
    root->right = rotateRight(root->right);
    return rotateLeft(root);
  }

  return root;
}

void AVLTree::remove(const std::u32string &key) {
  bool deleted = false;
  root = remove(root, key, deleted);
}

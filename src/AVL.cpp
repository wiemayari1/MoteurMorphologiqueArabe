#include "AVL.h"

#include <algorithm>
#include <utility>

static int nodeHeight(AVLNode* n) {
    return n ? n->height : 0;
}

int AVLTree::height(AVLNode* node) const {
    return nodeHeight(node);
}

int AVLTree::getBalance(AVLNode* node) const {
    if (!node) return 0;
    return nodeHeight(node->left) - nodeHeight(node->right);
}

AVLNode* AVLTree::rotateRight(AVLNode* y) {
    AVLNode* x = y->left;
    AVLNode* T2 = x->right;

    x->right = y;
    y->left = T2;

    y->height = 1 + std::max(nodeHeight(y->left), nodeHeight(y->right));
    x->height = 1 + std::max(nodeHeight(x->left), nodeHeight(x->right));

    return x;
}

AVLNode* AVLTree::rotateLeft(AVLNode* x) {
    AVLNode* y = x->right;
    AVLNode* T2 = y->left;

    y->left = x;
    x->right = T2;

    x->height = 1 + std::max(nodeHeight(x->left), nodeHeight(x->right));
    y->height = 1 + std::max(nodeHeight(y->left), nodeHeight(y->right));

    return y;
}

AVLNode* AVLTree::insert(AVLNode* node, const std::u32string& key) {
    if (!node) return new AVLNode(key);

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

void AVLTree::insert(const std::u32string& key) {
    root = insert(root, key);
}

AVLNode* AVLTree::find(AVLNode* node, const std::u32string& key) const {
    if (!node) return nullptr;
    if (key == node->key) return node;
    if (key < node->key) return find(node->left, key);
    return find(node->right, key);
}

bool AVLTree::contains(const std::u32string& key) const {
    return find(root, key) != nullptr;
}

void AVLTree::incrementFrequency(const std::u32string& key) {
    AVLNode* n = find(root, key);
    if (!n) return;
    n->frequency += 1;
}

void AVLTree::addDerived(const std::u32string& root_key, const std::u32string& d) {
    AVLNode* n = find(root, root_key);
    if (!n) return;

    // éviter doublons
    for (const auto& x : n->derived) {
        if (x == d) return;
    }
    n->derived.push_back(d);
}

static void inorder(const AVLNode* n, const std::function<void(const AVLNode*)>& cb) {
    if (!n) return;
    inorder(n->left, cb);
    cb(n);
    inorder(n->right, cb);
}

void AVLTree::forEach(std::function<void(const AVLNode*)> callback) const {
    inorder(root, callback);
}

std::vector<std::u32string> AVLTree::getAllKeys() const {
    std::vector<std::u32string> res;
    forEach([&](const AVLNode* n) { res.push_back(n->key); });
    return res;
}

void AVLTree::getAllKeys(const std::function<void(const AVLNode*)>& callback) const {
    forEach(callback);
}

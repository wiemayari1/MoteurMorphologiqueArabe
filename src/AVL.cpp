#include "AVL.h"
#include <algorithm>

int AVLTree::height(AVLNode* node) const {
    return node ? node->height : 0;
}

int AVLTree::getBalance(AVLNode* node) const {
    return node ? height(node->left) - height(node->right) : 0;
}

AVLNode* AVLTree::rotateRight(AVLNode* y) {
    AVLNode* x = y->left;
    AVLNode* T2 = x->right;

    x->right = y;
    y->left = T2;

    y->height = std::max(height(y->left), height(y->right)) + 1;
    x->height = std::max(height(x->left), height(x->right)) + 1;

    return x;
}

AVLNode* AVLTree::rotateLeft(AVLNode* x) {
    AVLNode* y = x->right;
    AVLNode* T2 = y->left;

    y->left = x;
    x->right = T2;

    x->height = std::max(height(x->left), height(x->right)) + 1;
    y->height = std::max(height(y->left), height(y->right)) + 1;

    return y;
}

AVLNode* AVLTree::insert(AVLNode* node, const std::u32string& key) {
    if (!node) return new AVLNode(key);

    if (key < node->key) {
        node->left = insert(node->left, key);
    } else if (key > node->key) {
        node->right = insert(node->right, key);
    } else {
        return node; // pas de doublons
    }

    node->height = 1 + std::max(height(node->left), height(node->right));
    int balance = getBalance(node);

    if (balance > 1 && key < node->left->key)
        return rotateRight(node);
    if (balance < -1 && key > node->right->key)
        return rotateLeft(node);
    if (balance > 1 && key > node->left->key) {
        node->left = rotateLeft(node->left);
        return rotateRight(node);
    }
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
    if (!node || node->key == key) return node;
    if (key < node->key) return find(node->left, key);
    return find(node->right, key);
}

bool AVLTree::contains(const std::u32string& key) const {
    return find(root, key) != nullptr;
}

void AVLTree::incrementFrequency(const std::u32string& key) {
    AVLNode* n = find(root, key);
    if (n) n->frequency++;
}

void AVLTree::addDerived(const std::u32string& root_key,
                         const std::u32string& derived) {
    AVLNode* n = find(root, root_key);
    if (n) n->derived.push_back(derived);
}

void AVLTree::forEach(std::function<void(const AVLNode*)> callback) const {
    std::function<void(const AVLNode*)> inorder = [&](const AVLNode* node) {
        if (!node) return;
        inorder(node->left);
        callback(node);
        inorder(node->right);
    };
    inorder(root);
}

void AVLTree::getAllKeys(const std::function<void(const AVLNode*)>& callback) const {
    forEach(callback);
}

std::vector<std::u32string> AVLTree::getAllKeys() const {
    std::vector<std::u32string> keys;
    getAllKeys([&](const AVLNode* n){ keys.push_back(n->key); });
    return keys;
}
// AVL tree minimal pour stocker racines (UTF-32 keys).
#pragma once
#include <memory>
#include <string>
#include <vector>
#include <algorithm>
#include <iostream>

// Node stocke key (UTF-32) et liste de dérivés (UTF-32)
struct AVLNode {
    std::u32string key;
    std::vector<std::u32string> derived;
    int height;
    std::unique_ptr<AVLNode> left;
    std::unique_ptr<AVLNode> right;

    AVLNode(const std::u32string& k)
        : key(k), height(1) {}
};

class AVLTree {
    std::unique_ptr<AVLNode> root;

    int height(const std::unique_ptr<AVLNode>& n) const {
        return n ? n->height : 0;
    }
    int balanceFactor(const std::unique_ptr<AVLNode>& n) const {
        return n ? height(n->left) - height(n->right) : 0;
    }
    void fixHeight(std::unique_ptr<AVLNode>& n) {
        n->height = 1 + std::max(height(n->left), height(n->right));
    }

    void rotateRight(std::unique_ptr<AVLNode>& p) {
        auto q = std::move(p->left);
        p->left = std::move(q->right);
        q->right = std::move(p);
        fixHeight(q->right);
        fixHeight(q);
        p = std::move(q);
    }

    void rotateLeft(std::unique_ptr<AVLNode>& q) {
        auto p = std::move(q->right);
        q->right = std::move(p->left);
        p->left = std::move(q);
        fixHeight(p->left);
        fixHeight(p);
        q = std::move(p);
    }

    void balance(std::unique_ptr<AVLNode>& n) {
        if (!n) return;
        fixHeight(n);
        int bf = balanceFactor(n);
        if (bf == 2) {
            if (balanceFactor(n->left) < 0)
                rotateLeft(n->left);
            rotateRight(n);
        } else if (bf == -2) {
            if (balanceFactor(n->right) > 0)
                rotateRight(n->right);
            rotateLeft(n);
        }
    }

    void insertNode(std::unique_ptr<AVLNode>& node, const std::u32string& key) {
        if (!node) {
            node = std::make_unique<AVLNode>(key);
            return;
        }
        if (key < node->key)
            insertNode(node->left, key);
        else if (key > node->key)
            insertNode(node->right, key);
        else
            return; // existant : ne rien faire
        balance(node);
    }

    AVLNode* findNode(AVLNode* node, const std::u32string& key) const {
        if (!node) return nullptr;
        if (key == node->key) return node;
        if (key < node->key) return findNode(node->left.get(), key);
        return findNode(node->right.get(), key);
    }

    void inorderPrint(const std::unique_ptr<AVLNode>& n, std::function<void(const AVLNode*)> f) const {
        if (!n) return;
        inorderPrint(n->left, f);
        f(n.get());
        inorderPrint(n->right, f);
    }

public:
    void insert(const std::u32string& key) {
        insertNode(root, key);
    }

    bool contains(const std::u32string& key) const {
        return findNode(root.get(), key) != nullptr;
    }

    AVLNode* find(const std::u32string& key) const {
        return findNode(root.get(), key);
    }

    void addDerived(const std::u32string& key, const std::u32string& word) {
        auto* n = find(key);
        if (!n) return;
        // éviter duplicata simple
        if (std::find(n->derived.begin(), n->derived.end(), word) == n->derived.end())
            n->derived.push_back(word);
    }

    void printAll(std::function<void(const AVLNode*)> f) const {
        inorderPrint(root, f);
    }
};

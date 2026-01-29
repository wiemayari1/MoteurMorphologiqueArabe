// include/AVLTree.hpp
#include <string>
#include <vector>

struct Node {
    std::string racine;             // ex: "كتب" [cite: 64]
    std::vector<std::string> derives; // Liste des mots validés [cite: 66]
    int frequence;                  // [cite: 68]
    Node *left, *right;
    int height;
};

class AVLTree {
public:
    void insert(std::string r);    // Insertion dynamique [cite: 28]
    Node* search(std::string r);   // Recherche logarithmique 
    void display();                 // Affichage structuré [cite: 30]
};

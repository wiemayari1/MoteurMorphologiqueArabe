#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "AVL.h"
#include "hash_table.h"
#include "morpho.h"
#include "unicode_utils.h"   // <-- contient utf8_to_u32 / u32_to_utf8

using namespace std;
using namespace unicode;
/* ==============================
   Chargement des données
================================ */

bool load_roots(const string& file, AVLTree& avl)
{
    ifstream in(file);
    if (!in) {
        cerr << "Erreur ouverture roots.txt\n";
        return false;
    }

    string line;
    while (getline(in, line)) {
        if (line.empty()) continue;
        avl.insert(normalize_ar(utf8_to_u32(line)));
    }

    return true;
}

bool load_schemes(const string& file, HashTable& ht)
{
    ifstream in(file);
    if (!in) {
        cerr << "Erreur ouverture schemes.txt\n";
        return false;
    }

    string name, templ;
    while (in >> name >> templ) {
        ht.put(utf8_to_u32(name), utf8_to_u32(templ));
    }

    return true;
}

/* ==============================
   Affichage menu
================================ */

void show_menu()
{
    cout << "\n=== Moteur Morphologique Arabe ===\n";
    cout << "1) Générer un mot (racine + schème)\n";
    cout << "2) Vérifier appartenance morphologique\n";
    cout << "3) Lister les schèmes\n";
    cout << "4) Lister les racines et dérivés\n";
    cout << "5) Ajouter une racine\n";
    cout << "6) Mini-jeu morphologique (bonus)\n";
    cout << "7) Générer famille morphologique complète\n";
    cout << "8) Gestion des schèmes\n";
    cout << "9) Quitter\n";
    cout << "Votre choix : ";
}

/* ==============================
   Actions CLI
================================ */

void generate_word(AVLTree& avl, HashTable& ht)
{
    string root_utf8, scheme_utf8;

    cout << "Racine : ";
    cin >> root_utf8;

    cout << "Schème : ";
    cin >> scheme_utf8;

    auto root = normalize_ar(utf8_to_u32(root_utf8));
    auto scheme = utf8_to_u32(scheme_utf8);

    auto result = generate_from_scheme(root, scheme, ht);

    cout << "\nMot généré : "
         << u32_to_utf8(result) << "\n";
}

void validate_word_cli(AVLTree& avl, HashTable& ht)
{
    string word_utf8, root_utf8;

    cout << "Mot : ";
    cin >> word_utf8;

    cout << "Racine attendue : ";
    cin >> root_utf8;

    auto word = utf8_to_u32(word_utf8);
    auto root = normalize_ar(utf8_to_u32(root_utf8));

    auto res = validate_word(word, root, ht);

    if (res.valid)
        cout << "✅ OUI — Schème : "
             << u32_to_utf8(res.scheme) << "\n";
    else
        cout << "❌ NON\n";
}

void add_root_cli(AVLTree& avl)
{
    string root_utf8;
    cout << "Nouvelle racine : ";
    cin >> root_utf8;

    avl.insert(normalize_ar(utf8_to_u32(root_utf8)));
    cout << "Racine ajoutée.\n";
}

void list_roots_cli(AVLTree& avl)
{
    vector<u32string> roots = avl.inorder();

    cout << "\n--- Racines ---\n";
    for (auto& r : roots)
        cout << u32_to_utf8(r) << "\n";
}

void list_schemes_cli(HashTable& ht)
{
    auto all = ht.entries();

    cout << "\n--- Schèmes ---\n";
    for (auto& p : all)
        cout << u32_to_utf8(p.first)
             << " -> "
             << u32_to_utf8(p.second) << "\n";
}

/* ==============================
   Mini jeu (BONUS)
================================ */

void mini_game(AVLTree& avl, HashTable& ht)
{
    auto root = avl.random_root();

    cout << "\nRacine : " << u32_to_utf8(root) << "\n";
    cout << "Propose un mot dérivé : ";

    string answer;
    cin >> answer;

    auto res = validate_word(
        utf8_to_u32(answer),
        root,
        ht
    );

    if (res.valid)
        cout << "Bravo !\n";
    else
        cout << "Incorrect.\n";
}

/* ==============================
   MAIN
================================ */

int main()
{
    AVLTree avl;
    HashTable ht;

    load_roots("data/roots.txt", avl);
    load_schemes("data/schemes.txt", ht);

    int choix;

    while (true)
    {
        show_menu();
        cin >> choix;

        switch (choix)
        {
            case 1: generate_word(avl, ht); break;
            case 2: validate_word_cli(avl, ht); break;
            case 3: list_schemes_cli(ht); break;
            case 4: list_roots_cli(avl); break;
            case 5: add_root_cli(avl); break;
            case 6: mini_game(avl, ht); break;
            case 7:
                cout << "Fonction famille morphologique à connecter.\n";
                break;
            case 8:
                cout << "Gestion schèmes (add/edit/delete).\n";
                break;
            case 9:
                cout << "Au revoir.\n";
                return 0;
            default:
                cout << "Choix invalide.\n";
        }
    }
}

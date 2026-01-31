// CLI minimal pour démonstration
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include "morpho.h"
#include "AVL.h"
#include "hash_table.h"

using namespace std;

static string read_file_utf8(const string& path) {
    ifstream f(path, ios::binary);
    if (!f) return "";
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

int main(int argc, char** argv) {
    if (argc < 3) {
        cerr << "Usage: " << argv[0] << " roots.txt schemes.txt\n";
        return 1;
    }
    string roots_content = read_file_utf8(argv[1]);
    string schemes_content = read_file_utf8(argv[2]);

    AVLTree tree;
    HashTable ht(2048);

    // charger racines
    {
        istringstream s(roots_content);
        string line;
        while (std::getline(s, line)) {
            if (line.empty()) continue;
            auto u = utf8_to_u32(line);
            auto n = normalize_ar(u);
            if (n.size() == 3) tree.insert(n);
        }
    }

    // charger schèmes (format: NAME|TEMPLATE)
    {
        istringstream s(schemes_content);
        string line;
        while (std::getline(s, line)) {
            if (line.empty()) continue;
            auto pos = line.find('|');
            if (pos == string::npos) continue;
            string name = line.substr(0, pos);
            string templ = line.substr(pos+1);
            ht.put(utf8_to_u32(name), utf8_to_u32(templ));
        }
    }

    cout << "Chargement OK. Racines chargées (in-order):\n";
    tree.printAll([](const AVLNode* n){
        cout << " - " << u32_to_utf8(n->key) << " (" << n->derived.size() << " dérivés)\n";
    });

    // menu simple
    while (true) {
        cout << "\nMenu:\n1) Générer mot (racine, schème)\n2) Valider mot vs racine\n3) Lister schèmes\n4) Lister racines et dérivés\n5) Ajouter racine\n6) Quitter\nChoix: ";
        int c; if (!(cin >> c)) break;
        string dummy; getline(cin, dummy);
        if (c == 1) {
            string r, sname;
            cout << "Racine (UTF-8): "; getline(cin, r);
            cout << "Nom du schème (ex: مفعول) : "; getline(cin, sname);
            auto ur = normalize_ar(utf8_to_u32(r));
            auto scheme = ht.get(utf8_to_u32(sname));
            if (!scheme) { cout << "Schème inconnu\n"; continue; }
            try {
                auto gen = apply_template(ur, scheme->templ);
                cout << "Mot généré: " << u32_to_utf8(gen) << "\n";
                // stocker dérivé
                tree.addDerived(ur, gen);
            } catch (const exception& e) {
                cout << "Erreur: " << e.what() << "\n";
            }
        } else if (c == 2) {
            string word, root;
            cout << "Mot (UTF-8): "; getline(cin, word);
            cout << "Racine candidate (UTF-8): "; getline(cin, root);
            auto uw = utf8_to_u32(word);
            auto ur = normalize_ar(utf8_to_u32(root));
            // tester tous les schèmes
            vector<string> matches;
            for (auto &s : ht.allSchemes()) {
                auto extracted = extract_root_from_word(uw, s.templ);
                if (extracted && normalize_ar(*extracted) == ur) {
                    matches.push_back(u32_to_utf8(s.name));
                    // ajouter dérivé à la racine stockée si la racine existe dans l'arbre
                    if (tree.contains(ur)) tree.addDerived(ur, normalize_ar(uw));
                }
            }
            if (matches.empty()) {
                cout << "NON — aucun schème trouvé correspondant.\n";
            } else {
                cout << "OUI. Schème(s) reconnu(s): ";
                for (auto &m : matches) cout << m << " ";
                cout << "\n";
            }
        } else if (c == 3) {
            cout << "Schèmes disponibles:\n";
            for (auto &s : ht.allSchemes()) {
                cout << " - " << u32_to_utf8(s.name) << " | template=" << u32_to_utf8(s.templ) << "\n";
            }
        } else if (c == 4) {
            cout << "Racines et dérivés:\n";
            tree.printAll([&](const AVLNode* n){
                cout << " * " << u32_to_utf8(n->key) << " -> ";
                for (auto &d : n->derived) cout << u32_to_utf8(d) << " ";
                cout << "\n";
            });
        } else if (c == 5) {
            string r; cout << "Nouvelle racine (UTF-8): "; getline(cin, r);
            auto ur = normalize_ar(utf8_to_u32(r));
            if (ur.size() != 3) cout << "Racine invalide après normalisation (doit être 3 lettres)\n";
            else {
                if (!tree.contains(ur)) { tree.insert(ur); cout << "Racine insérée\n"; }
                else cout << "Racine existante\n";
            }
        } else if (c == 6) {
            cout << "Au revoir\n"; break;
        } else {
            cout << "Choix invalide\n";
        }
    }

    return 0;
}

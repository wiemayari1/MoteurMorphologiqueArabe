#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

#include "AVL.h"
#include "hash_table.h"
#include "morpho.h"

using namespace std;

// ----------------------
// Outils d'affichage TUI
// ----------------------

void clear_screen() {
  // Efface l'écran (fonctionne dans la plupart des terminaux)
  cout << "\033[2J\033[H";
}

void print_logo() {
  cout << "\033[1;35m"; // magenta
  cout << "   Moteur Morphologique Arabe     \n";
  cout << "\033[0m\n";
}

void print_menu() {
  print_logo();
  cout << "\033[1;36m=== Menu principal ===\033[0m\n";
  cout << "  1) Générer un mot (racine + schème)\n";
  cout << "  2) Vérifier appartenance morphologique\n";
  cout << "  3) Lister les schèmes\n";
  cout << "  4) Lister les racines et dérivés\n";
  cout << "  5) Ajouter une racine\n";
  cout << "  6) Mini-jeu morphologique\n";
  cout << "  8) Ajouter un schème\n";
  cout << "  9) Modifier un schème\n";
  cout << "  10) Supprimer un schème\n";
  cout << "  11) Supprimer une racine\n";
  cout << "  7) Quitter\n";
  cout << "\nVotre choix : ";
}

// Lire un fichier texte UTF-8 complet dans une string
static string read_file_utf8(const string &path) {
  ifstream f(path, ios::binary);
  if (!f) {
    cerr << "Erreur: impossible d'ouvrir le fichier " << path << endl;
    return "";
  }
  ostringstream ss;
  ss << f.rdbuf();
  return ss.str();
}

int main(int argc, char **argv) {
  if (argc < 3) {
    cerr << "Usage: " << argv[0] << " data/roots.txt data/schemes.txt\n";
    return 1;
  }

  string roots_path = argv[1];
  string schemes_path = argv[2];

  AVLTree tree;
  HashTable ht(2048);

  // ---------------------
  // Chargement des racines
  // ---------------------
  {
    string content = read_file_utf8(roots_path);
    istringstream in(content);
    string line;
    while (getline(in, line)) {
      if (line.empty())
        continue;
      auto r_u32 = normalize_ar(unicode::utf8_to_u32(line));
      if (r_u32.size() == 3) {
        tree.insert(r_u32);
      } else {
        cerr << "Ignore racine invalide (pas trilittère) : " << line << endl;
      }
    }
  }

  // ---------------------
  // Chargement des schèmes (NOM|TEMPLATE)
  // ---------------------
  {
    string content = read_file_utf8(schemes_path);
    istringstream in(content);
    string line;
    while (getline(in, line)) {
      if (line.empty())
        continue;
      auto pos = line.find('|');
      if (pos == string::npos) {
        cerr << "Ligne de schème invalide : " << line << endl;
        continue;
      }
      string name = line.substr(0, pos);
      string templ = line.substr(pos + 1);

      auto name_u32 = unicode::utf8_to_u32(name);
      auto templ_u32 = unicode::utf8_to_u32(templ);
      ht.put(name_u32, templ_u32);
    }
  }

  // Boucle principale (TUI)
  while (true) {
    clear_screen();
    print_menu();

    int choix;
    if (!(cin >> choix)) {
      cout << "Entrée invalide, arrêt.\n";
      break;
    }
    cin.ignore(numeric_limits<streamsize>::max(), '\n'); // vider le buffer

    clear_screen();
    print_logo();

    if (choix == 1) {
      // === Génération de mot ===
      cout << "\033[1;33m[ Génération de mots dérivés ]\033[0m\n\n";
      string r_utf8, sname_utf8;
      cout << "Entrer la racine (en arabe, UTF-8) : ";
      getline(cin, r_utf8);
      cout << "Entrer le nom du schème (par ex. مفعول, فاعل, ...) : ";
      getline(cin, sname_utf8);

      auto r_u32 = normalize_ar(unicode::utf8_to_u32(r_utf8));
      auto sname = unicode::utf8_to_u32(sname_utf8);

      SchemeEntry *se = ht.get(sname);
      if (!se) {
        cout << "\033[1;31mSchème introuvable.\033[0m\n";
      } else {
        try {
          auto word = apply_template(r_u32, se->templ);
          cout << "Mot généré : \033[1;32m" << unicode::u32_to_utf8(word) << "\033[0m\n";

          // Mise à jour des dérivés + fréquence
          if (tree.contains(r_u32)) {
            tree.addDerived(r_u32, word);
            tree.incrementFrequency(r_u32);
          }
        } catch (const exception &e) {
          cout << "\033[1;31mErreur lors de la génération : " << e.what()
               << "\033[0m\n";
        }
      }

    } else if (choix == 2) {
      // === Vérification morphologique ===
      cout << "\033[1;33m[ Vérification d'appartenance morphologique "
              "]\033[0m\n\n";
      string w_utf8, r_utf8;
      cout << "Entrer le mot à vérifier : ";
      getline(cin, w_utf8);
      cout << "Entrer la racine candidate : ";
      getline(cin, r_utf8);

      auto w_u32 = unicode::utf8_to_u32(w_utf8);
      auto r_u32 = normalize_ar(unicode::utf8_to_u32(r_utf8));

      vector<u32string> matching_schemes;
      bool appartient = false;

      for (const auto &s : ht.allSchemes()) {
        auto maybe_r = extract_root_from_word(w_u32, s.templ);
        if (maybe_r) {
          auto rn = normalize_ar(*maybe_r);
          if (rn == r_u32) {
            appartient = true;
            matching_schemes.push_back(s.name);

            if (tree.contains(r_u32)) {
              tree.addDerived(r_u32, normalize_ar(w_u32));
              tree.incrementFrequency(r_u32);
            }
          }
        }
      }

      if (!appartient) {
        cout << "Résultat : \033[1;31mNON\033[0m, le mot ne correspond pas à "
                "cette racine.\n";
      } else {
        cout << "Résultat : \033[1;32mOUI\033[0m, le mot appartient à la "
                "racine donnée.\n";
        cout << "Schème(s) reconnu(s) : ";
        for (auto &n : matching_schemes) {
          cout << "\033[1;36m" << unicode::u32_to_utf8(n) << "\033[0m ";
        }
        cout << "\n";
      }

    } else if (choix == 3) {
      // === Lister les schèmes ===
      cout << "\033[1;33m[ Liste des schèmes ]\033[0m\n\n";
      for (const auto &s : ht.allSchemes()) {
        cout << " - Nom : \033[1;32m" << unicode::u32_to_utf8(s.name) << "\033[0m"
             << " | Template : \033[1;36m" << unicode::u32_to_utf8(s.templ)
             << "\033[0m\n";
      }

    } else if (choix == 4) {
      // === Lister les racines et leurs dérivés ===
      cout << "\033[1;33m[ Racines et dérivés ]\033[0m\n\n";
      tree.forEach([](const AVLNode *n) {
        cout << " * Racine : \033[1;32m" << unicode::u32_to_utf8(n->key) << "\033[0m"
             << " (freq=" << n->frequency << ")\n";
        if (!n->derived.empty()) {
          cout << "   Dérivés : ";
          for (const auto &d : n->derived) {
            cout << "\033[1;36m" << unicode::u32_to_utf8(d) << "\033[0m ";
          }
          cout << "\n";
        }
      });

    } else if (choix == 5) {
      // === Ajouter une racine ===
      cout << "\033[1;33m[ Ajout d'une nouvelle racine ]\033[0m\n\n";
      string r_utf8;
      cout << "Entrer la nouvelle racine (trilitère, en arabe) : ";
      getline(cin, r_utf8);

      auto r_u32 = normalize_ar(unicode::utf8_to_u32(r_utf8));
      if (r_u32.size() != 3) {
        cout << "\033[1;31mRacine invalide (après normalisation, doit avoir 3 "
                "lettres).\033[0m\n";
      } else if (tree.contains(r_u32)) {
        cout << "\033[1;31mCette racine existe déjà.\033[0m\n";
      } else {
        tree.insert(r_u32);
        cout << "\033[1;32mRacine ajoutée.\033[0m\n";
      }

    } else if (choix == 6) {
      // === Mini-jeu ===
      cout << "\033[1;33m[ Mini-jeu morphologique ]\033[0m\n\n";

      vector<u32string> roots;
      tree.getAllKeys([&](const AVLNode *n) { roots.push_back(n->key); });

      vector<u32string> scheme_names;
      vector<u32string> scheme_templates;
      for (const auto &s : ht.allSchemes()) {
        scheme_names.push_back(s.name);
        scheme_templates.push_back(s.templ);
      }

      play_minigame(roots, scheme_names, scheme_templates);

    } else if (choix == 7) {
      cout << "Au revoir.\n";
      break;

    } else if (choix == 8) {
      // === Ajouter un schème ===
      cout << "\033[1;33m[ Ajout d'un nouveau schème ]\033[0m\n\n";
      string name_utf8, templ_utf8;
      cout << "Entrer le nom du schème (ex: مفعول) : ";
      getline(cin, name_utf8);
      cout << "Entrer le template (ex: مفعول avec ف,ع,ل) : ";
      getline(cin, templ_utf8);

      auto name_u32 = unicode::utf8_to_u32(name_utf8);
      auto templ_u32 = unicode::utf8_to_u32(templ_utf8);

      if (name_u32.empty() || templ_u32.empty()) {
        cout << "\033[1;31mNom ou template vide.\033[0m\n";
      } else {
        ht.put(name_u32, templ_u32);
        cout << "\033[1;32mSchème ajouté/mis à jour.\033[0m\n";
      }

    } else if (choix == 9) {
      // === Modifier un schème ===
      cout << "\033[1;33m[ Modification d'un schème ]\033[0m\n\n";
      string name_utf8;
      cout << "Entrer le nom du schème à modifier : ";
      getline(cin, name_utf8);

      auto name_u32 = unicode::utf8_to_u32(name_utf8);
      if (ht.get(name_u32) == nullptr) {
        cout << "\033[1;31mSchème introuvable.\033[0m\n";
      } else {
        string templ_utf8;
        cout << "Entrer le nouveau template : ";
        getline(cin, templ_utf8);
        auto templ_u32 = unicode::utf8_to_u32(templ_utf8);

        ht.put(name_u32, templ_u32);
        cout << "\033[1;32mSchème modifié.\033[0m\n";
      }

    } else if (choix == 10) {
      // === Supprimer un schème ===
      cout << "\033[1;33m[ Suppression d'un schème ]\033[0m\n\n";
      string name_utf8;
      cout << "Entrer le nom du schème à supprimer : ";
      getline(cin, name_utf8);

      auto name_u32 = unicode::utf8_to_u32(name_utf8);
      if (ht.get(name_u32) == nullptr) {
        cout << "\033[1;31mSchème introuvable.\033[0m\n";
      } else {
        ht.remove(name_u32);
        cout << "\033[1;32mSchème supprimé.\033[0m\n";
      }

    } else if (choix == 11) {
      // === Supprimer une racine ===
      cout << "\033[1;33m[ Suppression d'une racine ]\033[0m\n\n";
      string r_utf8;
      cout << "Entrer la racine à supprimer : ";
      getline(cin, r_utf8);

      auto r_u32 = normalize_ar(unicode::utf8_to_u32(r_utf8));
      if (!tree.contains(r_u32)) {
        cout << "\033[1;31mRacine introuvable.\033[0m\n";
      } else {
        tree.remove(r_u32);
        cout << "\033[1;32mRacine supprimée.\033[0m\n";
      }

    } else {
      cout << "\033[1;31mChoix invalide.\033[0m\n";
    }

    cout << "\n\033[2m(Appuyez sur Entrée pour revenir au menu...)\033[0m";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
  }

  return 0;
}
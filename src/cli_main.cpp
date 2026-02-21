#include "AVL.h"
#include "hash_table.h"
#include "morpho.h"
#include "unicode_utils.h"
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>


using namespace std;

static string read_file(const string &path) {
  ifstream f(path, ios::binary);
  if (!f) {
    cerr << "Erreur : impossible d'ouvrir " << path << "\\n";
    return "";
  }
  ostringstream ss;
  ss << f.rdbuf();
  return ss.str();
}

bool load_roots(const string &file, AVLTree &avl) {
  string content = read_file(file);
  if (content.empty())
    return false;

  istringstream in(content);
  string line;
  int count = 0;

  while (getline(in, line)) {
    if (line.empty())
      continue;
    if (!line.empty() && line.back() == '\\r')
      line.pop_back();

    auto r = normalize_ar(unicode::utf8_to_u32(line));
    if (r.size() == 3) {
      avl.insert(r);
      count++;
    }
  }
  cout << count << " racines chargées.\\n";
  return true;
}

bool load_schemes(const string &file, HashTable &ht) {
  string content = read_file(file);
  if (content.empty())
    return false;

  istringstream in(content);
  string line;
  int count = 0;

  while (getline(in, line)) {
    if (line.empty())
      continue;
    if (!line.empty() && line.back() == '\\r')
      line.pop_back();

    auto sep = line.find('|');
    if (sep == string::npos)
      continue;

    string name = line.substr(0, sep);
    string templ = line.substr(sep + 1);
    ht.put(unicode::utf8_to_u32(name), unicode::utf8_to_u32(templ));
    count++;
  }
  cout << count << " schèmes chargés.\\n";
  return true;
}

void show_menu() {
  cout << "\\n=== Moteur Morphologique Arabe ===\\n";
  cout << "  1) Générer un mot (racine + schème)\\n";
  cout << "  2) Vérifier appartenance morphologique\\n";
  cout << "  3) Lister les schèmes\\n";
  cout << "  4) Lister les racines et dérivés\\n";
  cout << "  5) Ajouter une racine\\n";
  cout << "  6) Mini-jeu morphologique\\n";
  cout << "  7) Générer famille morphologique complète\\n";
  cout << "  8) Gestion des schèmes\\n";
  cout << "  9) Supprimer une racine\\n";
  cout << " 10) Quitter\\n";
  cout << "Votre choix : ";
}

void generate_word(AVLTree &avl, HashTable &ht) {
  cout << "\\n[ Génération de mot dérivé ]\\n";
  cout << "Racine (arabe, 3 lettres) : ";

  string root_utf8;
  getline(cin, root_utf8);

  cout << "Nom du schème (ex: مفعول) : ";
  string scheme_utf8;
  getline(cin, scheme_utf8);

  auto root = normalize_ar(unicode::utf8_to_u32(root_utf8));
  auto sname = normalize_ar(unicode::utf8_to_u32(scheme_utf8));

  if (root.size() != 3) {
    cout << "Racine invalide (doit avoir 3 lettres).\\n";
    return;
  }

  auto result = generate_from_scheme(root, sname, ht);
  if (result.empty()) {
    cout << "Schème introuvable.\\n";
    return;
  }

  cout << "Mot généré : " << unicode::u32_to_utf8(result) << "\\n";

  if (avl.contains(root)) {
    avl.addDerived(root, result);
    avl.incrementFrequency(root);
  }
}

void validate_word_cli(AVLTree &avl, HashTable &ht) {
  cout << "\\n[ Vérification morphologique ]\\n";
  cout << "Mot à vérifier : ";

  string word_utf8;
  getline(cin, word_utf8);

  cout << "Racine candidate : ";
  string root_utf8;
  getline(cin, root_utf8);

  auto word = normalize_ar(unicode::utf8_to_u32(word_utf8));
  auto root = normalize_ar(unicode::utf8_to_u32(root_utf8));

  auto res = validate_word(word, root, ht);

  if (res.valid) {
    cout << "Résultat : OUI — le mot appartient à la racine.\\n";
    cout << "Schème reconnu : " << unicode::u32_to_utf8(res.scheme) << "\\n";

    if (avl.contains(root)) {
      avl.addDerived(root, word);
      avl.incrementFrequency(root);
    }
  } else {
    cout << "Résultat : NON — le mot n'appartient pas à cette racine.\\n";
  }
}

void list_schemes_cli(HashTable &ht) {
  cout << "\\n[ Liste des schèmes ]\\n";
  auto all = ht.allSchemes();

  if (all.empty()) {
    cout << "Aucun schème enregistré.\\n";
    return;
  }

  for (const auto &s : all) {
    cout << " - " << unicode::u32_to_utf8(s.name)
         << " | Template : " << unicode::u32_to_utf8(s.templ) << "\\n";
  }
}

void list_roots_cli(AVLTree &avl) {
  cout << "\\n[ Racines et dérivés ]\\n";
  bool found = false;

  avl.forEach([&](const AVLNode *n) {
    found = true;
    cout << " * " << unicode::u32_to_utf8(n->key)
         << " (fréquence=" << n->frequency << ")";

    if (!n->derived.empty()) {
      cout << "\\n   Dérivés : ";
      for (const auto &d : n->derived) {
        cout << unicode::u32_to_utf8(d) << " ";
      }
    }
    cout << "\\n";
  });

  if (!found)
    cout << "Aucune racine enregistrée.\\n";
}

void add_root_cli(AVLTree &avl) {
  cout << "\\n[ Ajout d'une racine ]\\n";
  cout << "Nouvelle racine (3 lettres arabes) : ";

  string root_utf8;
  getline(cin, root_utf8);

  auto root = normalize_ar(unicode::utf8_to_u32(root_utf8));

  if (root.size() != 3) {
    cout << "Racine invalide (doit avoir exactement 3 lettres).\\n";
    return;
  }

  if (avl.contains(root)) {
    cout << "Cette racine existe déjà.\\n";
    return;
  }

  avl.insert(root);
  cout << "Racine ajoutée avec succès.\\n";
}

void mini_game(AVLTree &avl, HashTable &ht) {
  auto roots = avl.getAllKeys();

  vector<vector<char32_t>> scheme_names;
  vector<vector<char32_t>> scheme_templates;

  for (const auto &s : ht.allSchemes()) {
    scheme_names.push_back(s.name);
    scheme_templates.push_back(s.templ);
  }

  play_minigame(roots, scheme_names, scheme_templates);
}

void generate_family(AVLTree &avl, HashTable &ht) {
  cout << "\\n[ Famille morphologique complète ]\\n";
  cout << "Racine : ";

  string root_utf8;
  getline(cin, root_utf8);

  auto root = normalize_ar(unicode::utf8_to_u32(root_utf8));

  if (root.size() != 3) {
    cout << "Racine invalide.\\n";
    return;
  }

  cout << "\\nFamille de " << unicode::u32_to_utf8(root) << " :\\n";
  bool any = false;

  for (const auto &s : ht.allSchemes()) {
    auto word = apply_template(root, s.templ);
    if (!word.empty()) {
      cout << "  " << unicode::u32_to_utf8(s.name) << " → "
           << unicode::u32_to_utf8(word) << "\\n";

      if (avl.contains(root)) {
        avl.addDerived(root, word);
      }
      any = true;
    }
  }

  if (!any) {
    cout << "Aucun dérivé généré.\\n";
  } else if (avl.contains(root)) {
    avl.incrementFrequency(root);
  }
}

void manage_schemes(HashTable &ht) {
  cout << "\\n[ Gestion des schèmes ]\\n";
  cout << "  a) Ajouter un schème\\n";
  cout << "  b) Modifier un schème\\n";
  cout << "  c) Supprimer un schème\\n";
  cout << "Choix : ";

  string ch;
  getline(cin, ch);

  if (ch == "a" || ch == "A") {
    cout << "Nom du schème : ";
    string name;
    getline(cin, name);

    cout << "Template (avec ف/ع/ل) : ";
    string templ;
    getline(cin, templ);

    if (name.empty() || templ.empty()) {
      cout << "Nom ou template vide.\\n";
      return;
    }

    ht.put(unicode::utf8_to_u32(name), unicode::utf8_to_u32(templ));
    cout << "Schème ajouté.\\n";

  } else if (ch == "b" || ch == "B") {
    cout << "Nom du schème à modifier : ";
    string name;
    getline(cin, name);

    auto u32name = unicode::utf8_to_u32(name);
    if (!ht.get(u32name)) {
      cout << "Schème introuvable.\\n";
      return;
    }

    cout << "Nouveau template : ";
    string templ;
    getline(cin, templ);

    ht.put(u32name, unicode::utf8_to_u32(templ));
    cout << "Schème modifié.\\n";

  } else if (ch == "c" || ch == "C") {
    cout << "Nom du schème à supprimer : ";
    string name;
    getline(cin, name);

    auto u32name = unicode::utf8_to_u32(name);
    if (!ht.get(u32name)) {
      cout << "Schème introuvable.\\n";
      return;
    }

    ht.remove(u32name);
    cout << "Schème supprimé.\\n";
  } else {
    cout << "Choix invalide.\\n";
  }
}

void remove_root_cli(AVLTree &avl) {
  cout << "\\n[ Suppression d'une racine ]\\n";
  cout << "Racine à supprimer : ";

  string root_utf8;
  getline(cin, root_utf8);

  auto root = normalize_ar(unicode::utf8_to_u32(root_utf8));

  if (!avl.contains(root)) {
    cout << "Racine introuvable.\\n";
    return;
  }

  avl.remove(root);
  cout << "Racine supprimée.\\n";
}

int main() {
  AVLTree avl;
  HashTable ht(256);

  cout << "=== Moteur Morphologique Arabe — CLI ===\\n\\n";

  load_roots("data/roots.txt", avl);
  load_schemes("data/schemes.txt", ht);

  int choix;
  while (true) {
    show_menu();

    if (!(cin >> choix)) {
      cout << "Entrée invalide, arrêt.\\n";
      break;
    }
    cin.ignore(numeric_limits<streamsize>::max(), '\\n');

    switch (choix) {
    case 1:
      generate_word(avl, ht);
      break;
    case 2:
      validate_word_cli(avl, ht);
      break;
    case 3:
      list_schemes_cli(ht);
      break;
    case 4:
      list_roots_cli(avl);
      break;
    case 5:
      add_root_cli(avl);
      break;
    case 6:
      mini_game(avl, ht);
      break;
    case 7:
      generate_family(avl, ht);
      break;
    case 8:
      manage_schemes(ht);
      break;
    case 9:
      remove_root_cli(avl);
      break;
    case 10:
      cout << "Au revoir.\\n";
      return 0;
    default:
      cout << "Choix invalide.\\n";
    }

    cout << "\\n(Appuyez sur Entrée pour continuer...)";
    cin.ignore(numeric_limits<streamsize>::max(), '\\n');
  }

  return 0;
}
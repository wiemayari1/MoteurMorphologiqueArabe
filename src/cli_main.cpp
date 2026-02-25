#include "AVL.h"
#include "arabic_display.h"
#include "hash_table.h"
#include "morpho.h"
#include "unicode_utils.h"
#include <algorithm>
#include <cstdio>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

// ============================================================================
// CONSTANTES ET COULEURS ANSI
// ============================================================================
const string RESET = "\033[0m";
const string BOLD = "\033[1m";
const string RED = "\033[31m";
const string GREEN = "\033[32m";
const string YELLOW = "\033[33m";
const string BLUE = "\033[34m";
const string MAGENTA = "\033[35m";
const string CYAN = "\033[36m";
const string WHITE = "\033[37m";
const string BG_GREEN = "\033[42m";
const string BG_RED = "\033[41m";

// ============================================================================
// UTILITAIRES D'AFFICHAGE
// ============================================================================
void clear_screen() { cout << "\033[2J\033[H"; }

void print_header(const string &title) {
  cout << CYAN << "============================================================"
       << RESET << "\n";
  cout << CYAN << "  " << RESET << BOLD << title << RESET << CYAN << "  "
       << RESET << "\n";
  cout << CYAN << "============================================================"
       << RESET << "\n";
}

void print_success(const string &msg) {
  cout << GREEN << "✓ " << msg << RESET << "\n";
}

void print_error(const string &msg) {
  cout << RED << "✗ " << msg << RESET << "\n";
}

void print_info(const string &msg) {
  cout << BLUE << "ℹ " << msg << RESET << "\n";
}

void print_separator() { cout << CYAN << string(60, '-') << RESET << "\n"; }

void wait_enter() {
  cout << "\n" << YELLOW << "(Appuyez sur Entrée pour continuer...)" << RESET;
  cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

// ============================================================================
// CHARGEMENT DES DONNÉES
// ============================================================================
static string read_file(const string &path) {
  ifstream f(path, ios::binary);
  if (!f) {
    cerr << RED << "Erreur : impossible d'ouvrir " << path << RESET << "\n";
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
    if (!line.empty() && line.back() == '\r')
      line.pop_back();
    if (line[0] == '#')
      continue;

    auto r = normalize_ar(unicode::utf8_to_u32(line));
    if (r.size() == 3) {
      avl.insert(r);
      count++;
    }
  }
  print_success(to_string(count) + " racines chargees.");
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
    if (!line.empty() && line.back() == '\r')
      line.pop_back();
    if (line[0] == '#')
      continue;

    auto sep = line.find('|');
    if (sep == string::npos)
      continue;

    string name = line.substr(0, sep);
    string templ = line.substr(sep + 1);

    name.erase(0, name.find_first_not_of(" \t\r\n"));
    name.erase(name.find_last_not_of(" \t\r\n") + 1);
    templ.erase(0, templ.find_first_not_of(" \t\r\n"));
    templ.erase(templ.find_last_not_of(" \t\r\n") + 1);

    if (name.empty() || templ.empty())
      continue;

    ht.put(unicode::utf8_to_u32(name), unicode::utf8_to_u32(templ));
    count++;
  }
  print_success(to_string(count) + " schemes charges.");
  return true;
}

// ============================================================================
// MENU PRINCIPAL
// ============================================================================
void show_menu() {
  clear_screen();
  print_header("MOTEUR MORPHOLOGIQUE ARABE - CLI");

  cout << "\n" << BOLD << "Gestion des Racines :" << RESET << "\n";
  cout << "  " << CYAN << "1)" << RESET
       << " Generer un mot (racine + scheme)\n";
  cout << "  " << CYAN << "2)" << RESET
       << " Verifier appartenance morphologique\n";
  cout << "  " << CYAN << "3)" << RESET << " Ajouter une racine\n";
  cout << "  " << CYAN << "4)" << RESET << " Supprimer une racine\n";
  cout << "  " << CYAN << "5)" << RESET
       << " Lister toutes les racines et derives\n";
  cout << "  " << CYAN << "6)" << RESET
       << " Voir les derives d'une racine specifique\n";
  cout << "  " << CYAN << "7)" << RESET
       << " Generer famille morphologique complete\n";

  cout << "\n" << BOLD << "Gestion des Schemes :" << RESET << "\n";
  cout << "  " << CYAN << "8)" << RESET << " Lister les schemes\n";
  cout << "  " << CYAN << "9)" << RESET << " Ajouter un scheme\n";
  cout << "  " << CYAN << "10)" << RESET << " Modifier un scheme\n";
  cout << "  " << CYAN << "11)" << RESET << " Supprimer un scheme\n";

  cout << "\n" << BOLD << "Jeu :" << RESET << "\n";
  cout << "  " << CYAN << "12)" << RESET << " Mini-jeu morphologique\n";

  cout << "\n" << CYAN << "  0) Quitter" << RESET << "\n";
  cout << "\n" << YELLOW << "Votre choix : " << RESET;
}

// ============================================================================
// 1. GENERATION DE MOT (RACINE + SCHEME)
// ============================================================================
void generate_word(AVLTree &avl, HashTable &ht) {
  clear_screen();
  print_header("GENERATION DE MOT DERIVE");

  auto all_schemes = ht.allSchemes();
  if (all_schemes.empty()) {
    print_error("Aucun scheme charge!");
    wait_enter();
    return;
  }

  cout << "Racine (3 lettres arabes) : ";
  string root_utf8;
  getline(cin, root_utf8);

  auto root = normalize_ar(unicode::utf8_to_u32(root_utf8));
  if (root.size() != 3) {
    print_error("Racine invalide (doit avoir exactement 3 lettres).");
    wait_enter();
    return;
  }

  cout << "\nSchemes disponibles (" << all_schemes.size() << "):\n";
  for (size_t i = 0; i < all_schemes.size() && i < 10; i++) {
    string name = unicode::u32_to_utf8(all_schemes[i].name);
    string templ = unicode::u32_to_utf8(all_schemes[i].templ);

    cout << "  " << CYAN << (i + 1) << "." << RESET << " ";
    print_ar(name); // Affichage inversé (correct)
    cout << "  →  ";
    print_ar(templ); // Affichage inversé (correct)
    cout << "\n";
  }
  if (all_schemes.size() > 10) {
    cout << "  ... et " << (all_schemes.size() - 10) << " autres\n";
  }

  cout << "\nNom du scheme (ex: فاعل) : ";
  string scheme_utf8;
  getline(cin, scheme_utf8);

 
  auto sname = unicode::utf8_to_u32(scheme_utf8);

  // Optionnel : normaliser pour enlever les diacritiques si nécessaire
  sname = normalize_ar(sname);

  auto *entry = ht.get(sname);
  if (!entry) {
    // Essayer avec l'inverse au cas où le terminal a inversé la saisie
    auto sname_reversed = sname;
    reverse(sname_reversed.begin(), sname_reversed.end());
    entry = ht.get(sname_reversed);

    if (!entry) {
      cout << RED << "✗ Scheme '";
      print_ar(scheme_utf8); // Afficher ce que l'utilisateur a tapé
      cout << "' introuvable." << RESET << "\n";
      wait_enter();
      return;
    }
    // Si trouvé avec l'inverse, utiliser l'inverse pour la génération
    sname = sname_reversed;
  }

  auto result = entry->rule.apply(root);

  if (result.empty()) {
    print_error("Erreur lors de la generation du mot.");
  } else {
    print_separator();
    cout << BOLD << "RESULTAT :" << RESET << "\n";

    cout << "  Racine  : ";
    print_ar_u32(root, CYAN);
    cout << "\n";

    cout << "  Scheme  : ";
    print_ar(scheme_utf8, YELLOW); // Afficher l'entrée utilisateur
    cout << "\n";

    cout << "  Pattern : ";
    print_ar_u32(entry->templ, MAGENTA);
    cout << "\n";

    cout << "  Mot     : ";
    print_ar_u32(result, GREEN + BOLD);
    cout << "\n";

    print_separator();

    if (avl.contains(root)) {
      avl.addDerived(root, result);
      avl.incrementFrequency(root);
      print_info("Mot sauvegarde dans les derives de la racine.");
    } else {
      avl.insert(root);
      avl.addDerived(root, result);
      print_info("Nouvelle racine et mot sauvegardes.");
    }
  }
  wait_enter();
}

// ============================================================================
// 2. VERIFICATION APPARTENANCE MORPHOLOGIQUE
// ============================================================================
void validate_word_cli(AVLTree &avl, HashTable &ht) {
  clear_screen();
  print_header("VERIFICATION MORPHOLOGIQUE");

  cout << "Mot a verifier : ";
  string word_utf8;
  getline(cin, word_utf8);

  cout << "Racine candidate (3 lettres) : ";
  string root_utf8;
  getline(cin, root_utf8);

  auto word = normalize_ar(unicode::utf8_to_u32(word_utf8));
  auto root = normalize_ar(unicode::utf8_to_u32(root_utf8));

  if (root.size() != 3) {
    print_error("La racine doit avoir 3 lettres.");
    wait_enter();
    return;
  }

  bool found = false;
  string matched_scheme;

  auto all_schemes = ht.allSchemes();
  for (const auto &scheme_entry : all_schemes) {
    if (scheme_entry.rule.matches(word, root)) {
      found = true;
      matched_scheme = unicode::u32_to_utf8(scheme_entry.name);
      break;
    }
  }

  print_separator();
  cout << BOLD << "RESULTAT :" << RESET << "\n";

  cout << "  Mot    : ";
  print_ar(word_utf8);
  cout << "\n";

  cout << "  Racine : ";
  print_ar_u32(root);
  cout << "\n  ";

  if (found) {
    cout << BG_GREEN << WHITE << BOLD << " OUI " << RESET;
    cout << GREEN << " Le mot appartient a la racine." << RESET << "\n";
    cout << "  Scheme reconnu : ";
    print_ar(matched_scheme, CYAN);
    cout << "\n";

    if (avl.contains(root)) {
      avl.addDerived(root, word);
      avl.incrementFrequency(root);
    }
  } else {
    cout << BG_RED << WHITE << BOLD << " NON " << RESET;
    cout << RED << " Le mot n'appartient pas a cette racine." << RESET << "\n";
  }
  print_separator();

  wait_enter();
}

// ============================================================================
// 3. AJOUTER UNE RACINE
// ============================================================================
void add_root_cli(AVLTree &avl) {
  clear_screen();
  print_header("AJOUT D'UNE RACINE");

  cout << "Nouvelle racine (3 lettres arabes) : ";
  string root_utf8;
  getline(cin, root_utf8);

  auto root = normalize_ar(unicode::utf8_to_u32(root_utf8));

  if (root.size() != 3) {
    print_error("Racine invalide (doit avoir exactement 3 lettres).");
    wait_enter();
    return;
  }

  if (avl.contains(root)) {
    print_error("Cette racine existe deja.");
    wait_enter();
    return;
  }

  avl.insert(root);
  cout << GREEN << "✓ Racine '";
  print_ar_u32(root);
  cout << "' ajoutee avec succes." << RESET << "\n";
  wait_enter();
}

// ============================================================================
// 4. SUPPRIMER UNE RACINE
// ============================================================================
void remove_root_cli(AVLTree &avl) {
  clear_screen();
  print_header("SUPPRESSION D'UNE RACINE");

  cout << "Racine a supprimer : ";
  string root_utf8;
  getline(cin, root_utf8);

  auto root = normalize_ar(unicode::utf8_to_u32(root_utf8));

  if (!avl.contains(root)) {
    print_error("Racine introuvable.");
    wait_enter();
    return;
  }

  cout << YELLOW << "Confirmer la suppression de '";
  print_ar_u32(root);
  cout << "' ? (o/n) : " << RESET;

  string confirm;
  getline(cin, confirm);

  if (confirm == "o" || confirm == "O") {
    avl.remove(root);
    print_success("Racine supprimee.");
  } else {
    print_info("Operation annulee.");
  }
  wait_enter();
}

// ============================================================================
// 5. LISTER TOUTES LES RACINES ET DERIVES
// ============================================================================
void list_roots_cli(AVLTree &avl) {
  clear_screen();
  print_header("LISTE DES RACINES ET DERIVES");

  bool found = false;
  int count = 0;

  avl.forEach([&](const AVLNode *n) {
    found = true;
    count++;

    cout << CYAN << count << "." << RESET << " ";
    print_ar_u32(n->key, BOLD);
    cout << " (freq: " << n->frequency << ", derives: " << n->derived.size()
         << ")\n";

    if (!n->derived.empty()) {
      cout << "   Derives : ";
      for (size_t i = 0; i < n->derived.size(); i++) {
        if (i > 0)
          cout << ", ";
        print_ar_u32(n->derived[i], GREEN);
      }
      cout << "\n";
    }
    cout << "\n";
  });

  if (!found) {
    print_info("Aucune racine enregistree.");
  } else {
    print_separator();
    cout << "Total : " << count << " racine(s)\n";
  }
  wait_enter();
}

// ============================================================================
// 6. VOIR LES DERIVES D'UNE RACINE SPECIFIQUE
// ============================================================================
void view_root_derivatives(AVLTree &avl) {
  clear_screen();
  print_header("DERIVES D'UNE RACINE SPECIFIQUE");

  cout << "Entrez la racine : ";
  string root_utf8;
  getline(cin, root_utf8);

  auto root = normalize_ar(unicode::utf8_to_u32(root_utf8));

  if (!avl.contains(root)) {
    print_error("Racine introuvable.");
    wait_enter();
    return;
  }

  const AVLNode *target_node = nullptr;
  avl.forEach([&](const AVLNode *n) {
    if (n->key == root)
      target_node = n;
  });

  if (target_node) {
    cout << "\n" << BOLD << "Racine : ";
    print_ar_u32(target_node->key);
    cout << RESET << "\n";

    cout << "Frequence d'utilisation : " << target_node->frequency << "\n";
    cout << "Nombre de derives : " << target_node->derived.size() << "\n\n";

    if (!target_node->derived.empty()) {
      cout << BOLD << "Liste des derives :" << RESET << "\n";
      for (size_t i = 0; i < target_node->derived.size(); i++) {
        cout << "  " << (i + 1) << ". ";
        print_ar_u32(target_node->derived[i], CYAN);
        cout << "\n";
      }
    } else {
      print_info("Aucun derive enregistre pour cette racine.");
    }
  }
  wait_enter();
}

// ============================================================================
// 7. LISTER LES SCHEMES
// ============================================================================
void list_schemes_cli(HashTable &ht) {
  clear_screen();
  print_header("LISTE DES SCHEMES MORPHOLOGIQUES");

  auto all = ht.allSchemes();

  if (all.empty()) {
    print_info("Aucun scheme enregistre.");
    wait_enter();
    return;
  }

  print_separator();

  for (size_t i = 0; i < all.size(); i++) {
    string name = unicode::u32_to_utf8(all[i].name);
    string templ = unicode::u32_to_utf8(all[i].templ);

    cout << CYAN << setw(2) << (i + 1) << "." << RESET << " ";
    print_ar(name);
    cout << " " << YELLOW << "|" << RESET << " ";
    print_ar(templ);
    cout << "\n";
  }

  print_separator();
  cout << "\nTotal : " << all.size() << " scheme(s)\n";

  wait_enter();
}

// ============================================================================
// 8. AJOUTER UN SCHEME
// ============================================================================
void add_scheme_cli(HashTable &ht) {
  clear_screen();
  print_header("AJOUT D'UN SCHEME");

  cout << "Nom du scheme (ex: فاعل) : ";
  string name_utf8;
  getline(cin, name_utf8);

  cout << "Pattern (utilisez ف, ع, ل pour les lettres de la racine) : ";
  string templ_utf8;
  getline(cin, templ_utf8);

  cout << "Description (optionnelle) : ";
  string desc;
  getline(cin, desc);

  if (name_utf8.empty() || templ_utf8.empty()) {
    print_error("Nom et pattern sont obligatoires.");
    wait_enter();
    return;
  }

  auto templ_u32 = unicode::utf8_to_u32(templ_utf8);
  bool has_f = false, has_e = false, has_l = false;
  for (char32_t c : templ_u32) {
    if (c == U'ف')
      has_f = true;
    if (c == U'ع')
      has_e = true;
    if (c == U'ل')
      has_l = true;
  }

  if (!has_f || !has_e || !has_l) {
    print_error("Le pattern doit contenir les lettres ف, ع, et ل.");
    wait_enter();
    return;
  }

  ht.put(unicode::utf8_to_u32(name_utf8), templ_u32, desc);
  cout << GREEN << "✓ Scheme '";
  print_ar(name_utf8);
  cout << "' ajoute avec succes." << RESET << "\n";
  wait_enter();
}

// ============================================================================
// 9. MODIFIER UN SCHEME
// ============================================================================
void update_scheme_cli(HashTable &ht) {
  clear_screen();
  print_header("MODIFICATION D'UN SCHEME");

  cout << "Nom du scheme a modifier : ";
  string name_utf8;
  getline(cin, name_utf8);

  auto name_u32 = unicode::utf8_to_u32(name_utf8);
  auto *entry = ht.get(name_u32);

  if (!entry) {
    print_error("Scheme introuvable.");
    wait_enter();
    return;
  }

  cout << "Pattern actuel : ";
  print_ar_u32(entry->templ, CYAN);
  cout << "\n";

  cout << "Nouveau pattern (laisser vide pour garder l'actuel) : ";
  string new_templ;
  getline(cin, new_templ);

  if (new_templ.empty()) {
    print_info("Aucune modification effectuee.");
    wait_enter();
    return;
  }

  ht.put(name_u32, unicode::utf8_to_u32(new_templ));
  print_success("Scheme modifie avec succes.");
  wait_enter();
}

// ============================================================================
// 10. SUPPRIMER UN SCHEME
// ============================================================================
void delete_scheme_cli(HashTable &ht) {
  clear_screen();
  print_header("SUPPRESSION D'UN SCHEME");

  cout << "Nom du scheme a supprimer : ";
  string name_utf8;
  getline(cin, name_utf8);

  auto name_u32 = unicode::utf8_to_u32(name_utf8);

  if (!ht.get(name_u32)) {
    print_error("Scheme introuvable.");
    wait_enter();
    return;
  }

  cout << YELLOW << "Confirmer la suppression de '";
  print_ar(name_utf8);
  cout << "' ? (o/n) : " << RESET;

  string confirm;
  getline(cin, confirm);

  if (confirm == "o" || confirm == "O") {
    ht.remove(name_u32);
    print_success("Scheme supprime.");
  } else {
    print_info("Operation annulee.");
  }
  wait_enter();
}

// ============================================================================
// 11. GENERER FAMILLE MORPHOLOGIQUE COMPLETE
// ============================================================================
void generate_family(AVLTree &avl, HashTable &ht) {
  clear_screen();
  print_header("FAMILLE MORPHOLOGIQUE COMPLETE");

  cout << "Racine (3 lettres) : ";
  string root_utf8;
  getline(cin, root_utf8);

  auto root = normalize_ar(unicode::utf8_to_u32(root_utf8));

  if (root.size() != 3) {
    print_error("Racine invalide.");
    wait_enter();
    return;
  }

  cout << "\n" << BOLD << "Famille morphologique de : ";
  print_ar_u32(root, CYAN);
  cout << RESET << "\n\n";

  auto derivatives = ht.generateAllDerivatives(root);

  if (derivatives.empty()) {
    print_info("Aucun derive genere (verifiez que des schemes existent).");
    wait_enter();
    return;
  }

  print_separator();

  for (const auto &[scheme_name, word] : derivatives) {
    string sname = unicode::u32_to_utf8(scheme_name);
    string sword = unicode::u32_to_utf8(word);

    string pattern = "";
    auto *entry = ht.get(scheme_name);
    if (entry)
      pattern = unicode::u32_to_utf8(entry->templ);

    print_ar(sname, CYAN);
    cout << " " << YELLOW << "|" << RESET << " ";
    print_ar(pattern);
    cout << " " << YELLOW << "→" << RESET << " ";
    print_ar(sword, GREEN);
    cout << "\n";

    if (avl.contains(root)) {
      avl.addDerived(root, word);
    }
  }

  print_separator();

  cout << "\n" << derivatives.size() << " mot(s) genere(s).\n";

  if (avl.contains(root)) {
    avl.incrementFrequency(root);
    print_info("Derives sauvegardes dans la racine.");
  }

  wait_enter();
}

// ============================================================================
// MINI-JEU MORPHOLOGIQUE 
// ============================================================================
void play_minigame(const vector<vector<char32_t>> &roots,
                   const vector<vector<char32_t>> &scheme_names,
                   const vector<vector<char32_t>> &scheme_templates,
                   HashTable &ht) {

  int score = 0;
  int total = 5;

  for (int q = 0; q < total; q++) {
    clear_screen();
    print_header("MINI-JEU MORPHOLOGIQUE");
    cout << CYAN << "Question " << (q + 1) << "/" << total << RESET << "\n\n";

    int root_idx = rand() % roots.size();
    int scheme_idx = rand() % scheme_templates.size();

    auto root = roots[root_idx];
    auto scheme = scheme_templates[scheme_idx];
    auto scheme_name = scheme_names[scheme_idx];

    auto derivatives = ht.generateAllDerivatives(root);

    vector<char32_t> word;
    for (const auto &[sname, w] : derivatives) {
      if (sname == scheme_name) {
        word = w;
        break;
      }
    }

    if (word.empty()) {
      q--;
      continue;
    }

    int question_type = rand() % 2;

    if (question_type == 0) {
      cout << "Mot: ";
      print_ar_u32(word);
      cout << "\n";
      cout << "Scheme: ";
      print_ar_u32(scheme_name);
      cout << "\n";
      cout << "Quelle est la racine? ";

      string answer;
      getline(cin, answer);

      auto answer_u32 = normalize_ar(unicode::utf8_to_u32(answer));

      if (answer_u32 == root) {
        cout << GREEN << "\n✓ Correct!" << RESET << "\n";
        score++;
      } else {
        cout << RED << "\n✗ Incorrect. Reponse: ";
        print_ar_u32(root);
        cout << RESET << "\n";
      }
    } else {
      cout << "Mot: ";
      print_ar_u32(word);
      cout << "\n";
      cout << "Racine: ";
      print_ar_u32(root);
      cout << "\n";
      cout << "Quel est le scheme? ";

      string answer;
      getline(cin, answer);

      auto answer_u32 = normalize_ar(unicode::utf8_to_u32(answer));

      if (answer_u32 == scheme_name) {
        cout << GREEN << "\n✓ Correct!" << RESET << "\n";
        score++;
      } else {
        cout << RED << "\n✗ Incorrect. Reponse: ";
        print_ar_u32(scheme_name);
        cout << RESET << "\n";
      }
    }

    if (q < total - 1) {
      cout << "\n"
           << YELLOW << "(Appuyez sur Entrée pour continuer...)" << RESET;
      cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
  }

  clear_screen();
  print_header("RESULTAT DU JEU");
  cout << "\n" << BOLD << "Score: " << score << "/" << total << RESET << "\n\n";

  if (score == total) {
    cout << GREEN
         << "🎉 Parfait! Excellente connaissance de la morphologie arabe!"
         << RESET << "\n";
  } else if (score >= total * 0.6) {
    cout << YELLOW << "👍 Bien joue! Continuez a pratiquer." << RESET << "\n";
  } else {
    cout << RED << "📚 Continuez a pratiquer pour ameliorer vos connaissances."
         << RESET << "\n";
  }
}

// ============================================================================
// 12. MINI-JEU MORPHOLOGIQUE
// ============================================================================
void mini_game(AVLTree &avl, HashTable &ht) {
  auto roots = avl.getAllKeys();
  vector<vector<char32_t>> scheme_names;
  vector<vector<char32_t>> scheme_templates;

  for (const auto &s : ht.allSchemes()) {
    scheme_names.push_back(s.name);
    scheme_templates.push_back(s.templ);
  }

  if (roots.size() < 2 || scheme_templates.empty()) {
    clear_screen();
    print_header("MINI-JEU MORPHOLOGIQUE");
    print_error(
        "Donnees insuffisantes pour le jeu (minimum 2 racines et 1 scheme).");
    wait_enter();
    return;
  }

  srand(time(nullptr));

  play_minigame(roots, scheme_names, scheme_templates, ht);
  wait_enter();
}

// ============================================================================
// SAUVEGARDE DES DONNÉES
// ============================================================================
void save_roots_to_file(AVLTree &avl, const string &filename) {
  ofstream file(filename);
  if (!file.is_open()) {
    print_error("Impossible d'ouvrir " + filename + " pour ecriture");
    return;
  }

  avl.forEach([&](const AVLNode *n) {
    file << unicode::u32_to_utf8(n->key);
    for (size_t i = 0; i < n->derived.size(); ++i) {
      file << "|" << unicode::u32_to_utf8(n->derived[i]);
    }
    file << "\n";
  });

  file.close();
  print_success("Racines sauvegardees dans " + filename);
}

void save_schemes_to_file(HashTable &ht, const string &filename) {
  ofstream file(filename);
  if (!file.is_open()) {
    print_error("Impossible d'ouvrir " + filename + " pour ecriture");
    return;
  }

  auto all = ht.allSchemes();
  for (const auto &s : all) {
    file << unicode::u32_to_utf8(s.name) << "|" << unicode::u32_to_utf8(s.templ)
         << "\n";
  }

  file.close();
  print_success("Schemes sauvegardes dans " + filename);
}

// ============================================================================
// PROGRAMME PRINCIPAL
// ============================================================================
int main(int argc, char *argv[]) {
  AVLTree avl;
  HashTable ht(256);

  string roots_file = "data/roots.txt";
  string schemes_file = "data/schemes.txt";

  if (argc >= 3) {
    roots_file = argv[1];
    schemes_file = argv[2];
  } else if (argc == 2) {
    roots_file = argv[1];
  }

  clear_screen();
  print_header("MOTEUR MORPHOLOGIQUE ARABE");
  cout << "\nChargement des donnees...\n";
  cout << "Racines: " << roots_file << "\n";
  cout << "Schemes: " << schemes_file << "\n\n";

  bool roots_loaded = load_roots(roots_file, avl);
  bool schemes_loaded = load_schemes(schemes_file, ht);

  if (!roots_loaded) {
    print_error("Impossible de charger le fichier des racines");
  }
  if (!schemes_loaded) {
    print_error("Impossible de charger le fichier des schemes");
  }

  int choix;
  while (true) {
    show_menu();

    if (!(cin >> choix)) {
      cin.clear();
      cin.ignore(numeric_limits<streamsize>::max(), '\n');
      continue;
    }
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    switch (choix) {
    case 1:
      generate_word(avl, ht);
      break;
    case 2:
      validate_word_cli(avl, ht);
      break;
    case 3:
      add_root_cli(avl);
      save_roots_to_file(avl, roots_file);
      break;
    case 4:
      remove_root_cli(avl);
      save_roots_to_file(avl, roots_file);
      break;
    case 5:
      list_roots_cli(avl);
      break;
    case 6:
      view_root_derivatives(avl);
      break;
    case 7:
      generate_family(avl, ht);
      break;
    case 8:
      list_schemes_cli(ht);
      break;
    case 9:
      add_scheme_cli(ht);
      save_schemes_to_file(ht, schemes_file);
      break;
    case 10:
      update_scheme_cli(ht);
      save_schemes_to_file(ht, schemes_file);
      break;
    case 11:
      delete_scheme_cli(ht);
      save_schemes_to_file(ht, schemes_file);
      break;
    case 12:
      mini_game(avl, ht);
      break;
    case 0:
      save_roots_to_file(avl, roots_file);
      save_schemes_to_file(ht, schemes_file);
      cout << "\n" << GREEN << "Au revoir !" << RESET << "\n";
      return 0;
    default:
      print_error("Choix invalide.");
      wait_enter();
    }
  }

  return 0;
}

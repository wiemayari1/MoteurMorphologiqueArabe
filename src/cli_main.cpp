#include <algorithm>
#include <chrono>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <optional>
#include <random>
#include <sstream>
#include <string>
#include <vector>

#include "AVL.h"
#include "hash_table.h"
#include "morpho.h"

using std::string;
using std::u32string;

static string read_file_utf8(const string &path) {
  std::ifstream f(path, std::ios::binary);
  if (!f)
    return "";
  std::ostringstream ss;
  ss << f.rdbuf();
  return ss.str();
}

static string json_escape(const string &s) {
  std::string out;
  out.reserve(s.size() + 8);
  for (char c : s) {
    switch (c) {
    case '\\':
      out += "\\\\";
      break;
    case '"':
      out += "\\\"";
      break;
    case '\n':
      out += "\\n";
      break;
    case '\r':
      out += "\\r";
      break;
    case '\t':
      out += "\\t";
      break;
    default:
      out += c;
      break;
    }
  }
  return out;
}

struct Args {
  string data_path;
  string schemes_path;

  bool json = false;

  bool do_generate = false;
  bool do_validate = false;
  bool do_game = false;

  string root_utf8;
  string scheme_name_utf8;
  string word_utf8;
};

static void print_help(const char *prog) {
  std::cerr << "Usage:\n"
            << "  " << prog
            << " --data data/roots.txt --schemes data/schemes.txt [--json] "
               "COMMAND\n\n"
            << "Commands:\n"
            << "  --generate --root \"كتب\" --scheme \"مفعول\"\n"
            << "  --validate --word \"مكتوب\" --root \"كتب\"\n"
            << "  --game\n\n"
            << "Options:\n"
            << "  --json           Output JSON\n"
            << "  --data <path>    Path to roots file\n"
            << "  --schemes <path> Path to schemes file\n";
}

static std::optional<string> get_opt_value(int &i, int argc, char **argv) {
  if (i + 1 >= argc)
    return std::nullopt;
  return string(argv[++i]);
}

static bool parse_args(int argc, char **argv, Args &a) {
  for (int i = 1; i < argc; ++i) {
    string arg = argv[i];

    if (arg == "--help" || arg == "-h") {
      print_help(argv[0]);
      return false;
    } else if (arg == "--json") {
      a.json = true;
    } else if (arg == "--data") {
      auto v = get_opt_value(i, argc, argv);
      if (!v)
        return false;
      a.data_path = *v;
    } else if (arg == "--schemes") {
      auto v = get_opt_value(i, argc, argv);
      if (!v)
        return false;
      a.schemes_path = *v;
    } else if (arg == "--generate") {
      a.do_generate = true;
    } else if (arg == "--validate") {
      a.do_validate = true;
    } else if (arg == "--game") {
      a.do_game = true;
    } else if (arg == "--root") {
      auto v = get_opt_value(i, argc, argv);
      if (!v)
        return false;
      a.root_utf8 = *v;
    } else if (arg == "--scheme") {
      auto v = get_opt_value(i, argc, argv);
      if (!v)
        return false;
      a.scheme_name_utf8 = *v;
    } else if (arg == "--word") {
      auto v = get_opt_value(i, argc, argv);
      if (!v)
        return false;
      a.word_utf8 = *v;
    } else {
      std::cerr << "Unknown argument: " << arg << "\n";
      return false;
    }
  }

  if (a.data_path.empty() || a.schemes_path.empty()) {
    std::cerr << "Missing --data or --schemes\n";
    return false;
  }

  int cmd_count =
      (a.do_generate ? 1 : 0) + (a.do_validate ? 1 : 0) + (a.do_game ? 1 : 0);
  if (cmd_count != 1) {
    std::cerr
        << "Choose exactly one command: --generate OR --validate OR --game\n";
    return false;
  }

  if (a.do_generate) {
    if (a.root_utf8.empty() || a.scheme_name_utf8.empty()) {
      std::cerr << "Missing --root or --scheme for --generate\n";
      return false;
    }
  }
  if (a.do_validate) {
    if (a.word_utf8.empty() || a.root_utf8.empty()) {
      std::cerr << "Missing --word or --root for --validate\n";
      return false;
    }
  }
  return true;
}

static bool load_roots_into_avl(const string &roots_path, AVLTree &tree) {
  string content = read_file_utf8(roots_path);
  if (content.empty())
    return false;

  std::istringstream in(content);
  string line;
  while (std::getline(in, line)) {
    if (line.empty())
      continue;
    auto r_u32 = normalize_ar(utf8_to_u32(line));
    if (r_u32.size() == 3)
      tree.insert(r_u32);
  }
  return true;
}

static bool load_schemes_into_hash(const string &schemes_path, HashTable &ht) {
  string content = read_file_utf8(schemes_path);
  if (content.empty())
    return false;

  std::istringstream in(content);
  string line;
  while (std::getline(in, line)) {
    if (line.empty())
      continue;
    auto pos = line.find('|');
    if (pos == string::npos)
      continue;
    string name = line.substr(0, pos);
    string templ = line.substr(pos + 1);

    ht.put(utf8_to_u32(name), utf8_to_u32(templ));
  }
  return true;
}

static int run_game(AVLTree &tree, HashTable &ht, bool json_output) {
  std::vector<u32string> roots;
  tree.getAllKeys([&](const AVLNode *n) { roots.push_back(n->key); });

  std::vector<SchemeEntry> schemes = ht.allSchemes();

  if (roots.empty() || schemes.empty()) {
    if (json_output) {
      std::cout << "{\"ok\":false,\"error\":\"no_data\"}\n";
    } else {
      std::cerr << "Erreur: pas assez de données pour le jeu\n";
    }
    return 7;
  }

  std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());
  std::uniform_int_distribution<size_t> root_dist(0, roots.size() - 1);
  std::uniform_int_distribution<size_t> scheme_dist(0, schemes.size() - 1);

  const u32string &random_root = roots[root_dist(rng)];
  const SchemeEntry &random_scheme = schemes[scheme_dist(rng)];

  u32string correct_word;
  try {
    correct_word = apply_template(random_root, random_scheme.templ);
  } catch (const std::exception &e) {
    if (json_output) {
      std::cout << "{\"ok\":false,\"error\":\"generation_failed\"}\n";
    } else {
      std::cerr << "Erreur génération: " << e.what() << "\n";
    }
    return 8;
  }

  std::vector<u32string> options;
  options.push_back(correct_word);

  int attempts = 0;
  while (options.size() < 4 && attempts < 100) {
    attempts++;
    const u32string &wrong_root = roots[root_dist(rng)];
    const SchemeEntry &wrong_scheme = schemes[scheme_dist(rng)];

    try {
      u32string wrong_word = apply_template(wrong_root, wrong_scheme.templ);

      bool exists = false;
      for (const auto &o : options) {
        if (o == wrong_word) {
          exists = true;
          break;
        }
      }
      if (!exists)
        options.push_back(wrong_word);
    } catch (...) {
      continue;
    }
  }

  while (options.size() < 4) {
    options.push_back(u32string(U"؟؟؟"));
  }

  std::shuffle(options.begin(), options.end(), rng);

  int correct_index = -1;
  for (size_t i = 0; i < options.size(); i++) {
    if (options[i] == correct_word) {
      correct_index = (int)i;
      break;
    }
  }

  if (json_output) {
    std::cout << "{\"ok\":true,"
              << "\"root\":\"" << json_escape(u32_to_utf8(random_root)) << "\","
              << "\"scheme\":\"" << json_escape(u32_to_utf8(random_scheme.name))
              << "\","
              << "\"options\":[";
    for (size_t i = 0; i < options.size(); i++) {
      if (i)
        std::cout << ",";
      std::cout << "\"" << json_escape(u32_to_utf8(options[i])) << "\"";
    }
    std::cout << "],\"correct_index\":" << correct_index << "}\n";
  } else {
    std::cout << "Jeu Morphologique\n";
    std::cout << "=================\n";
    std::cout << "Racine: " << u32_to_utf8(random_root) << "\n";
    std::cout << "Schème: " << u32_to_utf8(random_scheme.name) << "\n";
    std::cout << "Quel est le mot dérivé?\n";
    for (size_t i = 0; i < options.size(); i++) {
      std::cout << "  " << (i + 1) << ") " << u32_to_utf8(options[i]) << "\n";
    }
    std::cout << "(Réponse correcte: " << (correct_index + 1) << ")\n";
  }

  return 0;
}

// Helper to process a single JSON command
static void process_server_command(AVLTree &tree, HashTable &ht,
                                   const string &json_line) {
  // Simple manual JSON parsing for the expected format:
  // {"command": "generate", "root": "...", "scheme": "..."}
  // {"command": "validate", "word": "...", "root": "..."}
  // {"command": "game_question"}
  // {"command": "game_check", "word": "...", "root": "..."}

  auto get_val = [&](const string &key) -> string {
    string search = "\"" + key + "\":";
    size_t pos = json_line.find(search);
    if (pos == string::npos)
      return "";

    size_t start = json_line.find("\"", pos + search.length());
    if (start == string::npos)
      return "";
    start++; // skip quote

    size_t end = start;
    while (end < json_line.length()) {
      if (json_line[end] == '"' && json_line[end - 1] != '\\')
        break;
      end++;
    }

    if (end >= json_line.length())
      return "";
    return json_line.substr(start, end - start);
  };

  string cmd = get_val("command");

  if (cmd == "generate") {
    string root_utf8 = get_val("root");
    string scheme_utf8 = get_val("scheme");

    auto root_u32 = normalize_ar(utf8_to_u32(root_utf8));
    auto scheme_name_u32 = utf8_to_u32(scheme_utf8);

    SchemeEntry *se = ht.get(scheme_name_u32);
    if (!se) {
      std::cout << "{\"ok\":false,\"error\":\"scheme_not_found\"}\n"
                << std::flush;
      return;
    }

    try {
      auto word_u32 = apply_template(root_u32, se->templ);

      // SPEC COMPLIANCE: Update AVL Tree with derived word
      if (tree.contains(root_u32)) {
        tree.addDerived(root_u32, word_u32);
        tree.incrementFrequency(root_u32);
      }

      // FIXED JSON FORMAT: includes word, root, scheme
      std::cout << "{\"ok\":true,"
                << "\"word\":\"" << json_escape(u32_to_utf8(word_u32)) << "\","
                << "\"root\":\"" << json_escape(root_utf8) << "\","
                << "\"scheme\":\"" << json_escape(scheme_utf8) << "\""
                << "}\n"
                << std::flush;
    } catch (const std::exception &e) {
      std::cout << "{\"ok\":false,\"error\":\"" << json_escape(e.what())
                << "\"}\n"
                << std::flush;
    }

  } else if (cmd == "validate") {
    string word_utf8 = get_val("word");
    string root_utf8 = get_val("root");

    auto word_u32 = utf8_to_u32(word_utf8);
    auto root_u32 = normalize_ar(utf8_to_u32(root_utf8));

    bool belongs = false;
    std::vector<u32string> matched;

    for (const auto &s : ht.allSchemes()) {
      auto maybe_r = extract_root_from_word(word_u32, s.templ);
      if (maybe_r) {
        auto rn = normalize_ar(*maybe_r);
        if (rn == root_u32) {
          belongs = true;
          matched.push_back(s.name);
        }
      }
    }

    // SPEC COMPLIANCE: Update AVL Tree if validated
    if (belongs) {
      if (tree.contains(root_u32)) {
        tree.addDerived(root_u32, word_u32);
        tree.incrementFrequency(root_u32);
      }
    }

    // FIXED JSON FORMAT: ok is always true, belongs is true/false
    std::cout << "{\"ok\":true,"
              << "\"belongs\":" << (belongs ? "true" : "false") << ","
              << "\"root\":\"" << json_escape(root_utf8) << "\","
              << "\"word\":\"" << json_escape(word_utf8) << "\"";

    if (belongs) {
      std::cout << ",\"schemes\":[";
      for (size_t i = 0; i < matched.size(); ++i) {
        if (i)
          std::cout << ",";
        std::cout << "\"" << json_escape(u32_to_utf8(matched[i])) << "\"";
      }
      std::cout << "]";
    }
    std::cout << "}\n" << std::flush;

  } else if (cmd == "game_question") {
    std::vector<u32string> roots;
    tree.getAllKeys([&](const AVLNode *n) { roots.push_back(n->key); });
    auto schemes = ht.allSchemes();

    if (roots.empty() || schemes.empty()) {
      std::cout << "{\"ok\":false,\"error\":\"no_data\"}\n" << std::flush;
      return;
    }

    static std::mt19937 rng(
        std::chrono::steady_clock::now().time_since_epoch().count());
    std::uniform_int_distribution<size_t> r_dist(0, roots.size() - 1);
    std::uniform_int_distribution<size_t> s_dist(0, schemes.size() - 1);

    for (int i = 0; i < 100; ++i) {
      const auto &r = roots[r_dist(rng)];
      const auto &s = schemes[s_dist(rng)];
      try {
        auto w = apply_template(r, s.templ);

        // Generate options (random words)
        std::vector<u32string> options;
        options.push_back(w);
        while (options.size() < 4) {
          const auto &r2 = roots[r_dist(rng)];
          const auto &s2 = schemes[s_dist(rng)];
          try {
            auto w2 = apply_template(r2, s2.templ);
            bool exists = false;
            for (auto &o : options)
              if (o == w2)
                exists = true;
            if (!exists)
              options.push_back(w2);
          } catch (...) {
          }
        }
        std::shuffle(options.begin(), options.end(), rng);

        int correct_idx = -1;
        for (size_t k = 0; k < options.size(); ++k)
          if (options[k] == w)
            correct_idx = k;

        // FIXED JSON FORMAT: ok=true
        std::cout << "{\"ok\":true,"
                  << "\"root\":\"" << json_escape(u32_to_utf8(r)) << "\","
                  << "\"scheme\":\"" << json_escape(u32_to_utf8(s.name))
                  << "\","
                  << "\"options\":[";
        for (size_t k = 0; k < options.size(); ++k) {
          if (k)
            std::cout << ",";
          std::cout << "\"" << json_escape(u32_to_utf8(options[k])) << "\"";
        }
        std::cout << "],\"correct_index\":" << correct_idx << "}\n"
                  << std::flush;
        return;
      } catch (...) {
      }
    }
    std::cout << "{\"ok\":false,\"error\":\"generation_failed\"}\n"
              << std::flush;

  } else {
    std::cout << "{\"ok\":false,\"error\":\"unknown_command\"}\n" << std::flush;
  }
}

int main(int argc, char **argv) {
  bool server_mode = false;
  for (int i = 1; i < argc; ++i) {
    if (string(argv[i]) == "--server")
      server_mode = true;
  }

  Args a;
  if (!server_mode) {
    if (!parse_args(argc, argv, a)) {
      if (a.json)
        std::cout << "{\"ok\":false,\"error\":\"bad_args\"}\n";
      return 2;
    }
  } else {
    for (int i = 1; i < argc; ++i) {
      string arg = argv[i];
      if (arg == "--data") {
        auto v = get_opt_value(i, argc, argv);
        if (v)
          a.data_path = *v;
      } else if (arg == "--schemes") {
        auto v = get_opt_value(i, argc, argv);
        if (v)
          a.schemes_path = *v;
      }
    }
    if (a.data_path.empty() || a.schemes_path.empty()) {
      std::cerr << "Missing --data or --schemes for server mode\n";
      return 1;
    }
  }

  AVLTree tree;
  HashTable ht(2048);

  if (!load_roots_into_avl(a.data_path, tree)) {
    if (server_mode || a.json)
      std::cout << "{\"ok\":false,\"error\":\"cannot_read_roots\"}\n";
    else
      std::cerr << "Erreur: impossible de lire roots\n";
    return 3;
  }

  if (!load_schemes_into_hash(a.schemes_path, ht)) {
    if (server_mode || a.json)
      std::cout << "{\"ok\":false,\"error\":\"cannot_read_schemes\"}\n";
    else
      std::cerr << "Erreur: impossible de lire schemes\n";
    return 4;
  }

  if (server_mode) {
    std::cerr << "Server mode started. Waiting for JSON on stdin...\n";
    string line;
    while (std::getline(std::cin, line)) {
      if (line.empty())
        continue;
      process_server_command(tree, ht, line);
    }
    return 0;
  }

  if (a.do_generate) {
    auto root_u32 = normalize_ar(utf8_to_u32(a.root_utf8));
    auto scheme_name_u32 = utf8_to_u32(a.scheme_name_utf8);

    SchemeEntry *se = ht.get(scheme_name_u32);
    if (!se) {
      if (a.json) {
        std::cout
            << "{\"ok\":false,\"error\":\"scheme_not_found\",\"scheme\":\""
            << json_escape(a.scheme_name_utf8) << "\"}\n";
      } else {
        std::cerr << "Schème introuvable: " << a.scheme_name_utf8 << "\n";
      }
      return 5;
    }

    try {
      auto word_u32 = apply_template(root_u32, se->templ);
      string word_utf8 = u32_to_utf8(word_u32);

      if (a.json) {
        std::cout << "{\"ok\":true,\"root\":\"" << json_escape(a.root_utf8)
                  << "\",\"scheme\":\"" << json_escape(a.scheme_name_utf8)
                  << "\",\"word\":\"" << json_escape(word_utf8) << "\"}\n";
      } else {
        std::cout << "Mot généré: " << word_utf8 << "\n";
      }
      return 0;
    } catch (const std::exception &e) {
      if (a.json) {
        std::cout << "{\"ok\":false,\"error\":\"exception\",\"message\":\""
                  << json_escape(e.what()) << "\"}\n";
      } else {
        std::cerr << "Erreur: " << e.what() << "\n";
      }
      return 6;
    }
  }

  if (a.do_game) {
    return run_game(tree, ht, a.json);
  }

  {
    auto word_u32 = utf8_to_u32(a.word_utf8);
    auto root_u32 = normalize_ar(utf8_to_u32(a.root_utf8));

    bool ok = false;
    std::vector<u32string> matched;

    for (const auto &s : ht.allSchemes()) {
      auto maybe_r = extract_root_from_word(word_u32, s.templ);
      if (maybe_r) {
        auto rn = normalize_ar(*maybe_r);
        if (rn == root_u32) {
          ok = true;
          matched.push_back(s.name);
        }
      }
    }

    if (a.json) {
      std::cout << "{\"ok\":true,\"belongs\":" << (ok ? "true" : "false")
                << ",\"root\":\"" << json_escape(a.root_utf8)
                << "\",\"word\":\"" << json_escape(a.word_utf8) << "\"";

      if (ok) {
        std::cout << ",\"schemes\":[";
        for (size_t i = 0; i < matched.size(); ++i) {
          if (i)
            std::cout << ",";
          std::cout << "\"" << json_escape(u32_to_utf8(matched[i])) << "\"";
        }
        std::cout << "]";
      }

      std::cout << "}\n";
    } else {
      if (!ok)
        std::cout << "Résultat: NON\n";
      else {
        std::cout << "Résultat: OUI\nSchème(s): ";
        for (auto &n : matched)
          std::cout << u32_to_utf8(n) << " ";
        std::cout << "\n";
      }
    }

    return 0;
  }
}

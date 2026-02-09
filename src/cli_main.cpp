#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <optional>

#include "morpho.h"
#include "AVL.h"
#include "hash_table.h"

using std::string;

static string read_file_utf8(const string& path) {
    std::ifstream f(path, std::ios::binary);
    if (!f) return "";
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

static string json_escape(const string& s) {
    std::string out;
    out.reserve(s.size() + 8);
    for (char c : s) {
        switch (c) {
            case '\\': out += "\\\\"; break;
            case '"':  out += "\\\""; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default:   out += c; break;
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

    string root_utf8;
    string scheme_name_utf8;
    string word_utf8;
};

static void print_help(const char* prog) {
    std::cerr
      << "Usage:\n"
      << "  " << prog << " --data data/roots.txt --schemes data/schemes.txt [--json] COMMAND\n\n"
      << "Commands:\n"
      << "  --generate --root \"كتب\" --scheme \"مفعول\"\n"
      << "  --validate --word \"مكتوب\" --root \"كتب\"\n\n"
      << "Options:\n"
      << "  --json           Output JSON\n"
      << "  --data <path>    Path to roots file\n"
      << "  --schemes <path> Path to schemes file\n";
}

static std::optional<string> get_opt_value(int& i, int argc, char** argv) {
    if (i + 1 >= argc) return std::nullopt;
    return string(argv[++i]);
}

static bool parse_args(int argc, char** argv, Args& a) {
    for (int i = 1; i < argc; ++i) {
        string arg = argv[i];

        if (arg == "--help" || arg == "-h") {
            print_help(argv[0]);
            return false;
        } else if (arg == "--json") {
            a.json = true;
        } else if (arg == "--data") {
            auto v = get_opt_value(i, argc, argv);
            if (!v) return false;
            a.data_path = *v;
        } else if (arg == "--schemes") {
            auto v = get_opt_value(i, argc, argv);
            if (!v) return false;
            a.schemes_path = *v;
        } else if (arg == "--generate") {
            a.do_generate = true;
        } else if (arg == "--validate") {
            a.do_validate = true;
        } else if (arg == "--root") {
            auto v = get_opt_value(i, argc, argv);
            if (!v) return false;
            a.root_utf8 = *v;
        } else if (arg == "--scheme") {
            auto v = get_opt_value(i, argc, argv);
            if (!v) return false;
            a.scheme_name_utf8 = *v;
        } else if (arg == "--word") {
            auto v = get_opt_value(i, argc, argv);
            if (!v) return false;
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
    if (a.do_generate == a.do_validate) {
        std::cerr << "Choose exactly one command: --generate OR --validate\n";
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

static bool load_roots_into_avl(const string& roots_path, AVLTree& tree) {
    string content = read_file_utf8(roots_path);
    if (content.empty()) return false;

    std::istringstream in(content);
    string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        auto r_u32 = normalize_ar(utf8_to_u32(line));
        if (r_u32.size() == 3) tree.insert(r_u32);
    }
    return true;
}

static bool load_schemes_into_hash(const string& schemes_path, HashTable& ht) {
    string content = read_file_utf8(schemes_path);
    if (content.empty()) return false;

    std::istringstream in(content);
    string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        auto pos = line.find('|');
        if (pos == string::npos) continue;
        string name = line.substr(0, pos);
        string templ = line.substr(pos + 1);

        ht.put(utf8_to_u32(name), utf8_to_u32(templ));
    }
    return true;
}

int main(int argc, char** argv) {
    Args a;
    if (!parse_args(argc, argv, a)) {
        if (a.json) {
            std::cout << "{\"ok\":false,\"error\":\"bad_args\"}\n";
        }
        return 2;
    }

    AVLTree tree;
    HashTable ht(2048);

    if (!load_roots_into_avl(a.data_path, tree)) {
        if (a.json) {
            std::cout << "{\"ok\":false,\"error\":\"cannot_read_roots\"}\n";
        } else {
            std::cerr << "Erreur: impossible de lire roots: " << a.data_path << "\n";
        }
        return 3;
    }

    if (!load_schemes_into_hash(a.schemes_path, ht)) {
        if (a.json) {
            std::cout << "{\"ok\":false,\"error\":\"cannot_read_schemes\"}\n";
        } else {
            std::cerr << "Erreur: impossible de lire schemes: " << a.schemes_path << "\n";
        }
        return 4;
    }

    if (a.do_generate) {
        auto root_u32 = normalize_ar(utf8_to_u32(a.root_utf8));
        auto scheme_name_u32 = utf8_to_u32(a.scheme_name_utf8);

        SchemeEntry* se = ht.get(scheme_name_u32);
        if (!se) {
            if (a.json) {
                std::cout << "{\"ok\":false,\"error\":\"scheme_not_found\",\"scheme\":\""
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
                std::cout
                  << "{\"ok\":true,\"root\":\""   << json_escape(a.root_utf8)
                  << "\",\"scheme\":\""          << json_escape(a.scheme_name_utf8)
                  << "\",\"word\":\""            << json_escape(word_utf8)
                  << "\"}\n";
            } else {
                std::cout << "Mot généré: " << word_utf8 << "\n";
            }

            return 0;
        } catch (const std::exception& e) {
            if (a.json) {
                std::cout << "{\"ok\":false,\"error\":\"exception\",\"message\":\""
                          << json_escape(e.what()) << "\"}\n";
            } else {
                std::cerr << "Erreur: " << e.what() << "\n";
            }
            return 6;
        }
    }

    // validate
    {
        auto word_u32 = utf8_to_u32(a.word_utf8);
        auto root_u32 = normalize_ar(utf8_to_u32(a.root_utf8));

        bool ok = false;
        std::vector<std::u32string> matched;

        for (const auto& s : ht.allSchemes()) {
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
                    if (i) std::cout << ",";
                    std::cout << "\"" << json_escape(u32_to_utf8(matched[i])) << "\"";
                }
                std::cout << "]";
            }

            std::cout << "}\n";
        } else {
            if (!ok) std::cout << "Résultat: NON\n";
            else {
                std::cout << "Résultat: OUI\nSchème(s): ";
                for (auto& n : matched) std::cout << u32_to_utf8(n) << " ";
                std::cout << "\n";
            }
        }

        return 0;
    }
}

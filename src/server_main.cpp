#include "AVL.h"
#include "api_routes.h"
#include "hash_table.h"
#include "http_server.h"
#include "morpho.h"
#include "unicode_utils.h"
#include <csignal>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unistd.h>

using namespace morpho;

// Handler pour SIGINT (Ctrl+C)
static bool g_running = true;
void signalHandler(int /*signum*/) {
  std::cout << "\nArrêt du serveur...\n";
  g_running = false;
}

static std::string read_file(const std::string &path) {
  std::ifstream f(path, std::ios::binary);
  if (!f) {
    return "";
  }
  std::ostringstream ss;
  ss << f.rdbuf();
  return ss.str();
}

static std::string getExecutableDir() {
  char buffer[1024];
  ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
  if (len != -1) {
    buffer[len] = '\0';
    std::string path(buffer);
    size_t lastSlash = path.find_last_of('/');
    if (lastSlash != std::string::npos) {
      return path.substr(0, lastSlash);
    }
  }
  return ".";
}

void loadData(AVLTree &roots, HashTable &schemes) {
  std::cout << "Chargement des données...\n";

  std::string baseDir = getExecutableDir();

  std::vector<std::string> rootsPaths = {
      baseDir + "/data/roots.txt", baseDir + "/../data/roots.txt",
      "./data/roots.txt", "../data/roots.txt", "data/roots.txt"};

  bool rootsLoaded = false;
  for (const auto &path : rootsPaths) {
    std::string rootsContent = read_file(path);
    if (!rootsContent.empty()) {
      std::istringstream in(rootsContent);
      std::string line;
      int count = 0;
      while (std::getline(in, line)) {
        if (line.empty())
          continue;
        if (!line.empty() && line.back() == '\r')
          line.pop_back();
        if (line[0] == '#')
          continue;

        auto u32 = normalize_ar(unicode::utf8_to_u32(line));
        if (u32.size() == 3) {
          roots.insert(u32);
          count++;
        }
      }
      std::cout << "  " << count << " racines chargées\n";
      rootsLoaded = true;
      break;
    }
  }

  if (!rootsLoaded) {
    std::cerr << "  ERREUR: Impossible de charger roots.txt\n";
  }

  std::vector<std::string> schemesPaths = {
      baseDir + "/data/schemes.txt", baseDir + "/../data/schemes.txt",
      "./data/schemes.txt", "../data/schemes.txt", "data/schemes.txt"};

  bool schemesLoaded = false;
  for (const auto &path : schemesPaths) {
    std::string schemesContent = read_file(path);
    if (!schemesContent.empty()) {
      std::istringstream in(schemesContent);
      std::string line;
      int count = 0;
      while (std::getline(in, line)) {
        if (line.empty())
          continue;
        if (!line.empty() && line.back() == '\r')
          line.pop_back();
        if (line[0] == '#')
          continue;

        size_t sep = line.find('|');
        if (sep == std::string::npos)
          continue;

        std::string name = line.substr(0, sep);
        std::string templ = line.substr(sep + 1);
        schemes.put(unicode::utf8_to_u32(name), unicode::utf8_to_u32(templ));
        count++;
      }
      std::cout << "  " << count << " schèmes chargés\n";
      schemesLoaded = true;
      break;
    }
  }

  if (!schemesLoaded) {
    std::cerr << "  ERREUR: Impossible de charger schemes.txt\n";
  }
}

int main(int argc, char **argv) {
  // Gestion des signaux
  std::signal(SIGINT, signalHandler);

  // Port par défaut ou depuis les arguments
  int port = 8080;
  if (argc > 1) {
    port = std::stoi(argv[1]);
  }

  std::cout << "========================================\n";
  std::cout << "  Moteur Morphologique Arabe - API\n";
  std::cout << "  Port: " << port << "\n";
  std::cout << "========================================\n\n";

  AVLTree roots;
  HashTable schemes(256);

  loadData(roots, schemes);

  // Vérification
  int verifyCount = 0;
  roots.forEach([&](const AVLNode *) { verifyCount++; });
  std::cout << "\nTotal: " << verifyCount << " racines, "
            << schemes.allSchemes().size() << " schèmes\n\n";

  if (verifyCount == 0) {
    std::cerr << "⚠️  Aucune racine chargée!\n\n";
  }

  HttpServer server(port);
  registerRoutes(server, roots, schemes);

  std::cout << "Démarrage sur http://localhost:" << port << "\n";
  std::cout << "Appuyez sur Ctrl+C pour arrêter\n\n";

  server.start();

  return 0;
}
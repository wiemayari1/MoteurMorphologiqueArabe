#include <iostream>
#include <fstream>
#include <memory>
#include "http_server.h"
#include "api_routes.h"
#include "AVL.h"
#include "hash_table.h"
#include "morpho.h"

using namespace morpho;

void loadData(AVLTree& roots, HashTable& schemes) {
    std::cout << "📂 Chargement des données...\n";
    
    // Obtenir le chemin absolu
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    std::cout << "   Répertoire courant: " << cwd << "\n";
    
    // Charger racines
    std::ifstream rootsFile("data/roots.txt");
    if (!rootsFile.is_open()) {
        std::cerr << "   ❌ Impossible d'ouvrir data/roots.txt\n";
        // Essayer avec chemin absolu
        rootsFile.open(std::string(cwd) + "/data/roots.txt");
    }
    
    if (rootsFile.is_open()) {
        std::cout << "   ✅ Fichier roots.txt ouvert\n";
        std::string line;
        int count = 0;
        while (std::getline(rootsFile, line)) {
            if (line.empty() || line[0] == '#') continue;
            std::cout << "   Ligne lue: [" << line << "]\n";
            auto u32line = normalize_ar(utf8_to_u32(line));
            std::cout << "   Après normalisation: " << u32line.length() << " caractères\n";
            if (u32line.length() == 3) {
                roots.insert(u32line);
                std::cout << "   📌 Racine chargée: " << line << "\n";
                count++;
            } else {
                std::cout << "   ⚠️  Ignoré (pas 3 caractères): " << line << "\n";
            }
        }
        rootsFile.close();
        std::cout << "   Total racines: " << count << "\n";
    } else {
        std::cerr << "   ❌ Échec ouverture roots.txt\n";
    }
    
    // Charger schèmes
    std::ifstream schemesFile("data/schemes.txt");
    if (!schemesFile.is_open()) {
        schemesFile.open(std::string(cwd) + "/data/schemes.txt");
    }
    
    if (schemesFile.is_open()) {
        std::cout << "   ✅ Fichier schemes.txt ouvert\n";
        std::string line;
        int count = 0;
        while (std::getline(schemesFile, line)) {
            if (line.empty() || line[0] == '#') continue;
            size_t sep = line.find('|');
            if (sep != std::string::npos) {
                std::string name = line.substr(0, sep);
                std::string templ = line.substr(sep + 1);
                schemes.put(normalize_ar(utf8_to_u32(name)), 
                           normalize_ar(utf8_to_u32(templ)));
                std::cout << "   ⚖️  Schème chargé: " << name << "\n";
                count++;
            }
        }
        schemesFile.close();
        std::cout << "   Total schèmes: " << count << "\n";
    } else {
        std::cerr << "   ❌ Échec ouverture schemes.txt\n";
    }
}

int main(int, char**) {
    std::cout << "========================================\n";
    std::cout << "  🔧 Moteur Morphologique Arabe - API\n";
    std::cout << "========================================\n\n";
    
    AVLTree roots;
    HashTable schemes(100);
    
    loadData(roots, schemes);
    
    std::cout << "\n📊 Structures initialisées:\n";
    std::cout << "   🌳 AVL Tree (Racines): Recherche O(log n)\n";
    std::cout << "   🔀 Hash Table (Schèmes): Accès O(1)\n\n";
    
    HttpServer server(8080);
    registerRoutes(server, roots, schemes);
    
    std::cout << "🚀 Serveur prêt sur http://localhost:8080\n\n";
    
    server.start();
    
    return 0;
}

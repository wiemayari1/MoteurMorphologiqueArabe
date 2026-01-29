// include/HashTable.hpp
#include <string>

struct Scheme {
    std::string nom;      // ex: "مفعول" [cite: 71]
    std::string regle;    // Représentation abstraite [cite: 73]
};

class HashTable {
private:
    Scheme* table[100]; // Implémentation manuelle demandée [cite: 32]
public:
    int hashFunction(std::string key);
    void insert(std::string nom, std::string regle); // [cite: 34]
    std::string getRegle(std::string nom);           // Accès direct [cite: 33]
};

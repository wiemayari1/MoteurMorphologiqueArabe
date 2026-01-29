// include/Generator.hpp
class MorphoEngine {
public:
    // Génère "مكتوب" à partir de "كتب" et "مفعول" [cite: 89, 90, 91]
    std::string generate(std::string racine, std::string schemeNom);
    
    // Vérifie si "مكتوب" appartient à "كتب" [cite: 94, 96]
    bool validate(std::string mot, std::string racine); 
};

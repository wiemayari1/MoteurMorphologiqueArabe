#include "api_routes.h"
#include "morpho.h"
#include <sstream>
#include <iomanip>
#include <ctime>
#include <random>
#include <algorithm>
#include <set>

namespace morpho {

// ===== HELPERS JSON =====

std::string jsonEscape(const std::string& s) {
    std::ostringstream o;
    for (auto c : s) {
        switch (c) {
            case '"': o << "\\\""; break;
            case '\\': o << "\\\\"; break;
            case '\b': o << "\\b"; break;
            case '\f': o << "\\f"; break;
            case '\n': o << "\\n"; break;
            case '\r': o << "\\r"; break;
            case '\t': o << "\\t"; break;
            default: o << c;
        }
    }
    return o.str();
}

std::string jsonObject(const std::vector<std::pair<std::string, std::string>>& fields) {
    std::ostringstream json;
    json << "{";
    for (size_t i = 0; i < fields.size(); i++) {
        if (i > 0) json << ",";
        json << "\"" << fields[i].first << "\":\"" << jsonEscape(fields[i].second) << "\"";
    }
    json << "}";
    return json.str();
}

std::string jsonObjectMixed(const std::vector<std::pair<std::string, std::string>>& fields, 
                            const std::vector<std::string>& rawFields) {
    std::ostringstream json;
    json << "{";
    for (size_t i = 0; i < fields.size(); i++) {
        if (i > 0) json << ",";
        bool isRaw = (std::find(rawFields.begin(), rawFields.end(), fields[i].first) != rawFields.end());
        json << "\"" << fields[i].first << "\":";
        if (isRaw) {
            json << fields[i].second;
        } else {
            json << "\"" << jsonEscape(fields[i].second) << "\"";
        }
    }
    json << "}";
    return json.str();
}

std::string jsonArray(const std::vector<std::string>& items) {
    if (items.empty()) return "[]";
    std::ostringstream json;
    json << "[";
    for (size_t i = 0; i < items.size(); i++) {
        if (i > 0) json << ",";
        json << items[i];
    }
    json << "]";
    return json;
}

std::string successResponse(const std::string& data) {
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    std::ostringstream ts;
    ts << std::put_time(&tm, "%Y-%m-%dT%H:%M:%S");
    
    return "{\"success\":true,\"data\":" + data + 
           ",\"timestamp\":\"" + ts.str() + "\"}";
}

std::string errorResponse(const std::string& message) {
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    std::ostringstream ts;
    ts << std::put_time(&tm, "%Y-%m-%dT%H:%M:%S");
    
    return "{\"success\":false,\"error\":\"" + jsonEscape(message) + 
           "\",\"timestamp\":\"" + ts.str() + "\"}";
}

// ===== PARSE JSON SIMPLE =====

std::map<std::string, std::string> parseJson(const std::string& json) {
    std::map<std::string, std::string> result;
    size_t pos = 0;
    
    while ((pos = json.find('"', pos)) != std::string::npos) {
        size_t key_start = pos + 1;
        size_t key_end = json.find('"', key_start);
        if (key_end == std::string::npos) break;
        
        std::string key = json.substr(key_start, key_end - key_start);
        
        size_t colon = json.find(':', key_end);
        if (colon == std::string::npos) break;
        
        size_t val_start = json.find_first_not_of(" \t\n\r", colon + 1);
        if (val_start == std::string::npos) break;
        
        std::string value;
        if (json[val_start] == '"') {
            size_t val_end = json.find('"', val_start + 1);
            value = json.substr(val_start + 1, val_end - val_start - 1);
            pos = val_end + 1;
        } else if (json[val_start] == '[') {
            size_t val_end = val_start;
            int bracketCount = 1;
            val_end++;
            while (val_end < json.size() && bracketCount > 0) {
                if (json[val_end] == '[') bracketCount++;
                else if (json[val_end] == ']') bracketCount--;
                val_end++;
            }
            value = json.substr(val_start, val_end - val_start);
            pos = val_end;
        } else {
            size_t val_end = json.find_first_of(",}", val_start);
            value = json.substr(val_start, val_end - val_start);
            pos = val_end;
        }
        
        result[key] = value;
    }
    
    return result;
}

std::vector<std::string> parseJsonArray(const std::string& arr) {
    std::vector<std::string> result;
    if (arr.empty() || arr == "[]") return result;
    
    size_t start = arr.find('[');
    size_t end = arr.rfind(']');
    if (start == std::string::npos || end == std::string::npos) return result;
    
    std::string content = arr.substr(start + 1, end - start - 1);
    size_t pos = 0;
    
    while (pos < content.size()) {
        pos = content.find('"', pos);
        if (pos == std::string::npos) break;
        
        size_t str_start = pos + 1;
        size_t str_end = content.find('"', str_start);
        if (str_end == std::string::npos) break;
        
        result.push_back(content.substr(str_start, str_end - str_start));
        pos = str_end + 1;
    }
    
    return result;
}

static int g_nextId = 1;

// ===== ROUTES =====

void registerRoutes(HttpServer& server, AVLTree& roots, HashTable& schemes) {

    // ===== RACINES =====
    
    server.get("/api/roots", [&roots](const std::string&, const std::string&,
                                      const std::string&, const std::map<std::string, std::string>&) {
        std::vector<std::string> items;
        int id = 1;
        
        roots.forEach([&](const AVLNode* node) {
            std::string key_utf8 = u32_to_utf8(node->key);
            std::string letters;
            for (auto c : node->key) {
                if (!letters.empty()) letters += "-";
                letters += u32_to_utf8(std::u32string(1, c));
            }
            
            items.push_back(jsonObject({
                {"id", std::to_string(id++)},
                {"value", key_utf8},
                {"letters", letters},
                {"frequency", std::to_string(node->frequency)},
                {"derived_count", std::to_string(node->derived.size())}
            }));
        });
        
        return successResponse(jsonArray(items));
    });
    
    server.post("/api/roots", [&roots](const std::string&, const std::string&,
                                       const std::string& body, const std::map<std::string, std::string>&) {
        auto data = parseJson(body);
        std::string value = data["value"];
        
        std::u32string u32value = utf8_to_u32(value);
        u32value = normalize_ar(u32value);
        
        if (!isValidArabicRoot(u32value)) {
            return errorResponse("الجذر يجب أن يكون 3 أحرف عربية فقط");
        }
        
        if (roots.contains(u32value)) {
            return errorResponse("هذا الجذر موجود مسبقاً");
        }
        
        roots.insert(u32value);
        
        std::string letters;
        for (auto c : u32value) {
            if (!letters.empty()) letters += "-";
            letters += u32_to_utf8(std::u32string(1, c));
        }
        
        return successResponse(jsonObject({
            {"id", std::to_string(g_nextId++)},
            {"value", u32_to_utf8(u32value)},
            {"letters", letters},
            {"message", "تم إضافة الجذر بنجاح"}
        }));
    });
    
    server.del("/api/roots/:value", [&roots](const std::string&, const std::string& path,
                                          const std::string&, const std::map<std::string, std::string>&) {
        size_t last_slash = path.rfind('/');
        std::string encoded_value = path.substr(last_slash + 1);
        
        std::string value;
        for (size_t i = 0; i < encoded_value.size(); i++) {
            if (encoded_value[i] == '%' && i + 2 < encoded_value.size()) {
                int hex = std::stoi(encoded_value.substr(i + 1, 2), nullptr, 16);
                value += static_cast<char>(hex);
                i += 2;
            } else if (encoded_value[i] == '+') {
                value += ' ';
            } else {
                value += encoded_value[i];
            }
        }
        
        std::u32string u32value = utf8_to_u32(value);
        u32value = normalize_ar(u32value);
        
        if (!roots.contains(u32value)) {
            return errorResponse("الجذر غير موجود");
        }
        
        roots.remove(u32value);
        
        return successResponse(jsonObject({
            {"message", "تم حذف الجذر بنجاح"},
            {"value", value}
        }));
    });
    
    // ===== SCHÈMES =====
    
    server.get("/api/schemes", [&schemes](const std::string&, const std::string&,
                                          const std::string&, const std::map<std::string, std::string>&) {
        auto all = schemes.allSchemes();
        std::vector<std::string> items;
        int id = 1;
        
        for (const auto& entry : all) {
            items.push_back(jsonObject({
                {"id", std::to_string(id++)},
                {"name", u32_to_utf8(entry.name)},
                {"pattern", u32_to_utf8(entry.templ)},
                {"rule", u32_to_utf8(entry.rule.description)},
                {"rule_pattern", u32_to_utf8(entry.rule.pattern)}
            }));
        }
        
        return successResponse(jsonArray(items));
    });
    
    server.post("/api/schemes", [&schemes](const std::string&, const std::string&,
                                           const std::string& body, const std::map<std::string, std::string>&) {
        auto data = parseJson(body);
        
        std::u32string name = normalize_ar(utf8_to_u32(data["name"]));
        std::u32string pattern = normalize_ar(utf8_to_u32(data["pattern"]));
        std::string description = data.count("description") ? data["description"] : "";
        
        if (!isValidArabic(name)) {
            return errorResponse("اسم الوزن يجب أن يكون بالعربية");
        }
        
        if (name.empty() || pattern.empty()) {
            return errorResponse("الرجاء إدخال الاسم والنمط");
        }
        
        schemes.put(name, pattern, description);
        
        return successResponse(jsonObject({
            {"id", std::to_string(g_nextId++)},
            {"name", u32_to_utf8(name)},
            {"pattern", u32_to_utf8(pattern)},
            {"rule", description.empty() ? u32_to_utf8(pattern) : description}
        }));
    });
    
    server.del("/api/schemes/:name", [&schemes](const std::string&, const std::string& path,
                                                const std::string&, const std::map<std::string, std::string>&) {
        size_t last_slash = path.rfind('/');
        std::string encoded_name = path.substr(last_slash + 1);
        
        std::string name;
        for (size_t i = 0; i < encoded_name.size(); i++) {
            if (encoded_name[i] == '%' && i + 2 < encoded_name.size()) {
                int hex = std::stoi(encoded_name.substr(i + 1, 2), nullptr, 16);
                name += static_cast<char>(hex);
                i += 2;
            } else if (encoded_name[i] == '+') {
                name += ' ';
            } else {
                name += encoded_name[i];
            }
        }
        
        std::u32string u32name = normalize_ar(utf8_to_u32(name));
        
        auto* entry = schemes.get(u32name);
        if (!entry) {
            return errorResponse("الوزن غير موجود");
        }
        
        schemes.remove(u32name);
        
        return successResponse(jsonObject({
            {"message", "تم حذف الوزن بنجاح"},
            {"name", name}
        }));
    });
    
    server.put("/api/schemes/:name", [&schemes](const std::string&, const std::string& path,
                                                const std::string& body, const std::map<std::string, std::string>&) {
        size_t last_slash = path.rfind('/');
        std::string encoded_name = path.substr(last_slash + 1);
        
        std::string name;
        for (size_t i = 0; i < encoded_name.size(); i++) {
            if (encoded_name[i] == '%' && i + 2 < encoded_name.size()) {
                int hex = std::stoi(encoded_name.substr(i + 1, 2), nullptr, 16);
                name += static_cast<char>(hex);
                i += 2;
            } else if (encoded_name[i] == '+') {
                name += ' ';
            } else {
                name += encoded_name[i];
            }
        }
        
        std::u32string u32name = normalize_ar(utf8_to_u32(name));
        
        auto* entry = schemes.get(u32name);
        if (!entry) {
            return errorResponse("الوزن غير موجود");
        }
        
        auto data = parseJson(body);
        
        std::u32string newPattern = entry->templ;
        std::string newDescription = u32_to_utf8(entry->rule.description);
        
        if (data.count("pattern") && !data["pattern"].empty()) {
            newPattern = normalize_ar(utf8_to_u32(data["pattern"]));
        }
        if (data.count("description") && !data["description"].empty()) {
            newDescription = data["description"];
        }
        
        schemes.put(u32name, newPattern, newDescription);
        
        return successResponse(jsonObject({
            {"message", "تم تحديث الوزن بنجاح"},
            {"name", name},
            {"pattern", u32_to_utf8(newPattern)}
        }));
    });
    
    // ===== VALIDATION (CORRIGÉE) =====
    
    server.post("/api/validate", [&roots, &schemes](const std::string&, const std::string&,
                                                    const std::string& body, const std::map<std::string, std::string>&) {
        auto data = parseJson(body);
        std::u32string word = normalize_ar(utf8_to_u32(data["word"]));
        std::u32string root = normalize_ar(utf8_to_u32(data["root"]));
        
        if (!isValidArabic(word)) {
            return errorResponse("الكلمة يجب أن تكون بالعربية فقط");
        }
        if (!isValidArabicRoot(root)) {
            return errorResponse("الجذر يجب أن يكون 3 أحرف عربية");
        }
        
        // CORRECTION: Vérifier si la racine existe dans la base
        if (!roots.contains(root)) {
            return successResponse(jsonObject({
                {"valid", "false"},
                {"word", u32_to_utf8(word)},
                {"root", u32_to_utf8(root)},
                {"message", "الجذر غير موجود في القاعدة"},
                {"complexity", "O(log n)"}
            }));
        }
        
        // CORRECTION: Tester tous les schèmes pour voir si le mot correspond à la racine donnée
        auto allSchemes = schemes.allSchemes();
        std::string matchedScheme;
        std::string matchedPattern;
        bool valid = false;
        
        for (const auto& scheme : allSchemes) {
            auto extracted = extract_root_from_word(word, scheme.templ);
            if (extracted && *extracted == root) {
                valid = true;
                matchedScheme = u32_to_utf8(scheme.name);
                matchedPattern = u32_to_utf8(scheme.templ);
                break;
            }
        }
        
        std::vector<std::pair<std::string, std::string>> fields = {
            {"valid", valid ? "true" : "false"},
            {"word", u32_to_utf8(word)},
            {"root", u32_to_utf8(root)},
            {"message", valid ? "الكلمة صحيحة" : "الكلمة لا تنتمي لهذا الجذر"},
            {"complexity", "O(log n + m) حيث m = عدد الأوزان"}
        };
        
        if (valid) {
            fields.push_back({"scheme_name", matchedScheme});
            fields.push_back({"scheme_pattern", matchedPattern});
            roots.incrementFrequency(root);
        }
        
        return successResponse(jsonObject(fields));
    });
    
    // ===== GÉNÉRATION =====
    
    server.post("/api/generate", [&roots, &schemes](const std::string&, const std::string&,
                                                    const std::string& body, const std::map<std::string, std::string>&) {
        auto data = parseJson(body);
        std::u32string root = normalize_ar(utf8_to_u32(data["root"]));
        
        if (!isValidArabicRoot(root)) {
            return errorResponse("الرجاء إدخال جذر عربي من 3 أحرف");
        }
        
        if (!roots.contains(root)) {
            return errorResponse("الجذر غير موجود في القاعدة");
        }
        
        std::vector<std::pair<std::u32string, std::u32string>> selectedSchemes;
        
        if (data.count("schemes") && !data["schemes"].empty()) {
            std::vector<std::string> schemeNames = parseJsonArray(data["schemes"]);
            
            for (const auto& name : schemeNames) {
                auto* entry = schemes.get(normalize_ar(utf8_to_u32(name)));
                if (entry) {
                    selectedSchemes.push_back({entry->name, entry->templ});
                }
            }
        }
        
        if (selectedSchemes.empty()) {
            auto all = schemes.allSchemes();
            for (const auto& s : all) {
                selectedSchemes.push_back({s.name, s.templ});
            }
        }
        
        std::vector<std::string> results;
        std::vector<std::u32string> generatedWords;
        
        for (const auto& [name, templ] : selectedSchemes) {
            try {
                std::u32string generated = apply_template(root, templ);
                generatedWords.push_back(generated);
                
                results.push_back(jsonObject({
                    {"root", u32_to_utf8(root)},
                    {"scheme_name", u32_to_utf8(name)},
                    {"scheme_pattern", u32_to_utf8(templ)},
                    {"result", u32_to_utf8(generated)}
                }));
                
                roots.addDerived(root, generated);
            } catch (...) {
                continue;
            }
        }
        
        roots.incrementFrequency(root);
        
        std::string familyList = "[";
        for (size_t i = 0; i < generatedWords.size(); i++) {
            if (i > 0) familyList += ",";
            familyList += "\"" + jsonEscape(u32_to_utf8(generatedWords[i])) + "\"";
        }
        familyList += "]";
        
        return successResponse("{\"derivatives\":" + jsonArray(results) + 
                              ",\"family\":" + familyList + 
                              ",\"root\":\"" + jsonEscape(u32_to_utf8(root)) + "\"}");
    });
    
    // ===== JEU (CORRIGÉ) =====
    
    static std::vector<GameQuestion> currentGameQuestions;
    static std::map<int, std::string> correctAnswers;
    
    server.get("/api/game/start", [&roots, &schemes](const std::string&, const std::string&,
                                                     const std::string&, const std::map<std::string, std::string>&) {
        std::vector<std::u32string> allRoots;
        roots.forEach([&](const AVLNode* node) {
            allRoots.push_back(node->key);
        });
        
        auto allSchemes = schemes.allSchemes();
        std::vector<std::u32string> schemeNames;
        std::vector<std::u32string> schemeTemplates;
        
        for (const auto& s : allSchemes) {
            schemeNames.push_back(s.name);
            schemeTemplates.push_back(s.templ);
        }
        
        if (allRoots.size() < 4 || schemeNames.empty()) {
            return errorResponse("لا يوجد بيانات كافية للعبة. أضف جذور وأوزان أولاً.");
        }
        
        // CORRECTION: Générer 10 questions valides
        currentGameQuestions.clear();
        correctAnswers.clear();
        
        static std::random_device rd;
        static std::mt19937 gen(rd());
        
        int attempts = 0;
        while (currentGameQuestions.size() < 10 && attempts < 100) {
            attempts++;
            auto q = generate_game_question(allRoots, schemeNames, schemeTemplates);
            q.id = currentGameQuestions.size() + 1;
            
            // Vérifier que la question est valide
            if (!q.word.empty() && !q.correct_answer.empty() && q.options.size() >= 2) {
                currentGameQuestions.push_back(q);
                correctAnswers[q.id] = u32_to_utf8(q.correct_answer);
            }
        }
        
        if (currentGameQuestions.size() < 4) {
            return errorResponse("لا يمكن إنشاء أسئلة كافية. تحقق من البيانات.");
        }
        
        // Sélectionner 4 questions aléatoires
        std::shuffle(currentGameQuestions.begin(), currentGameQuestions.end(), gen);
        
        std::vector<std::string> selectedQuestions;
        for (int i = 0; i < 4 && i < (int)currentGameQuestions.size(); i++) {
            const auto& q = currentGameQuestions[i];
            
            std::vector<std::string> optStr;
            for (const auto& opt : q.options) {
                optStr.push_back(u32_to_utf8(opt));
            }
            
            std::vector<std::pair<std::string, std::string>> fields = {
                {"id", std::to_string(q.id)},
                {"type", q.type},
                {"word", u32_to_utf8(q.word)},
                {"root", u32_to_utf8(q.root)},
                {"scheme_name", u32_to_utf8(q.scheme_name)},
                {"difficulty", q.difficulty},
                {"options", jsonArray(optStr)}
            };
            
            selectedQuestions.push_back(jsonObjectMixed(fields, {"id", "options"}));
        }
        
        return successResponse("{\"questions\":" + jsonArray(selectedQuestions) + 
                              ",\"total\":" + std::to_string(selectedQuestions.size()) + 
                              ",\"pool\":" + std::to_string(currentGameQuestions.size()) + "}");
    });
    
    server.post("/api/game/answer", [](const std::string&, const std::string&,
                                       const std::string& body, const std::map<std::string, std::string>&) {
        auto data = parseJson(body);
        
        if (!data.count("questionId") || !data.count("answer")) {
            return errorResponse("معرف السؤال أو الإجابة مفقود");
        }
        
        int questionId = std::stoi(data["questionId"]);
        std::string answer = data["answer"];
        
        bool correct = false;
        std::string correctAnswerStr = "";
        
        auto it = correctAnswers.find(questionId);
        if (it != correctAnswers.end()) {
            correctAnswerStr = it->second;
            correct = (answer == correctAnswerStr);
        }
        
        return successResponse(jsonObject({
            {"correct", correct ? "true" : "false"},
            {"correctAnswer", correctAnswerStr}
        }));
    });
    
    server.get("/api", [](const std::string&, const std::string&, const std::string&, 
                          const std::map<std::string, std::string>&) {
        return successResponse(jsonObject({
            {"message", "Moteur Morphologique Arabe API"},
            {"version", "2.1.0"},
            {"features", "Validation corrigée, Jeu fonctionnel, Génération morphologique"}
        }));
    });
}

} // namespace morpho

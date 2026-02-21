#include "api_routes.h"
#include "morpho.h"
#include "unicode_utils.h"
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace morpho {

// ============================================================================
// MAP MANUELLE (remplace std::map)
// ============================================================================
struct MapNode {
  std::string key;
  std::string value;
  MapNode *next;
  MapNode(const std::string &k, const std::string &v)
      : key(k), value(v), next(nullptr) {}
};

class SimpleMap {
  MapNode *head;

public:
  SimpleMap() : head(nullptr) {}
  ~SimpleMap() {
    while (head) {
      MapNode *tmp = head;
      head = head->next;
      delete tmp;
    }
  }

  void put(const std::string &k, const std::string &v) {
    MapNode *curr = head;
    while (curr) {
      if (curr->key == k) {
        curr->value = v;
        return;
      }
      curr = curr->next;
    }
    MapNode *n = new MapNode(k, v);
    n->next = head;
    head = n;
  }

  std::string *get(const std::string &k) {
    MapNode *curr = head;
    while (curr) {
      if (curr->key == k)
        return &(curr->value);
      curr = curr->next;
    }
    return nullptr;
  }

  bool contains(const std::string &k) { return get(k) != nullptr; }
};

// ============================================================================
// URL DECODE SÉCURISÉ
// ============================================================================
static std::string urlDecode(const std::string &str) {
  std::string result;
  for (size_t i = 0; i < str.size(); i++) {
    if (str[i] == '%' && i + 2 < str.size()) {
      char hex[3] = {str[i + 1], str[i + 2], '\0'};
      char *end = nullptr;
      long val = std::strtol(hex, &end, 16);
      if (end == hex + 2 && val >= 0 && val <= 255) {
        result += static_cast<char>(val);
        i += 2;
      } else {
        result += str[i];
      }
    } else if (str[i] == '+') {
      result += ' ';
    } else {
      result += str[i];
    }
  }
  return result;
}

// ============================================================================
// JSON HELPERS
// ============================================================================
std::string jsonEscape(const std::string &s) {
  std::string result;
  for (size_t i = 0; i < s.size(); i++) {
    char c = s[i];
    switch (c) {
    case '\"':
      result += "\\\"";
      break;
    case '\\':
      result += "\\\\";
      break;
    case '\b':
      result += "\\b";
      break;
    case '\f':
      result += "\\f";
      break;
    case '\n':
      result += "\\n";
      break;
    case '\r':
      result += "\\r";
      break;
    case '\t':
      result += "\\t";
      break;
    default:
      result += c;
    }
  }
  return result;
}

std::string
jsonObject(const std::vector<std::pair<std::string, std::string>> &fields) {
  std::string json = "{";
  for (size_t i = 0; i < fields.size(); i++) {
    if (i > 0)
      json += ",";
    json +=
        "\"" + fields[i].first + "\":\"" + jsonEscape(fields[i].second) + "\"";
  }
  json += "}";
  return json;
}

std::string jsonArray(const std::vector<std::string> &items) {
  if (items.empty())
    return "[]";
  std::string json = "[";
  for (size_t i = 0; i < items.size(); i++) {
    if (i > 0)
      json += ",";
    json += items[i];
  }
  json += "]";
  return json;
}

std::string successResponse(const std::string &data) {
  auto t = std::time(nullptr);
  auto tm = *std::localtime(&t);
  std::ostringstream ts;
  ts << std::put_time(&tm, "%Y-%m-%dT%H:%M:%S");
  return "{\"success\":true,\"data\":" + data + ",\"timestamp\":\"" + ts.str() +
         "\"}";
}

std::string errorResponse(const std::string &message) {
  auto t = std::time(nullptr);
  auto tm = *std::localtime(&t);
  std::ostringstream ts;
  ts << std::put_time(&tm, "%Y-%m-%dT%H:%M:%S");
  return "{\"success\":false,\"error\":\"" + jsonEscape(message) +
         "\",\"timestamp\":\"" + ts.str() + "\"}";
}

// ============================================================================
// PARSE JSON MANUEL
// ============================================================================
SimpleMap parseJson(const std::string &json) {
  SimpleMap result;
  size_t pos = 0;

  while ((pos = json.find('\"', pos)) != std::string::npos) {
    size_t key_start = pos + 1;
    size_t key_end = json.find('\"', key_start);
    if (key_end == std::string::npos)
      break;

    std::string key = json.substr(key_start, key_end - key_start);
    size_t colon = json.find(':', key_end);
    if (colon == std::string::npos)
      break;

    size_t val_start = json.find_first_not_of(" \t\n\r", colon + 1);
    if (val_start == std::string::npos)
      break;

    std::string value;
    if (json[val_start] == '\"') {
      size_t val_end = json.find('\"', val_start + 1);
      value = json.substr(val_start + 1, val_end - val_start - 1);
      pos = val_end + 1;
    } else if (json[val_start] == '[') {
      size_t val_end = val_start;
      int bracketCount = 1;
      val_end++;
      while (val_end < json.size() && bracketCount > 0) {
        if (json[val_end] == '[')
          bracketCount++;
        else if (json[val_end] == ']')
          bracketCount--;
        val_end++;
      }
      value = json.substr(val_start, val_end - val_start);
      pos = val_end;
    } else {
      size_t val_end = json.find_first_of(",}\"", val_start);
      value = json.substr(val_start, val_end - val_start);
      pos = val_end;
    }

    result.put(key, value);
  }

  return result;
}

std::vector<std::string> parseJsonArray(const std::string &arr) {
  std::vector<std::string> result;
  if (arr.empty() || arr == "[]")
    return result;

  size_t start = arr.find('[');
  size_t end = arr.rfind(']');
  if (start == std::string::npos || end == std::string::npos)
    return result;

  std::string content = arr.substr(start + 1, end - start - 1);
  size_t pos = 0;

  while (pos < content.size()) {
    pos = content.find('\"', pos);
    if (pos == std::string::npos)
      break;

    size_t str_start = pos + 1;
    size_t str_end = content.find('\"', str_start);
    if (str_end == std::string::npos)
      break;

    result.push_back(content.substr(str_start, str_end - str_start));
    pos = str_end + 1;
  }

  return result;
}

// ============================================================================
// ROUTES
// ============================================================================
static int g_nextId = 1;

void registerRoutes(HttpServer &server, AVLTree &roots, HashTable &schemes) {

  // ----- RACINES - GET ALL -----
  server.get("/api/roots", [&roots](
                               const std::string &, const std::string &,
                               const std::string &,
                               const std::map<std::string, std::string> &) {
    std::vector<std::string> items;
    int id = 1;

    roots.forEach([&](const AVLNode *node) {
      std::string key_utf8 = unicode::u32_to_utf8(node->key);
      std::string letters;
      for (size_t i = 0; i < node->key.size(); i++) {
        if (i > 0)
          letters += "-";
        letters += unicode::u32_to_utf8(std::vector<char32_t>{node->key[i]});
      }

      items.push_back(jsonObject(
          {{"id", std::to_string(id++)},
           {"value", key_utf8},
           {"letters", letters},
           {"frequency", std::to_string(node->frequency)},
           {"derived_count", std::to_string(node->derived.size())}}));
    });

    return successResponse(jsonArray(items));
  });

  // ----- RACINES - GET ONE -----
  server.get(
      "/api/roots/:value",
      [&roots](const std::string &, const std::string &path,
               const std::string &,
               const std::map<std::string, std::string> &) {
        size_t lastSlash = path.find_last_of('/');
        if (lastSlash == std::string::npos || lastSlash + 1 >= path.size()) {
          return errorResponse("URL invalide");
        }

        std::string value = urlDecode(path.substr(lastSlash + 1));
        auto u32value = normalize_ar(unicode::utf8_to_u32(value));

        const AVLNode *foundNode = nullptr;
        roots.forEach([&](const AVLNode *n) {
          if (!foundNode && n->key == u32value) {
            foundNode = n;
          }
        });

        if (!foundNode) {
          return errorResponse("الجذر غير موجود");
        }

        std::string letters;
        for (size_t i = 0; i < foundNode->key.size(); i++) {
          if (i > 0)
            letters += "-";
          letters +=
              unicode::u32_to_utf8(std::vector<char32_t>{foundNode->key[i]});
        }

        return successResponse(jsonObject(
            {{"id", "1"},
             {"value", unicode::u32_to_utf8(foundNode->key)},
             {"letters", letters},
             {"frequency", std::to_string(foundNode->frequency)},
             {"derived_count", std::to_string(foundNode->derived.size())}}));
      });

  // ----- RACINES - POST -----
  server.post(
      "/api/roots", [&roots](const std::string &, const std::string &,
                             const std::string &body,
                             const std::map<std::string, std::string> &) {
        SimpleMap data = parseJson(body);
        std::string *valuePtr = data.get("value");
        if (!valuePtr)
          return errorResponse("valeur manquante");

        std::string value = *valuePtr;
        auto u32value = normalize_ar(unicode::utf8_to_u32(value));

        if (!isValidArabicRoot(u32value)) {
          return errorResponse("الجذر يجب أن يكون 3 أحرف عربية فقط");
        }

        if (roots.contains(u32value)) {
          return errorResponse("هذا الجذر موجود مسبقاً");
        }

        roots.insert(u32value);

        std::string letters;
        for (size_t i = 0; i < u32value.size(); i++) {
          if (i > 0)
            letters += "-";
          letters += unicode::u32_to_utf8(std::vector<char32_t>{u32value[i]});
        }

        return successResponse(
            jsonObject({{"id", std::to_string(g_nextId++)},
                        {"value", unicode::u32_to_utf8(u32value)},
                        {"letters", letters},
                        {"message", "تم إضافة الجذر بنجاح"}}));
      });

  // ----- RACINES - DELETE -----
  server.del(
      "/api/roots/:value",
      [&roots](const std::string &, const std::string &path,
               const std::string &,
               const std::map<std::string, std::string> &) {
        size_t lastSlash = path.find_last_of('/');
        if (lastSlash == std::string::npos || lastSlash + 1 >= path.size()) {
          return errorResponse("URL invalide");
        }

        std::string value = urlDecode(path.substr(lastSlash + 1));
        auto u32value = normalize_ar(unicode::utf8_to_u32(value));

        if (!roots.contains(u32value)) {
          return errorResponse("الجذر غير موجود");
        }

        roots.remove(u32value);
        return successResponse(jsonObject({{"message", "تم حذف الجذر بنجاح"}}));
      });

  // ----- SCHÈMES - GET ALL -----
  server.get(
      "/api/schemes",
      [&schemes](const std::string &, const std::string &, const std::string &,
                 const std::map<std::string, std::string> &) {
        auto all = schemes.allSchemes();
        std::vector<std::string> items;
        int id = 1;

        for (const auto &entry : all) {
          items.push_back(jsonObject(
              {{"id", std::to_string(id++)},
               {"name", unicode::u32_to_utf8(entry.name)},
               {"pattern", unicode::u32_to_utf8(entry.templ)},
               {"rule", unicode::u32_to_utf8(entry.rule.description)},
               {"rule_pattern", unicode::u32_to_utf8(entry.rule.pattern)}}));
        }

        return successResponse(jsonArray(items));
      });

  // ----- SCHÈMES - POST -----
  server.post(
      "/api/schemes", [&schemes](const std::string &, const std::string &,
                                 const std::string &body,
                                 const std::map<std::string, std::string> &) {
        SimpleMap data = parseJson(body);

        std::string *namePtr = data.get("name");
        std::string *patternPtr = data.get("pattern");
        if (!namePtr || !patternPtr) {
          return errorResponse("الرجاء إدخال الاسم والنمط");
        }

        auto name = normalize_ar(unicode::utf8_to_u32(*namePtr));
        auto pattern = normalize_ar(unicode::utf8_to_u32(*patternPtr));
        std::string *descPtr = data.get("description");
        std::string description = descPtr ? *descPtr : "";

        if (!isValidArabic(name)) {
          return errorResponse("اسم الوزن يجب أن يكون بالعربية");
        }

        schemes.put(name, pattern, description);

        return successResponse(jsonObject(
            {{"id", std::to_string(g_nextId++)},
             {"name", unicode::u32_to_utf8(name)},
             {"pattern", unicode::u32_to_utf8(pattern)},
             {"rule", description.empty() ? unicode::u32_to_utf8(pattern)
                                          : description}}));
      });

  // ----- SCHÈMES - PUT (UPDATE) -----
  server.put(
      "/api/schemes/:id",
      [&schemes](const std::string &, const std::string &path,
                 const std::string &body,
                 const std::map<std::string, std::string> &) {
        size_t lastSlash = path.find_last_of('/');
        if (lastSlash == std::string::npos || lastSlash + 1 >= path.size()) {
          return errorResponse("URL invalide");
        }

        std::string idStr = path.substr(lastSlash + 1);
        int id = 0;
        try {
          id = std::stoi(idStr);
        } catch (...) {
          return errorResponse("ID invalide");
        }

        SimpleMap data = parseJson(body);

        auto all = schemes.allSchemes();
        if (id < 1 || id > static_cast<int>(all.size())) {
          return errorResponse("الوزن غير موجود");
        }

        auto &entry = all[id - 1];

        std::string *namePtr = data.get("name");
        std::string *patternPtr = data.get("pattern");
        std::string *descPtr = data.get("description");

        auto newName =
            namePtr ? normalize_ar(unicode::utf8_to_u32(*namePtr)) : entry.name;
        auto newPattern = patternPtr
                              ? normalize_ar(unicode::utf8_to_u32(*patternPtr))
                              : entry.templ;
        std::string newDesc =
            descPtr ? *descPtr : unicode::u32_to_utf8(entry.rule.description);

        schemes.remove(entry.name);
        schemes.put(newName, newPattern, newDesc);

        return successResponse(
            jsonObject({{"id", std::to_string(id)},
                        {"name", unicode::u32_to_utf8(newName)},
                        {"pattern", unicode::u32_to_utf8(newPattern)},
                        {"rule", newDesc}}));
      });

  // ----- SCHÈMES - DELETE -----
  server.del(
      "/api/schemes/:id",
      [&schemes](const std::string &, const std::string &path,
                 const std::string &,
                 const std::map<std::string, std::string> &) {
        size_t lastSlash = path.find_last_of('/');
        if (lastSlash == std::string::npos || lastSlash + 1 >= path.size()) {
          return errorResponse("URL invalide");
        }

        std::string idStr = path.substr(lastSlash + 1);
        int id = 0;
        try {
          id = std::stoi(idStr);
        } catch (...) {
          return errorResponse("ID invalide");
        }

        auto all = schemes.allSchemes();
        if (id < 1 || id > static_cast<int>(all.size())) {
          return errorResponse("الوزن غير موجود");
        }

        schemes.remove(all[id - 1].name);
        return successResponse(jsonObject({{"message", "تم حذف الوزن بنجاح"}}));
      });

  // ----- VALIDATION -----
  server.post("/api/validate", [&roots, &schemes](
                                   const std::string &, const std::string &,
                                   const std::string &body,
                                   const std::map<std::string, std::string> &) {
    SimpleMap data = parseJson(body);

    std::string *wordPtr = data.get("word");
    std::string *rootPtr = data.get("root");
    if (!wordPtr || !rootPtr) {
      return errorResponse("paramètres manquants");
    }

    auto word = normalize_ar(unicode::utf8_to_u32(*wordPtr));
    auto root = normalize_ar(unicode::utf8_to_u32(*rootPtr));

    if (!isValidArabic(word)) {
      return errorResponse("الكلمة يجب أن تكون بالعربية فقط");
    }
    if (!isValidArabicRoot(root)) {
      return errorResponse("الجذر يجب أن يكون 3 أحرف عربية");
    }

    if (!roots.contains(root)) {
      return successResponse(
          jsonObject({{"valid", "false"},
                      {"word", unicode::u32_to_utf8(word)},
                      {"root", unicode::u32_to_utf8(root)},
                      {"message", "الجذر غير موجود في القاعدة"},
                      {"complexity", "O(log n)"}}));
    }

    auto allSchemes = schemes.allSchemes();
    std::string matchedScheme;
    std::string matchedPattern;
    bool valid = false;

    for (const auto &scheme : allSchemes) {
      auto extracted = extract_root_from_word(word, scheme.templ);
      if (extracted && *extracted == root) {
        valid = true;
        matchedScheme = unicode::u32_to_utf8(scheme.name);
        matchedPattern = unicode::u32_to_utf8(scheme.templ);
        break;
      }
    }

    std::vector<std::pair<std::string, std::string>> fields = {
        {"valid", valid ? "true" : "false"},
        {"word", unicode::u32_to_utf8(word)},
        {"root", unicode::u32_to_utf8(root)},
        {"message", valid ? "الكلمة صحيحة" : "الكلمة لا تنتمي لهذا الجذر"},
        {"complexity", "O(log n + m) حيث m = عدد الأوزان"}};

    if (valid) {
      fields.push_back({"scheme_name", matchedScheme});
      fields.push_back({"scheme_pattern", matchedPattern});
      roots.incrementFrequency(root);
    }

    return successResponse(jsonObject(fields));
  });

  // ----- GÉNÉRATION -----
  server.post("/api/generate", [&roots, &schemes](
                                   const std::string &, const std::string &,
                                   const std::string &body,
                                   const std::map<std::string, std::string> &) {
    SimpleMap data = parseJson(body);

    std::string *rootPtr = data.get("root");
    if (!rootPtr)
      return errorResponse("الرجاء إدخال الجذر");

    auto root = normalize_ar(unicode::utf8_to_u32(*rootPtr));

    if (!isValidArabicRoot(root)) {
      return errorResponse("الرجاء إدخال جذر عربي من 3 أحرف");
    }

    if (!roots.contains(root)) {
      return errorResponse("الجذر غير موجود في القاعدة");
    }

    std::vector<std::pair<std::vector<char32_t>, std::vector<char32_t>>>
        selectedSchemes;

    std::string *schemesPtr = data.get("schemes");
    if (schemesPtr && !schemesPtr->empty()) {
      std::vector<std::string> schemeNames = parseJsonArray(*schemesPtr);

      for (const auto &name : schemeNames) {
        auto *entry = schemes.get(normalize_ar(unicode::utf8_to_u32(name)));
        if (entry) {
          selectedSchemes.push_back({entry->name, entry->templ});
        }
      }
    }

    if (selectedSchemes.empty()) {
      auto all = schemes.allSchemes();
      for (const auto &s : all) {
        selectedSchemes.push_back({s.name, s.templ});
      }
    }

    std::vector<std::string> results;
    std::vector<std::vector<char32_t>> generatedWords;

    for (const auto &[name, templ] : selectedSchemes) {
      try {
        auto generated = apply_template(root, templ);
        generatedWords.push_back(generated);

        results.push_back(
            jsonObject({{"root", unicode::u32_to_utf8(root)},
                        {"scheme_name", unicode::u32_to_utf8(name)},
                        {"scheme_pattern", unicode::u32_to_utf8(templ)},
                        {"result", unicode::u32_to_utf8(generated)}}));

        roots.addDerived(root, generated);
      } catch (...) {
        continue;
      }
    }

    roots.incrementFrequency(root);

    std::string familyList = "[";
    for (size_t i = 0; i < generatedWords.size(); i++) {
      if (i > 0)
        familyList += ",";
      familyList +=
          "\"" + jsonEscape(unicode::u32_to_utf8(generatedWords[i])) + "\"";
    }
    familyList += "]";

    return successResponse("{\"derivatives\":" + jsonArray(results) +
                           ",\"family\":" + familyList + ",\"root\":\"" +
                           jsonEscape(unicode::u32_to_utf8(root)) + "\"}");
  });

  // ----- JEU - START -----
  server.get(
      "/api/game/start",
      [&roots, &schemes](const std::string &, const std::string &,
                         const std::string &,
                         const std::map<std::string, std::string> &) {
        std::vector<std::vector<char32_t>> allRoots;
        roots.forEach(
            [&](const AVLNode *node) { allRoots.push_back(node->key); });

        if (allRoots.empty()) {
          return errorResponse("لا توجد جذور متاحة في النظام");
        }

        auto allSchemes = schemes.allSchemes();
        if (allSchemes.empty()) {
          return errorResponse("لا توجد أوزان صرفية متاحة");
        }

        std::vector<std::vector<char32_t>> schemeNames;
        std::vector<std::vector<char32_t>> schemeTemplates;
        for (const auto &s : allSchemes) {
          schemeNames.push_back(s.name);
          schemeTemplates.push_back(s.templ);
        }

        std::vector<std::string> questions;
        for (int i = 0; i < 5; i++) {
          GameQuestion q =
              generate_game_question(allRoots, schemeNames, schemeTemplates);
          q.id = i + 1;

          std::string optionsArray = "[";
          for (size_t j = 0; j < q.options.size(); j++) {
            if (j > 0)
              optionsArray += ",";
            optionsArray +=
                "\"" + jsonEscape(unicode::u32_to_utf8(q.options[j])) + "\"";
          }
          optionsArray += "]";

          questions.push_back(
              jsonObject({{"id", std::to_string(q.id)},
                          {"type", q.type},
                          {"word", unicode::u32_to_utf8(q.word)},
                          {"root", unicode::u32_to_utf8(q.root)},
                          {"scheme_name", unicode::u32_to_utf8(q.scheme_name)},
                          {"difficulty", q.difficulty},
                          {"options", optionsArray}}));
        }

        return successResponse(
            "{\"questions\":" + jsonArray(questions) +
            ",\"total\":\"5\",\"pool\":\"" +
            std::to_string(allRoots.size() * allSchemes.size()) + "\"}");
      });

  // ----- JEU - ANSWER -----
  server.post("/api/game/answer",
              [](const std::string &, const std::string &,
                 const std::string &body,
                 const std::map<std::string, std::string> &) {
                SimpleMap data = parseJson(body);

                std::string *qidPtr = data.get("questionId");
                std::string *ansPtr = data.get("answer");

                if (!qidPtr || !ansPtr) {
                  return errorResponse("معرف السؤال أو الإجابة مفقود");
                }

                bool correct = !ansPtr->empty();

                return successResponse(
                    jsonObject({{"correct", correct ? "true" : "false"},
                                {"correctAnswer", *ansPtr}}));
              });

  // Route racine
  server.get("/api", [](const std::string &, const std::string &,
                        const std::string &,
                        const std::map<std::string, std::string> &) {
    return successResponse(jsonObject(
        {{"message", "Moteur Morphologique Arabe API"},
         {"version", "2.0.0"},
         {"features", "100% Manuel, AVL, HashTable, Validation, Jeu"}}));
  });
}

} // namespace morpho
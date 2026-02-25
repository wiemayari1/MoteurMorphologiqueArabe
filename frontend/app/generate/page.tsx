"use client";

import { useState, useEffect } from "react";
import { Card, CardContent, CardHeader, CardTitle } from "@/components/ui/card";
import { Input } from "@/components/ui/input";
import { Button } from "@/components/ui/button";
import { Badge } from "@/components/ui/badge";
import { Alert } from "@/components/ui/alert";
import { Spinner } from "@/components/ui/spinner";
import { generateWords, getSchemes, isValidRoot, isArabicText } from "@/lib/api";
import type { GeneratedWord, Scheme } from "@/lib/types";
import { Sparkles, Copy, Check, AlertCircle, BookOpen, Filter } from "lucide-react";

// Checkbox natif sans dépendance externe
function Checkbox({
  checked,
  onCheckedChange,
  className
}: {
  checked: boolean;
  onCheckedChange: (checked: boolean) => void;
  className?: string;
}) {
  return (
    <input
      type="checkbox"
      checked={checked}
      onChange={(e) => onCheckedChange(e.target.checked)}
      className={`w-4 h-4 rounded border-gray-300 text-teal-600 focus:ring-teal-500 cursor-pointer ${className}`}
    />
  );
}

export default function GeneratePage() {
  const [root, setRoot] = useState("");
  const [loading, setLoading] = useState(false);
  const [results, setResults] = useState<GeneratedWord[]>([]);
  const [family, setFamily] = useState<string[]>([]);
  const [error, setError] = useState("");
  const [copied, setCopied] = useState<string | null>(null);
  const [schemes, setSchemes] = useState<Scheme[]>([]);
  const [selectedSchemes, setSelectedSchemes] = useState<string[]>([]);
  const [showFilters, setShowFilters] = useState(false);
  const [inputError, setInputError] = useState("");

  useEffect(() => {
    loadSchemes();
  }, []);

  const loadSchemes = async () => {
    try {
      const response = await getSchemes();
      if (response.success && response.data && Array.isArray(response.data)) {
        setSchemes(response.data);
      } else {
        console.error('Failed to load schemes:', response.error);
      }
    } catch (err) {
      console.error('Error loading schemes:', err);
    }
  };

  const validateInput = (value: string) => {
    if (!value) {
      setInputError("");
      return;
    }
    if (!isArabicText(value)) {
      setInputError("يجب إدخال حروف عربية فقط");
    } else if (value.length !== 3) {
      setInputError("الجذر يجب أن يكون 3 أحرف");
    } else {
      setInputError("");
    }
  };

  const handleRootChange = (e: React.ChangeEvent<HTMLInputElement>) => {
    const value = e.target.value;
    setRoot(value);
    validateInput(value);
  };

  const toggleScheme = (schemeName: string) => {
    setSelectedSchemes(prev =>
      prev.includes(schemeName)
        ? prev.filter(s => s !== schemeName)
        : [...prev, schemeName]
    );
  };

  const handleGenerate = async () => {
    if (!isValidRoot(root)) {
      setError("الرجاء إدخال جذر صحيح (3 أحرف عربية)");
      return;
    }

    setLoading(true);
    setError("");
    setResults([]);
    setFamily([]);

    try {
      const response = await generateWords(root, selectedSchemes.length > 0 ? selectedSchemes : undefined);

      if (response.success && response.data) {
        // CORRECTION: Vérification correcte des données
        const data = response.data;

        // Vérifier que derivatives existe et est un tableau
        if (data.derivatives && Array.isArray(data.derivatives)) {
          setResults(data.derivatives);
        } else {
          setResults([]);
        }

        // Vérifier que family existe et est un tableau
        if (data.family && Array.isArray(data.family)) {
          setFamily(data.family);
        } else {
          setFamily([]);
        }

        // Message si aucun résultat
        if (!data.derivatives || data.derivatives.length === 0) {
          setError("لم يتم العثور على أوزان صرفية. أضف أوزاناً أولاً.");
        }
      } else {
        setError(response.error || "فشل في التوليد");
      }
    } catch (err) {
      setError("حدث خطأ غير متوقع");
      console.error(err);
    }

    setLoading(false);
  };

  const copyToClipboard = (text: string) => {
    navigator.clipboard.writeText(text);
    setCopied(text);
    setTimeout(() => setCopied(null), 2000);
  };

  const selectAllSchemes = () => {
    setSelectedSchemes(schemes.map(s => s.name));
  };

  const clearSelection = () => {
    setSelectedSchemes([]);
  };

  return (
    <div className="max-w-4xl mx-auto animate-slide-up">
      <Card className="border-0 shadow-glass">
        <CardHeader>
          <CardTitle className="text-2xl font-arabic text-teal-900 flex items-center gap-2">
            <Sparkles className="w-6 h-6" />
            توليد الكلمات المشتقة
          </CardTitle>
        </CardHeader>
        <CardContent className="space-y-6">
          {/* Info */}
          <div className="bg-amber-50 p-4 rounded-lg flex items-start gap-3">
            <BookOpen className="w-5 h-5 text-amber-600 mt-0.5" />
            <div className="text-sm text-amber-800">
              <p className="font-bold mb-1">توليد العائلة الصرفية:</p>
              <p>أدخل جذراً من 3 أحرف عربية لإنشاء جميع الكلمات المشتقة باستخدام الأوزان الصرفية</p>
            </div>
          </div>

          {/* Input */}
          <div className="flex gap-2">
            <div className="flex-1 relative">
              <Input
                placeholder="أدخل الجذر (٣ أحرف عربية)"
                value={root}
                onChange={handleRootChange}
                className={`text-lg text-right ${inputError ? "border-red-500" : ""}`}
                maxLength={3}
              />
              {inputError && (
                <p className="text-red-500 text-xs mt-1 flex items-center gap-1">
                  <AlertCircle className="w-3 h-3" />
                  {inputError}
                </p>
              )}
            </div>
            <Button
              onClick={handleGenerate}
              disabled={loading || !!inputError || root.length !== 3}
              className="gap-2"
            >
              {loading ? (
                <span className="animate-spin">⏳</span>
              ) : (
                <>
                  <Sparkles className="w-4 h-4" />
                  توليد
                </>
              )}
            </Button>
          </div>

          {/* Filters */}
          {schemes.length > 0 && (
            <div className="border rounded-lg p-4 bg-gray-50">
              <div
                className="flex items-center justify-between cursor-pointer"
                onClick={() => setShowFilters(!showFilters)}
              >
                <div className="flex items-center gap-2 text-gray-700">
                  <Filter className="w-4 h-4" />
                  <span className="font-arabic">اختيار الأوزان (اختياري)</span>
                </div>
                <Badge variant="secondary">
                  {selectedSchemes.length > 0 ? `${selectedSchemes.length} مختار` : "الكل"}
                </Badge>
              </div>

              {showFilters && (
                <div className="mt-4 space-y-3 animate-fade-in">
                  <div className="flex gap-2">
                    <Button variant="outline" size="sm" onClick={selectAllSchemes}>
                      اختيار الكل
                    </Button>
                    <Button variant="outline" size="sm" onClick={clearSelection}>
                      إلغاء الكل
                    </Button>
                  </div>
                  <div className="grid grid-cols-2 md:grid-cols-3 gap-2">
                    {schemes.map((scheme) => (
                      <label
                        key={scheme.id}
                        className="flex items-center gap-2 p-2 bg-white rounded border cursor-pointer hover:bg-gray-50"
                      >
                        <Checkbox
                          checked={selectedSchemes.includes(scheme.name)}
                          onCheckedChange={() => toggleScheme(scheme.name)}
                        />
                        <span className="font-arabic text-sm">{scheme.name}</span>
                        <code className="text-xs text-gray-500 mr-auto">{scheme.pattern}</code>
                      </label>
                    ))}
                  </div>
                </div>
              )}
            </div>
          )}

          {error && <Alert variant="error">{error}</Alert>}

          {/* Loading State */}
          {loading && (
            <div className="py-12">
              <Spinner size="md" text="جاري توليد الكلمات..." />
            </div>
          )}

          {/* Results - CORRECTION: Vérification sécurisée */}
          {!loading && results && results.length > 0 && (
            <div className="space-y-6 animate-fade-in">
              {/* Family Overview */}
              <div className="bg-teal-50 p-4 rounded-lg">
                <h3 className="font-arabic font-bold text-teal-900 mb-3">
                  عائلة الجذر: {root}
                </h3>
                <div className="flex flex-wrap gap-2">
                  {family && family.map((word, idx) => (
                    <Badge key={idx} variant="default" className="text-lg py-1 px-3">
                      {word}
                    </Badge>
                  ))}
                </div>
              </div>

              {/* Detailed Results */}
              <div className="grid gap-3">
                <h3 className="font-arabic text-lg text-gray-700">
                  التفاصيل ({results.length} كلمة):
                </h3>
                {results.map((item, index) => (
                  <div
                    key={index}
                    className="flex items-center justify-between p-4 bg-white/70 rounded-xl border border-gray-100 hover:border-teal-200 transition-colors"
                  >
                    <div className="flex items-center gap-2">
                      <Button
                        variant="ghost"
                        size="icon"
                        className="text-gray-400 hover:text-teal-600"
                        onClick={() => copyToClipboard(item.result)}
                      >
                        {copied === item.result ? (
                          <Check className="w-4 h-4 text-green-500" />
                        ) : (
                          <Copy className="w-4 h-4" />
                        )}
                      </Button>
                    </div>

                    <div className="text-right flex-1 mr-4">
                      <p className="text-2xl font-bold text-teal-900 font-arabic mb-1">
                        {item.result}
                      </p>
                      <p className="text-sm text-gray-500 font-arabic">
                        الوزن: <span className="font-bold">{item.scheme_name}</span>
                        {" "}(<code className="bg-gray-100 px-1 rounded">{item.scheme_pattern}</code>)
                      </p>
                    </div>

                    <Badge variant="secondary">{index + 1}</Badge>
                  </div>
                ))}
              </div>
            </div>
          )}

          {/* No Results Message */}
          {!loading && !error && results && results.length === 0 && root && (
            <div className="text-center py-8 text-gray-500 font-arabic">
              اضغط على زر "توليد" لإنشاء الكلمات المشتقة
            </div>
          )}
        </CardContent>
      </Card>
    </div>
  );
}

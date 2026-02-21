"use client";

import { useState } from "react";
import { Card, CardContent, CardHeader, CardTitle } from "@/components/ui/card";
import { Input } from "@/components/ui/input";
import { Button } from "@/components/ui/button";
import { Badge } from "@/components/ui/badge";
import { Alert } from "@/components/ui/alert";
import { Spinner } from "@/components/ui/spinner";
import { validateWord, isArabicText, isValidRoot } from "@/lib/api";
import type { ValidationResult } from "@/lib/types";
import { Search, CheckCircle, XCircle, AlertCircle, BookOpen } from "lucide-react";

export default function ValidatePage() {
  const [word, setWord] = useState("");
  const [root, setRoot] = useState("");
  const [loading, setLoading] = useState(false);
  const [result, setResult] = useState<ValidationResult | null>(null);
  const [error, setError] = useState("");

  const validateInput = () => {
    if (!isArabicText(word)) {
      setError("الكلمة يجب أن تكون بالعربية فقط");
      return false;
    }
    if (!isValidRoot(root)) {
      setError("الجذر يجب أن يكون 3 أحرف عربية");
      return false;
    }
    return true;
  };

  const handleValidate = async () => {
    if (!validateInput()) return;

    setLoading(true);
    setError("");
    setResult(null);

    const response = await validateWord(word, root);

    if (response.success && response.data) {
      setResult(response.data);
    } else {
      setError(response.error || "فشل في التحقق");
    }

    setLoading(false);
  };

  // Helper pour vérifier si c'est valide (gère boolean et string)
  const isValid = (valid: boolean | string | undefined): boolean => {
    return valid === true || valid === "true";
  };

  return (
    <div className="max-w-2xl mx-auto animate-slide-up">
      <Card className="border-0 shadow-glass">
        <CardHeader>
          <CardTitle className="text-2xl font-arabic text-teal-900 flex items-center gap-2 justify-center">
            <Search className="w-6 h-6" />
            التحقق من الكلمات
          </CardTitle>
        </CardHeader>
        <CardContent className="space-y-6">
          {/* Info */}
          <div className="bg-blue-50 p-4 rounded-lg flex items-start gap-3">
            <BookOpen className="w-5 h-5 text-blue-600 mt-0.5" />
            <p className="text-sm text-blue-800">
              أدخل كلمة عربية وجذرها للتحقق من انتماء الكلمة لهذا الجذر صرفياً
            </p>
          </div>

          {/* Inputs */}
          <div className="space-y-4">
            <div>
              <label className="block text-sm font-medium text-gray-700 mb-1 text-right font-arabic">
                الكلمة المراد التحقق منها
              </label>
              <Input
                placeholder="مثال: كتب، مكتوب، كاتب..."
                value={word}
                onChange={(e) => {
                  setWord(e.target.value);
                  setError("");
                }}
                className="text-lg text-right"
              />
            </div>

            <div>
              <label className="block text-sm font-medium text-gray-700 mb-1 text-right font-arabic">
                الجذر المقترح (3 أحرف)
              </label>
              <Input
                placeholder="مثال: كتب"
                value={root}
                onChange={(e) => {
                  setRoot(e.target.value);
                  setError("");
                }}
                className="text-lg text-right"
                maxLength={3}
              />
            </div>
          </div>

          {error && (
            <Alert variant="error" className="flex items-center gap-2">
              <AlertCircle className="w-5 h-5" />
              {error}
            </Alert>
          )}

          <Button
            onClick={handleValidate}
            disabled={loading || !word || root.length !== 3}
            className="w-full gap-2 text-lg"
            size="lg"
          >
            {loading ? (
              <Spinner size="sm" />
            ) : (
              <>
                <Search className="w-5 h-5" />
                تحقق
              </>
            )}
          </Button>

          {/* Result */}
          {result && (
            <div className={`p-6 rounded-xl border-2 animate-fade-in ${isValid(result.valid)
                ? "bg-green-50 border-green-200"
                : "bg-red-50 border-red-200"
              }`}>
              <div className="text-center mb-4">
                {isValid(result.valid) ? (
                  <CheckCircle className="w-16 h-16 text-green-500 mx-auto mb-2" />
                ) : (
                  <XCircle className="w-16 h-16 text-red-500 mx-auto mb-2" />
                )}
                <h3 className={`text-2xl font-bold font-arabic ${isValid(result.valid) ? "text-green-800" : "text-red-800"
                  }`}>
                  {isValid(result.valid) ? "✅ نعم، الكلمة صحيحة" : "❌ لا، الكلمة غير صحيحة"}
                </h3>
              </div>

              <div className="bg-white/70 rounded-lg p-4 space-y-3">
                <div className="flex justify-between items-center">
                  <span className="font-bold text-gray-700">{result.word}</span>
                  <span className="text-gray-500">الكلمة</span>
                </div>
                <div className="flex justify-between items-center">
                  <span className="font-bold text-gray-700">{result.root}</span>
                  <span className="text-gray-500">الجذر</span>
                </div>

                {isValid(result.valid) && result.scheme_name && (
                  <>
                    <div className="flex justify-between items-center">
                      <Badge variant="default" className="font-arabic text-lg">
                        {result.scheme_name}
                      </Badge>
                      <span className="text-gray-500">الوزن الصرفي</span>
                    </div>
                    <div className="flex justify-between items-center">
                      <code className="bg-gray-100 px-2 py-1 rounded text-sm">
                        {result.scheme_pattern}
                      </code>
                      <span className="text-gray-500">النمط</span>
                    </div>
                  </>
                )}

                <div className="pt-2 border-t mt-2">
                  <p className="text-xs text-gray-400 text-center font-mono">
                    {result.complexity}
                  </p>
                </div>
              </div>
            </div>
          )}
        </CardContent>
      </Card>
    </div>
  );
}
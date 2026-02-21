// frontend/app/roots/page.tsx - VERSION CORRIGÉE AVEC ID
"use client";

import { useEffect, useState } from "react";
import { Card, CardContent, CardHeader, CardTitle } from "@/components/ui/card";
import { Input } from "@/components/ui/input";
import { Button } from "@/components/ui/button";
import { Badge } from "@/components/ui/badge";
import { Alert } from "@/components/ui/alert";
import { Spinner } from "@/components/ui/spinner";
import { getRoots, addRoot, deleteRoot, isValidRoot, isArabicText } from "@/lib/api";
import type { Root } from "@/lib/types";
import { TreePine, Plus, Trash2, RefreshCw, AlertCircle } from "lucide-react";

export default function RootsPage() {
  const [roots, setRoots] = useState<Root[]>([]);
  const [newRoot, setNewRoot] = useState("");
  const [loading, setLoading] = useState(true);
  const [adding, setAdding] = useState(false);
  const [error, setError] = useState("");
  const [success, setSuccess] = useState("");
  const [inputError, setInputError] = useState("");
  const [deletingId, setDeletingId] = useState<number | null>(null); // ← number, pas string

  useEffect(() => {
    loadRoots();
  }, []);

  const loadRoots = async () => {
    setLoading(true);
    setError("");
    const response = await getRoots();
    if (response.success && response.data) {
      setRoots(response.data);
    } else {
      setError(response.error || "فشل في تحميل الجذور");
    }
    setLoading(false);
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

  const handleAdd = async () => {
    if (!isValidRoot(newRoot)) {
      setError("الرجاء إدخال جذر صحيح (3 أحرف عربية)");
      return;
    }

    setAdding(true);
    setError("");
    setSuccess("");

    const response = await addRoot(newRoot);
    if (response.success) {
      setSuccess("تم إضافة الجذر بنجاح");
      setNewRoot("");
      await loadRoots();
    } else {
      setError(response.error || "فشل في إضافة الجذر");
    }
    setAdding(false);
  };

  // CORRECTION: Utiliser l'ID (number) au lieu de la valeur (string)
  const handleDelete = async (id: number, value: string) => {
    if (!confirm(`هل أنت متأكد من حذف الجذر "${value}"؟`)) {
      return;
    }

    setDeletingId(id);
    setError("");
    setSuccess("");

    const response = await deleteRoot(id); // ← ID, pas value !
    if (response.success) {
      setSuccess("تم حذف الجذر بنجاح");
      await loadRoots();
    } else {
      setError(response.error || "فشل في حذف الجذر");
    }
    setDeletingId(null);
  };

  return (
    <div className="max-w-4xl mx-auto animate-slide-up">
      <Card className="border-0 shadow-glass">
        <CardHeader>
          <CardTitle className="text-2xl font-arabic text-teal-900 flex items-center gap-2">
            <TreePine className="w-6 h-6" />
            إدارة الجذور
          </CardTitle>
        </CardHeader>
        <CardContent className="space-y-6">
          {/* Add Root */}
          <div className="flex gap-2">
            <Button
              onClick={loadRoots}
              variant="outline"
              size="icon"
              disabled={loading}
            >
              <RefreshCw className={`w-4 h-4 ${loading ? "animate-spin" : ""}`} />
            </Button>
            <Button
              onClick={handleAdd}
              disabled={adding || !!inputError || newRoot.length !== 3}
              className="gap-2"
            >
              {adding ? (
                <span className="animate-spin">⏳</span>
              ) : (
                <>
                  <Plus className="w-4 h-4" />
                  إضافة
                </>
              )}
            </Button>
            <div className="flex-1 relative">
              <Input
                placeholder="إضافة جذر (٣ أحرف عربية)"
                value={newRoot}
                onChange={(e) => {
                  setNewRoot(e.target.value);
                  validateInput(e.target.value);
                }}
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
          </div>

          {error && <Alert variant="error">{error}</Alert>}
          {success && <Alert variant="success">{success}</Alert>}

          {/* Search */}
          <div className="relative">
            <Input
              placeholder="بحث..."
              className="text-right"
              onChange={(e) => {
                const term = e.target.value;
                if (!term) {
                  loadRoots();
                  return;
                }
                setRoots(prev => prev.filter(r =>
                  r.value.includes(term) || r.letters.includes(term)
                ));
              }}
            />
          </div>

          {/* Roots List */}
          {loading ? (
            <div className="py-12">
              <Spinner size="md" text="جاري التحميل..." />
            </div>
          ) : (
            <div className="space-y-2">
              <div className="flex justify-between items-center text-sm text-gray-500 mb-2">
                <span>العدد: {roots.length}</span>
                <span>جذر</span>
              </div>

              {roots.map((root, index) => (
                <div
                  key={root.id}
                  className="flex items-center justify-between p-3 bg-white/70 rounded-xl border border-gray-100 hover:border-teal-200 transition-all"
                >
                  <div className="flex items-center gap-2">
                    <Button
                      variant="ghost"
                      size="icon"
                      className="text-red-400 hover:text-red-600 hover:bg-red-50"
                      onClick={() => handleDelete(root.id, root.value)} // ← ID + value !
                      disabled={deletingId === root.id}
                    >
                      {deletingId === root.id ? (
                        <span className="animate-spin text-xs">⏳</span>
                      ) : (
                        <Trash2 className="w-4 h-4" />
                      )}
                    </Button>
                  </div>

                  <div className="text-right flex-1 mr-4">
                    <p className="text-xl font-bold text-teal-900 font-arabic">
                      {root.value}
                    </p>
                    <p className="text-sm text-gray-500 font-mono">
                      {root.letters}
                    </p>
                  </div>

                  <Badge variant="secondary" className="ml-2">
                    {index + 1}
                  </Badge>
                </div>
              ))}

              {roots.length === 0 && !loading && (
                <div className="text-center py-8 text-gray-500 font-arabic">
                  لا توجد جذور مسجلة
                </div>
              )}
            </div>
          )}
        </CardContent>
      </Card>
    </div>
  );
}
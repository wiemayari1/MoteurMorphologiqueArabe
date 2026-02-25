"use client";

import { useEffect, useState } from "react";
import { Card, CardContent, CardHeader, CardTitle } from "@/components/ui/card";
import { Input } from "@/components/ui/input";
import { Button } from "@/components/ui/button";
import { Badge } from "@/components/ui/badge";
import { Alert } from "@/components/ui/alert";
import { Spinner } from "@/components/ui/spinner";
import { getSchemes, addScheme, deleteScheme, updateScheme, isArabicText } from "@/lib/api";
import type { Scheme } from "@/lib/types";
import { Grid3X3, Plus, Trash2, Edit2, Check, X, AlertCircle, BookOpen } from "lucide-react";

export default function SchemesPage() {
  const [schemes, setSchemes] = useState<Scheme[]>([]);
  const [newScheme, setNewScheme] = useState({ name: "", pattern: "", rule: "" });
  const [loading, setLoading] = useState(true);
  const [adding, setAdding] = useState(false);
  const [error, setError] = useState("");
  const [success, setSuccess] = useState("");
  const [editingId, setEditingId] = useState<number | null>(null);
  const [editData, setEditData] = useState({ name: "", pattern: "", rule: "" });

  useEffect(() => {
    loadSchemes();
  }, []);

  const loadSchemes = async () => {
    setLoading(true);
    setError("");
    const response = await getSchemes();
    if (response.success && response.data) {
      setSchemes(response.data);
    } else {
      setError(response.error || "فشل في تحميل الأوزان");
    }
    setLoading(false);
  };

  const validatePattern = (pattern: string) => {
    // Vérifier que le pattern contient ف، ع، ل
    return pattern.includes('ف') && pattern.includes('ع') && pattern.includes('ل');
  };

  const handleAdd = async () => {
    if (!isArabicText(newScheme.name)) {
      setError("اسم الوزن يجب أن يكون بالعربية");
      return;
    }
    if (!validatePattern(newScheme.pattern)) {
      setError("القاعدة يجب أن تحتوي على ف، ع، ل");
      return;
    }

    setAdding(true);
    setError("");
    setSuccess("");

    const response = await addScheme(newScheme);
    if (response.success) {
      setSuccess("تم إضافة الوزن بنجاح");
      setNewScheme({ name: "", pattern: "", rule: "" });
      await loadSchemes();
    } else {
      setError(response.error || "فشل في إضافة الوزن");
    }
    setAdding(false);
  };

  // Utiliser l'ID (number) au lieu du nom (string)
  const handleDelete = async (id: number, name: string) => {
    if (!confirm(`هل أنت متأكد من حذف الوزن "${name}"؟`)) {
      return;
    }

    setError("");
    setSuccess("");

    const response = await deleteScheme(id); // ← ID, pas name !
    if (response.success) {
      setSuccess("تم حذف الوزن بنجاح");
      await loadSchemes();
    } else {
      setError(response.error || "فشل في حذف الوزن");
    }
  };

  const startEdit = (scheme: Scheme) => {
    setEditingId(scheme.id);
    setEditData({
      name: scheme.name,
      pattern: scheme.pattern,
      rule: scheme.rule
    });
  };

  const cancelEdit = () => {
    setEditingId(null);
    setEditData({ name: "", pattern: "", rule: "" });
  };

  // Utiliser l'ID (number) au lieu du nom (string)
  const handleUpdate = async (id: number) => {
    setError("");
    setSuccess("");

    // Validation
    if (!isArabicText(editData.name)) {
      setError("اسم الوزن يجب أن يكون بالعربية");
      return;
    }
    if (!validatePattern(editData.pattern)) {
      setError("القاعدة يجب أن تحتوي على ف، ع، ل");
      return;
    }

    const response = await updateScheme(id, {
      name: editData.name,
      pattern: editData.pattern,
      rule: editData.rule
    });

    if (response.success) {
      setSuccess("تم تحديث الوزن بنجاح");
      setEditingId(null);
      await loadSchemes();
    } else {
      setError(response.error || "فشل في تحديث الوزن");
    }
  };

  return (
    <div className="max-w-4xl mx-auto animate-slide-up">
      <Card className="border-0 shadow-glass">
        <CardHeader>
          <CardTitle className="text-2xl font-arabic text-teal-900 flex items-center gap-2">
            <Grid3X3 className="w-6 h-6" />
            إدارة الأوزان الصرفية
          </CardTitle>
        </CardHeader>
        <CardContent className="space-y-6">
          {/* Info */}
          <div className="bg-blue-50 p-4 rounded-lg flex items-start gap-3">
            <BookOpen className="w-5 h-5 text-blue-600 mt-0.5" />
            <div className="text-sm text-blue-800">
              <p className="font-bold mb-1">كيفية إضافة وزن:</p>
              <p>استخدم حروف ف، ع، ل لتمثيل مواضع الجذر مثال: "فاعل" لـ كاتب، "مفعول" لـ مكتوب</p>
            </div>
          </div>

          {/* Add Scheme */}
          <div className="flex gap-2 items-start">
            <Button
              onClick={handleAdd}
              disabled={adding || !newScheme.name || !newScheme.pattern}
              className="gap-2 mt-0"
            >
              {adding ? (
                <span className="animate-spin">⏳</span>
              ) : (
                <>
                  <Plus className="w-4 h-4" />
                </>
              )}
            </Button>
            <div className="flex-1 space-y-2">
              <Input
                placeholder="اسم الوزن (مثال: فاعل)"
                value={newScheme.name}
                onChange={(e) => setNewScheme({ ...newScheme, name: e.target.value })}
                className="text-lg text-right"
              />
              <Input
                placeholder="القالب (مثال: فاعل)"
                value={newScheme.pattern}
                onChange={(e) => setNewScheme({ ...newScheme, pattern: e.target.value })}
                className="text-lg text-right"
              />
              <Input
                placeholder="وصف القاعدة (اختياري)"
                value={newScheme.rule}
                onChange={(e) => setNewScheme({ ...newScheme, rule: e.target.value })}
                className="text-right text-sm"
              />
              {!validatePattern(newScheme.pattern) && newScheme.pattern && (
                <p className="text-red-500 text-xs flex items-center gap-1">
                  <AlertCircle className="w-3 h-3" />
                  يجب أن يحتوي على ف، ع، ل
                </p>
              )}
            </div>
          </div>

          {error && <Alert variant="error">{error}</Alert>}
          {success && <Alert variant="success">{success}</Alert>}

          {/* Schemes Table */}
          {loading ? (
            <div className="py-12">
              <Spinner size="md" text="جاري التحميل..." />
            </div>
          ) : (
            <div className="border rounded-lg overflow-hidden">
              <table className="w-full text-right">
                <thead className="bg-gray-50">
                  <tr>
                    <th className="p-3 text-sm font-medium text-gray-700">#</th>
                    <th className="p-3 text-sm font-medium text-gray-700">الوزن</th>
                    <th className="p-3 text-sm font-medium text-gray-700">القالب</th>
                    <th className="p-3 text-sm font-medium text-gray-700">القاعدة</th>
                    <th className="p-3 text-sm font-medium text-gray-700">إجراءات</th>
                  </tr>
                </thead>
                <tbody className="divide-y">
                  {schemes.map((scheme) => (
                    <tr key={scheme.id} className="hover:bg-gray-50">
                      <td className="p-3 text-sm text-gray-500">{scheme.id}</td>
                      <td className="p-3 font-arabic font-bold text-teal-900">
                        {editingId === scheme.id ? (
                          <Input
                            value={editData.name}
                            onChange={(e) => setEditData({ ...editData, name: e.target.value })}
                            className="text-sm"
                          />
                        ) : (
                          scheme.name
                        )}
                      </td>
                      <td className="p-3">
                        {editingId === scheme.id ? (
                          <Input
                            value={editData.pattern}
                            onChange={(e) => setEditData({ ...editData, pattern: e.target.value })}
                            className="text-sm"
                          />
                        ) : (
                          <Badge variant="secondary" className="font-arabic">
                            {scheme.pattern}
                          </Badge>
                        )}
                      </td>
                      <td className="p-3 text-sm text-gray-600 font-arabic">
                        {editingId === scheme.id ? (
                          <Input
                            value={editData.rule}
                            onChange={(e) => setEditData({ ...editData, rule: e.target.value })}
                            className="text-sm"
                          />
                        ) : (
                          scheme.rule
                        )}
                      </td>
                      <td className="p-3">
                        <div className="flex items-center gap-1 justify-end">
                          {editingId === scheme.id ? (
                            <>
                              <Button
                                variant="ghost"
                                size="icon"
                                className="text-green-600 hover:text-green-800"
                                onClick={() => handleUpdate(scheme.id)} // ← ID !
                              >
                                <Check className="w-4 h-4" />
                              </Button>
                              <Button
                                variant="ghost"
                                size="icon"
                                className="text-gray-600 hover:text-gray-800"
                                onClick={cancelEdit}
                              >
                                <X className="w-4 h-4" />
                              </Button>
                            </>
                          ) : (
                            <>
                              <Button
                                variant="ghost"
                                size="icon"
                                className="text-blue-400 hover:text-blue-600 hover:bg-blue-50"
                                onClick={() => startEdit(scheme)}
                              >
                                <Edit2 className="w-4 h-4" />
                              </Button>
                              <Button
                                variant="ghost"
                                size="icon"
                                className="text-red-400 hover:text-red-600 hover:bg-red-50"
                                onClick={() => handleDelete(scheme.id, scheme.name)} // ← ID + nom !
                              >
                                <Trash2 className="w-4 h-4" />
                              </Button>
                            </>
                          )}
                        </div>
                      </td>
                    </tr>
                  ))}
                </tbody>
              </table>

              {schemes.length === 0 && !loading && (
                <div className="text-center py-8 text-gray-500 font-arabic">
                  لا توجد أوزان مسجلة
                </div>
              )}
            </div>
          )}
        </CardContent>
      </Card>
    </div>
  );
}

// frontend/lib/api.ts - VERSION CORRIGÉE COMPLÈTE ET FINALE
const API_BASE = process.env.NEXT_PUBLIC_API_URL || "http://localhost:3001";
// ==================== INTERFACES ====================

export interface ApiResponse<T> {
  success: boolean;
  data?: T;
  error?: string;
  timestamp?: string;
}

export interface Root {
  id: number;
  value: string;
  letters: string;
  frequency: number;
  derived_count: number;
}

export interface Scheme {
  id: number;
  name: string;
  pattern: string;
  rule: string;
  rule_pattern?: string;
}

export interface GeneratedWord {
  root: string;
  scheme_name: string;
  scheme_pattern: string;
  result: string;
}

export interface GenerateResponse {
  derivatives: GeneratedWord[];
  family: string[];
  root: string;
}

export interface ValidationResult {
  valid: boolean;
  word: string;
  root: string;
  message: string;
  complexity: string;
  scheme_name?: string;
  scheme_pattern?: string;
}

export interface GameQuestion {
  id: number;
  type: "find_root" | "find_scheme" | "validate_word";
  word: string;
  root: string;
  scheme_name: string;
  difficulty: "easy" | "medium" | "hard";
  options: string[];
}

// ==================== VALIDATION UTILITAIRES ====================

export function isArabicText(text: string): boolean {
  if (!text || typeof text !== 'string') return false;
  const arabicRegex = /^[\u0600-\u06FF\u0750-\u077F\u08A0-\u08FF\uFB50-\uFDFF\uFE70-\uFEFF]+$/;
  const cleaned = text.trim().replace(/\s+/g, '');
  return arabicRegex.test(cleaned) && cleaned.length > 0;
}

export function isValidRoot(text: string): boolean {
  if (!text || typeof text !== 'string') return false;
  const normalized = text.trim().replace(/[\u0640]/g, '').replace(/\s+/g, '');
  return isArabicText(normalized) && normalized.length === 3;
}

export function normalizeArabic(text: string): string {
  if (!text) return '';
  return text
    .trim()
    .replace(/[\u0640]/g, '')
    .replace(/\s+/g, ' ')
    .replace(/[\u200B-\u200F\uFEFF]/g, '')
    .trim();
}

// ==================== FONCTION DE BASE FETCH ====================

async function fetchApi<T>(
  endpoint: string,
  options?: RequestInit
): Promise<ApiResponse<T>> {
  try {
    const url = `${API_BASE}${endpoint}`;
    console.log(`[API] ${options?.method || 'GET'} ${url}`);

    const controller = new AbortController();
    const timeoutId = setTimeout(() => controller.abort(), 10000);

    const response = await fetch(url, {
      ...options,
      signal: controller.signal,
      headers: {
        'Content-Type': 'application/json; charset=utf-8',
        'Accept': 'application/json',
        ...options?.headers,
      },
    });

    clearTimeout(timeoutId);

    if (!response.ok) {
      const errorText = await response.text();
      console.error(`[API Error] ${response.status}:`, errorText);

      let errorMessage = `خطأ في الاتصال بالخادم: ${response.status}`;

      if (response.status === 404) {
        errorMessage = 'المورد غير موجود (404)';
      } else if (response.status === 500) {
        errorMessage = 'خطأ في الخادم الداخلي (500)';
      } else if (response.status === 400) {
        errorMessage = 'طلب غير صالح (400)';
      }

      return {
        success: false,
        error: errorMessage,
      };
    }

    const data = await response.json();
    console.log('[API Response]:', data);

    if (data && typeof data.success === 'boolean') {
      return data as ApiResponse<T>;
    }

    return {
      success: true,
      data: data as T,
    };

  } catch (error) {
    console.error('[API Fetch Error]:', error);

    let errorMessage = 'فشل في الاتصال بالخادم';

    if (error instanceof TypeError && error.message === 'Failed to fetch') {
      errorMessage = 'لا يمكن الاتصال بالخادم. تأكد من أن الخادم يعمل على ' + API_BASE;
    } else if (error instanceof Error) {
      if (error.name === 'AbortError') {
        errorMessage = 'انتهت مهلة الاتصال بالخادم';
      } else {
        errorMessage = error.message;
      }
    }

    return {
      success: false,
      error: errorMessage,
    };
  }
}

// ==================== API ROOTS ====================

export async function getRoots(): Promise<ApiResponse<Root[]>> {
  return fetchApi<Root[]>('/api/roots');
}

export async function addRoot(value: string): Promise<ApiResponse<Root>> {
  const normalizedValue = normalizeArabic(value);

  if (!isValidRoot(normalizedValue)) {
    return {
      success: false,
      error: 'الجذر يجب أن يكون 3 أحرف عربية فقط (بدون حروف إضافية)',
    };
  }

  return fetchApi<Root>('/api/roots', {
    method: 'POST',
    body: JSON.stringify({ value: normalizedValue }),
  });
}

export async function deleteRoot(value: string): Promise<ApiResponse<void>> {
  const normalizedValue = normalizeArabic(value);
  return fetchApi<void>(`/api/roots/${encodeURIComponent(normalizedValue)}`, {
    method: 'DELETE',
  });
}

// ==================== API SCHEMES ====================

export async function getSchemes(): Promise<ApiResponse<Scheme[]>> {
  return fetchApi<Scheme[]>('/api/schemes');
}

export async function addScheme(scheme: {
  name: string;
  pattern: string;
  rule?: string
}): Promise<ApiResponse<Scheme>> {
  const normalizedName = normalizeArabic(scheme.name);
  const normalizedPattern = normalizeArabic(scheme.pattern);

  if (!isArabicText(normalizedName)) {
    return {
      success: false,
      error: 'اسم الوزن يجب أن يكون بالعربية فقط',
    };
  }

  const hasFa = normalizedPattern.includes('ف');
  const hasAin = normalizedPattern.includes('ع');
  const hasLam = normalizedPattern.includes('ل');

  if (!hasFa || !hasAin || !hasLam) {
    return {
      success: false,
      error: 'القاعدة يجب أن تحتوي على حروف ف، ع، ل (مثل: فَعَلَ)',
    };
  }

  return fetchApi<Scheme>('/api/schemes', {
    method: 'POST',
    body: JSON.stringify({
      name: normalizedName,
      pattern: normalizedPattern,
      description: scheme.rule ? normalizeArabic(scheme.rule) : normalizedPattern,
    }),
  });
}

// ==================== API GENERATION ====================

export async function generateWords(
  root: string,
  selectedSchemes?: string[]
): Promise<ApiResponse<GenerateResponse>> {
  const normalizedRoot = normalizeArabic(root);

  if (!isValidRoot(normalizedRoot)) {
    return {
      success: false,
      error: 'الرجاء إدخال جذر عربي صحيح من 3 أحرف (مثل: كتب، فعل)',
    };
  }

  const body: any = { root: normalizedRoot };

  if (selectedSchemes && selectedSchemes.length > 0) {
    body.schemes = selectedSchemes.map(s => normalizeArabic(s));
  }

  return fetchApi<GenerateResponse>('/api/generate', {
    method: 'POST',
    body: JSON.stringify(body),
  });
}

// ==================== API VALIDATION - CORRIGÉ ====================

export async function validateWord(
  word: string,
  root: string
): Promise<ApiResponse<ValidationResult>> {
  const normalizedWord = normalizeArabic(word);
  const normalizedRoot = normalizeArabic(root);

  if (!normalizedWord) {
    return {
      success: false,
      error: 'الرجاء إدخال كلمة للتحقق',
    };
  }

  if (!isArabicText(normalizedWord)) {
    return {
      success: false,
      error: 'الكلمة يجب أن تحتوي على حروف عربية فقط',
    };
  }

  if (!isValidRoot(normalizedRoot)) {
    return {
      success: false,
      error: 'الجذر يجب أن يكون 3 أحرف عربية (مثل: كتب، فعل، لعب)',
    };
  }

  const response = await fetchApi<ValidationResult>('/api/validate', {
    method: 'POST',
    body: JSON.stringify({
      word: normalizedWord,
      root: normalizedRoot
    }),
  });

  // CORRECTION CRITIQUE: Convertir valid en boolean de manière robuste
  if (response.data) {
    const validValue = response.data.valid;
    let isValid = false;

    if (typeof validValue === 'boolean') {
      isValid = validValue;
    } else if (typeof validValue === 'string') {
      const lowerValue = validValue.toLowerCase().trim();
      isValid = ['true', '1', 'yes', 'valid', 'صحيح', 'نعم'].includes(lowerValue);
    } else if (typeof validValue === 'number') {
      isValid = validValue === 1 || validValue > 0;
    }

    response.data.valid = isValid;

    if (!response.data.message) {
      response.data.message = isValid ? 'الكلمة صحيحة' : 'الكلمة غير صحيحة';
    }

    console.log('[Validation] Résultat:', isValid, '| Mot:', normalizedWord, '| Racine:', normalizedRoot);
  }

  return response;
}

// ==================== API GAME - CORRIGÉ ====================

export async function getGameQuestions(): Promise<ApiResponse<{
  questions: GameQuestion[];
  total: number;
  pool: number;
}>> {
  return fetchApi('/api/game/start');
}

export async function submitAnswer(
  questionId: number,
  answer: string
): Promise<ApiResponse<{ correct: boolean; correctAnswer: string }>> {
  return fetchApi('/api/game/answer', {
    method: 'POST',
    body: JSON.stringify({
      questionId: questionId.toString(),
      answer: normalizeArabic(answer)
    }),
  });
}

// ==================== UTILITAIRES EXPORTÉS ====================

export const apiUtils = {
  isArabicText,
  isValidRoot,
  normalizeArabic,
};

export default {
  getRoots,
  addRoot,
  deleteRoot,
  getSchemes,
  addScheme,
  generateWords,
  validateWord,
  getGameQuestions,
  submitAnswer,
};
// frontend/lib/api.ts 
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

// CORRECTION: Delete par ID (utilisé par le frontend)
export async function deleteRoot(id: number): Promise<ApiResponse<void>> {
  // Si votre backend attend une valeur au lieu d'un ID, changez ici
  return fetchApi<void>(`/api/roots/${id}`, {
    method: 'DELETE',
  });
}

// Alternative: Delete par valeur si le backend l'attend
export async function deleteRootByValue(value: string): Promise<ApiResponse<void>> {
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

// Update scheme par ID 
export async function updateScheme(
  id: number,
  scheme: {
    name?: string;
    pattern?: string;
    rule?: string;
  }
): Promise<ApiResponse<Scheme>> {
  const body: any = {};

  if (scheme.name) {
    const normalizedName = normalizeArabic(scheme.name);
    if (!isArabicText(normalizedName)) {
      return {
        success: false,
        error: 'اسم الوزن يجب أن يكون بالعربية فقط',
      };
    }
    body.name = normalizedName;
  }

  if (scheme.pattern) {
    const normalizedPattern = normalizeArabic(scheme.pattern);
    const hasFa = normalizedPattern.includes('ف');
    const hasAin = normalizedPattern.includes('ع');
    const hasLam = normalizedPattern.includes('ل');

    if (!hasFa || !hasAin || !hasLam) {
      return {
        success: false,
        error: 'القاعدة يجب أن تحتوي على حروف ف، ع، ل (مثل: فَعَلَ)',
      };
    }
    body.pattern = normalizedPattern;
  }

  if (scheme.rule) {
    body.description = normalizeArabic(scheme.rule);
  }

  return fetchApi<Scheme>(`/api/schemes/${id}`, {
    method: 'PUT',
    body: JSON.stringify(body),
  });
}

// CORRECTION: Delete scheme par ID (manquait !)
export async function deleteScheme(id: number): Promise<ApiResponse<void>> {
  return fetchApi<void>(`/api/schemes/${id}`, {
    method: 'DELETE',
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

  // Convertir valid en boolean de manière robuste
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

// ==================== API GAME ====================

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
  const response = await fetchApi<{ correct: any; correctAnswer: string }>('/api/game/answer', {
    method: 'POST',
    body: JSON.stringify({
      questionId: questionId, // Garder comme number si le backend l'attend
      answer: normalizeArabic(answer)
    }),
  });

  // Normaliser le boolean correct
  if (response.data) {
    const correctValue = response.data.correct;
    let isCorrect = false;

    if (typeof correctValue === 'boolean') {
      isCorrect = correctValue;
    } else if (typeof correctValue === 'string') {
      isCorrect = correctValue.toLowerCase() === 'true' || correctValue === '1';
    } else if (typeof correctValue === 'number') {
      isCorrect = correctValue === 1;
    }

    // Retourner avec correct comme boolean garanti
    return {
      ...response,
      data: {
        correct: isCorrect,
        correctAnswer: response.data.correctAnswer || answer
      }
    };
  }

  return response as ApiResponse<{ correct: boolean; correctAnswer: string }>;
}

// ==================== UTILITAIRES EXPORTÉS ====================

const apiUtils = {
  isArabicText,
  isValidRoot,
  normalizeArabic,
};

export { apiUtils };

export default {
  getRoots,
  addRoot,
  deleteRoot,
  deleteRootByValue,
  getSchemes,
  addScheme,
  updateScheme,
  deleteScheme,
  generateWords,
  validateWord,
  getGameQuestions,
  submitAnswer,
};

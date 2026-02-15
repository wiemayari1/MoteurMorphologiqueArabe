const API_BASE = process.env.NEXT_PUBLIC_API_URL || "http://localhost:8080";

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

// Validation côté client pour l'arabe
export function isArabicText(text: string): boolean {
  if (!text) return false;
  // Regex pour lettres arabes uniquement (sans espaces)
  const arabicRegex = /^[\u0600-\u06FF\u0750-\u077F]+$/;
  return arabicRegex.test(text);
}

export function isValidRoot(text: string): boolean {
  if (!text) return false;
  const normalized = text.trim().replace(/[\u0640]/g, '');
  return isArabicText(normalized) && normalized.length === 3;
}

async function fetchApi<T>(endpoint: string, options?: RequestInit): Promise<ApiResponse<T>> {
  try {
    const url = `${API_BASE}${endpoint}`;
    console.log('Fetching:', url, options?.method || 'GET');
    
    const response = await fetch(url, {
      ...options,
      headers: {
        'Content-Type': 'application/json',
        ...options?.headers,
      },
    });
    
    if (!response.ok) {
      const errorText = await response.text();
      console.error('HTTP Error:', response.status, errorText);
      return {
        success: false,
        error: `خطأ في الاتصال بالخادم: ${response.status}`,
      };
    }
    
    const data = await response.json();
    console.log('Response:', data);
    
    return data;
  } catch (error) {
    console.error('Fetch Error:', error);
    return {
      success: false,
      error: error instanceof Error ? error.message : 'فشل في الاتصال بالخادم',
    };
  }
}

// API Roots
export async function getRoots(): Promise<ApiResponse<Root[]>> {
  return fetchApi<Root[]>('/api/roots');
}

export async function addRoot(value: string): Promise<ApiResponse<Root>> {
  if (!isValidRoot(value)) {
    return {
      success: false,
      error: 'الجذر يجب أن يكون 3 أحرف عربية فقط',
    };
  }
  
  return fetchApi<Root>('/api/roots', {
    method: 'POST',
    body: JSON.stringify({ value }),
  });
}

// CORRECTION: Envoyer la valeur du root au lieu de l'ID
export async function deleteRoot(rootValue: string): Promise<ApiResponse<void>> {
  return fetchApi<void>(`/api/roots/${encodeURIComponent(rootValue)}`, {
    method: 'DELETE',
  });
}

// API Schemes
export async function getSchemes(): Promise<ApiResponse<Scheme[]>> {
  return fetchApi<Scheme[]>('/api/schemes');
}

export async function addScheme(scheme: { name: string; pattern: string; rule?: string }): Promise<ApiResponse<Scheme>> {
  if (!isArabicText(scheme.name)) {
    return {
      success: false,
      error: 'اسم الوزن يجب أن يكون بالعربية',
    };
  }
  
  // Validation: le pattern doit contenir ف، ع، ل
  const validPattern = scheme.pattern.includes('ف') && 
                       scheme.pattern.includes('ع') && 
                       scheme.pattern.includes('ل');
  
  if (!validPattern) {
    return {
      success: false,
      error: 'القاعدة يجب أن تحتوي على ف، ع، ل',
    };
  }
  
  return fetchApi<Scheme>('/api/schemes', {
    method: 'POST',
    body: JSON.stringify({
      name: scheme.name,
      pattern: scheme.pattern,
      description: scheme.rule || scheme.pattern,
    }),
  });
}

// NOUVEAU: Supprimer un schéma
export async function deleteScheme(name: string): Promise<ApiResponse<void>> {
  return fetchApi<void>(`/api/schemes/${encodeURIComponent(name)}`, {
    method: 'DELETE',
  });
}

// NOUVEAU: Modifier un schéma
export async function updateScheme(
  name: string, 
  updates: { pattern?: string; description?: string }
): Promise<ApiResponse<Scheme>> {
  return fetchApi<Scheme>(`/api/schemes/${encodeURIComponent(name)}`, {
    method: 'PUT',
    body: JSON.stringify(updates),
  });
}

// API Generation - CORRECTION: envoyer schemes comme tableau
export async function generateWords(
  root: string, 
  selectedSchemes?: string[]
): Promise<ApiResponse<GenerateResponse>> {
  if (!isValidRoot(root)) {
    return {
      success: false,
      error: 'الرجاء إدخال جذر عربي من 3 أحرف',
    };
  }
  
  const body: any = { root };
  if (selectedSchemes && selectedSchemes.length > 0) {
    // Envoyer comme tableau JSON au lieu de string
    body.schemes = selectedSchemes;
  }
  
  return fetchApi<GenerateResponse>('/api/generate', {
    method: 'POST',
    body: JSON.stringify(body),
  });
}

// API Validation
export async function validateWord(word: string, root: string): Promise<ApiResponse<ValidationResult>> {
  if (!isArabicText(word)) {
    return {
      success: false,
      error: 'الكلمة يجب أن تكون بالعربية فقط',
    };
  }
  if (!isValidRoot(root)) {
    return {
      success: false,
      error: 'الجذر يجب أن يكون 3 أحرف عربية',
    };
  }
  
  return fetchApi<ValidationResult>('/api/validate', {
    method: 'POST',
    body: JSON.stringify({ word, root }),
  });
}

// API Game
export async function getGameQuestions(): Promise<ApiResponse<{ questions: GameQuestion[]; total: number; pool: number }>> {
  return fetchApi('/api/game/start');
}

export async function submitAnswer(questionId: number, answer: string): Promise<ApiResponse<{ correct: boolean; correctAnswer: string }>> {
  return fetchApi('/api/game/answer', {
    method: 'POST',
    body: JSON.stringify({ questionId: questionId.toString(), answer }),
  });
}

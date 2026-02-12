import { Injectable } from '@angular/core';
import { HttpClient, HttpErrorResponse } from '@angular/common/http';
import { Observable, throwError, of } from 'rxjs';
import { catchError, retry, timeout, map } from 'rxjs/operators';
import { environment } from '../../environments/environment';

// ============================================
// INTERFACES (basées sur votre API réelle)
// ============================================

/**
 * Réponse de l'API pour une question de jeu
 * GET /api/game/question
 */
export interface GameQuestion {
  ok: boolean;
  root: string;           // Le mot à deviner (ex: "كتب")
  scheme: string;         // La réponse correcte (ex: "فاعل")
  options: string[];      // Les options possibles
  correct_index: number;  // Index de la réponse correcte
  error?: string;         // Message d'erreur si ok: false
}

/**
 * Réponse de l'API pour la liste des racines
 * GET /api/roots
 */
export interface RootsResponse {
  roots: RootItem[];
  count: number;
}

export interface RootItem {
  root: string;
  id?: number;
}

/**
 * Réponse de l'API pour la liste des schémas
 * GET /api/schemes
 */
export interface SchemesResponse {
  schemes: SchemeItem[];
  count: number;
}

export interface SchemeItem {
  id?: number;
  name: string;
  pattern: string;
}

/**
 * Résultat de validation d'un mot
 * POST /api/validate
 */
export interface ValidationResult {
  isValid: boolean;
  word: string;
  root?: string;
  scheme?: string;
  confidence: number;
  suggestions?: string[];
  error?: string;
}

// ============================================
// SERVICE
// ============================================

@Injectable({
  providedIn: 'root'
})
export class ApiService {
  private apiUrl = environment.apiUrl || 'http://localhost:3001/api';

  constructor(private http: HttpClient) {}

  // ============================================
  // GESTION DES ERREURS
  // ============================================

  private handleError(error: HttpErrorResponse): Observable<never> {
    let errorMessage = 'حدث خطأ غير متوقع';
    
    if (error.error instanceof ErrorEvent) {
      // Erreur client
      errorMessage = `خطأ: ${error.error.message}`;
    } else {
      // Erreur serveur
      switch (error.status) {
        case 0:
          errorMessage = 'لا يمكن الاتصال بالخادم. تأكد من تشغيل الخادم (localhost:3001)';
          break;
        case 404:
          errorMessage = 'الخدمة غير متوفرة حالياً (404)';
          break;
        case 400:
          errorMessage = 'طلب غير صالح (400)';
          break;
        case 500:
          errorMessage = 'خطأ في الخادم (500)';
          break;
        default:
          errorMessage = `خطأ ${error.status}: ${error.message}`;
      }
    }
    
    console.error('API Error:', error);
    return throwError(() => new Error(errorMessage));
  }

  // ============================================
  // GAME (اللعبة)
  // ============================================

  /**
   * Récupère une question de jeu
   * GET /api/game/question
   */
  getGameQuestion(): Observable<GameQuestion> {
    return this.http.get<GameQuestion>(`${this.apiUrl}/game/question`).pipe(
      timeout(5000),
      retry(1),
      catchError(this.handleError)
    );
  }

  /**
   * Vérifie la réponse d'une question
   * POST /api/game/verify
   */
  verifyAnswer(root: string, selectedScheme: string): Observable<{correct: boolean; correctScheme: string}> {
    return this.http.post<{correct: boolean; correctScheme: string}>(
      `${this.apiUrl}/game/verify`, 
      { root, selectedScheme }
    ).pipe(
      timeout(5000),
      catchError(this.handleError)
    );
  }

  // ============================================
  // ROOTS (الجذور)
  // ============================================

  /**
   * Récupère toutes les racines arabes
   * GET /api/roots
   */
  getRoots(): Observable<RootsResponse> {
    return this.http.get<RootsResponse>(`${this.apiUrl}/roots`).pipe(
      timeout(5000),
      retry(1),
      catchError(this.handleError)
    );
  }

  /**
   * Ajoute une nouvelle racine
   * POST /api/roots
   */
  addRoot(root: string): Observable<RootItem> {
    return this.http.post<RootItem>(`${this.apiUrl}/roots`, { root }).pipe(
      timeout(5000),
      catchError(this.handleError)
    );
  }

  /**
   * Supprime une racine par ID
   * DELETE /api/roots/:id
   */
  deleteRoot(id: number): Observable<any> {
    return this.http.delete(`${this.apiUrl}/roots/${id}`).pipe(
      timeout(5000),
      catchError(this.handleError)
    );
  }

  /**
   * Recherche des racines
   * GET /api/roots/search?q=...
   */
  searchRoots(query: string): Observable<RootItem[]> {
    return this.http.get<RootItem[]>(`${this.apiUrl}/roots/search`, {
      params: { q: query }
    }).pipe(
      timeout(5000),
      catchError(this.handleError)
    );
  }

  // ============================================
  // SCHEMES (الأوزان)
  // ============================================

  /**
   * Récupère tous les schémas (أوزان)
   * GET /api/schemes
   */
  getSchemes(): Observable<SchemesResponse> {
    return this.http.get<SchemesResponse>(`${this.apiUrl}/schemes`).pipe(
      timeout(5000),
      retry(1),
      catchError(this.handleError)
    );
  }

  /**
   * Ajoute un nouveau schéma
   * POST /api/schemes
   */
  addScheme(scheme: SchemeItem): Observable<SchemeItem> {
    return this.http.post<SchemeItem>(`${this.apiUrl}/schemes`, scheme).pipe(
      timeout(5000),
      catchError(this.handleError)
    );
  }

  /**
   * Met à jour un schéma
   * PUT /api/schemes/:id
   */
  updateScheme(id: number, scheme: SchemeItem): Observable<SchemeItem> {
    return this.http.put<SchemeItem>(`${this.apiUrl}/schemes/${id}`, scheme).pipe(
      timeout(5000),
      catchError(this.handleError)
    );
  }

  /**
   * Supprime un schéma
   * DELETE /api/schemes/:id
   */
  deleteScheme(id: number): Observable<any> {
    return this.http.delete(`${this.apiUrl}/schemes/${id}`).pipe(
      timeout(5000),
      catchError(this.handleError)
    );
  }

  // ============================================
  // GENERATE (التوليد)
  // ============================================

  /**
   * Génère des mots à partir d'une racine et d'un schéma
   * POST /api/generate
   */
  generateWords(root: string, scheme: string): Observable<string[]> {
    return this.http.post<string[]>(`${this.apiUrl}/generate`, { 
      root, 
      scheme 
    }).pipe(
      timeout(10000), // Plus long pour la génération
      catchError(this.handleError)
    );
  }

  /**
   * Génère avec options avancées
   */
  generateWordsAdvanced(
    root: string, 
    scheme: string, 
    options: {
      includeDerivatives?: boolean;
      includeConjugations?: boolean;
      maxResults?: number;
    } = {}
  ): Observable<string[]> {
    return this.http.post<string[]>(`${this.apiUrl}/generate`, {
      root,
      scheme,
      options
    }).pipe(
      timeout(10000),
      catchError(this.handleError)
    );
  }

  // ============================================
  // VALIDATE (التحقق)
  // ============================================

  /**
   * Valide la morphologie d'un mot
   * POST /api/validate
   */
  validateWord(word: string): Observable<ValidationResult> {
    return this.http.post<ValidationResult>(`${this.apiUrl}/validate`, { word }).pipe(
      timeout(5000),
      catchError(this.handleError)
    );
  }

  /**
   * Valide avec détails complets
   */
  validateWordDetailed(word: string): Observable<ValidationResult> {
    return this.http.post<ValidationResult>(`${this.apiUrl}/validate/detailed`, { word }).pipe(
      timeout(5000),
      catchError(this.handleError)
    );
  }

  // ============================================
  // STATS & HEALTH
  // ============================================

  /**
   * Vérifie si l'API est en ligne
   * GET /api/health
   */
  healthCheck(): Observable<{status: string; timestamp: string}> {
    return this.http.get<{status: string; timestamp: string}>(`${this.apiUrl}/health`).pipe(
      timeout(3000),
      catchError(() => of({ status: 'offline', timestamp: new Date().toISOString() }))
    );
  }

  /**
   * Récupère les statistiques
   * GET /api/stats
   */
  getStats(): Observable<{
    totalRoots: number;
    totalSchemes: number;
    totalGenerations: number;
    lastUpdate: string;
  }> {
    return this.http.get<{
      totalRoots: number;
      totalSchemes: number;
      totalGenerations: number;
      lastUpdate: string;
    }>(`${this.apiUrl}/stats`).pipe(
      timeout(5000),
      catchError(this.handleError)
    );
  }
}

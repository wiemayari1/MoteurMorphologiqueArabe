import { Injectable } from '@angular/core';
import { HttpClient, HttpErrorResponse } from '@angular/common/http';
import { Observable, throwError, timeout, catchError } from 'rxjs';

// Interfaces
export interface GenerateResponse {
  ok: boolean;
  word?: string;
  root?: string;
  scheme?: string;
  error?: string;
}

export interface ValidateResponse {
  ok: boolean;
  belongs?: boolean;
  schemes?: string[];
  error?: string;
}

export interface GameQuestion {
  ok: boolean;
  root: string;
  scheme: string;
  options: string[];
  correct_index: number;
  error?: string;
}

export interface RootItem {
  root: string;
  meaning?: string;
}

export interface RootsResponse {
  ok: boolean;
  roots: RootItem[] | string[];
  error?: string;
}

export interface SchemeItem {
  name: string;
  template: string;  // pattern comme "1َ2َ3َ"
}

export interface SchemesResponse {
  ok: boolean;
  schemes: SchemeItem[];
  error?: string;
}

export interface ApiResponse {
  ok: boolean;
  error?: string;
}

export interface DerivativeItem {
  word: string;
  scheme: string;
  timestamp: number;
}

export interface DerivativesResponse {
  ok: boolean;
  derivatives: DerivativeItem[];
  error?: string;
}

@Injectable({
  providedIn: 'root'
})
export class ApiService {
  // On utilise l'URL absolue pour éviter les problèmes de proxy en dev
  private apiUrl = 'http://localhost:3001/api';

  constructor(private http: HttpClient) { }

  // ========== GESTION DES ERREURS ==========
  private handleError(error: HttpErrorResponse) {
    let errorMsg = 'خطأ في الاتصال بالخادم';
    if (error.error instanceof ErrorEvent) {
      errorMsg = `خطأ: ${error.error.message}`;
    } else {
      errorMsg = `رمز الخطأ: ${error.status}, الرسالة: ${error.message}`;
    }
    console.error('API Error:', error);
    return throwError(() => new Error(errorMsg));
  }

  // ========== DERIVATIVES (Dérivés validés) ==========
  getDerivatives(root: string): Observable<DerivativesResponse> {
    return this.http.get<DerivativesResponse>(
      `${this.apiUrl}/derivatives/${encodeURIComponent(root.trim())}`
    ).pipe(catchError(this.handleError));
  }

  // ========== RACINES (الجذور) ==========

  // Récupérer toutes les racines
  getRoots(): Observable<RootsResponse> {
    return this.http.get<RootsResponse>(`${this.apiUrl}/roots`)
      .pipe(catchError(this.handleError));
  }

  // Alias pour compatibilité
  listRoots(): Observable<RootsResponse> {
    return this.getRoots();
  }

  // Ajouter une racine
  addRoot(root: string, meaning: string = ''): Observable<ApiResponse> {
    return this.http.post<ApiResponse>(`${this.apiUrl}/roots`, {
      root: root.trim(),
      meaning: meaning.trim()
    }).pipe(catchError(this.handleError));
  }

  // NOUVEAU: Supprimer une racine
  deleteRoot(root: string): Observable<ApiResponse> {
    return this.http.delete<ApiResponse>(
      `${this.apiUrl}/roots/${encodeURIComponent(root.trim())}`
    ).pipe(catchError(this.handleError));
  }

  // NOUVEAU: Rechercher une racine
  searchRoot(query: string): Observable<RootsResponse> {
    return this.http.get<RootsResponse>(
      `${this.apiUrl}/roots/search?q=${encodeURIComponent(query.trim())}`
    ).pipe(catchError(this.handleError));
  }

  // ========== SCHÉMAS (الأوزان) ==========

  // Récupérer tous les schémas
  getSchemes(): Observable<SchemesResponse> {
    return this.http.get<SchemesResponse>(`${this.apiUrl}/schemes`)
      .pipe(catchError(this.handleError));
  }

  // Alias pour compatibilité
  listSchemes(): Observable<SchemesResponse> {
    return this.getSchemes();
  }

  // Ajouter un nouveau schéma
  addScheme(name: string, template: string): Observable<ApiResponse> {
    return this.http.post<ApiResponse>(`${this.apiUrl}/schemes`, {
      name: name.trim(),
      pattern: template.trim()  // Le backend attend "pattern" pas "template"
    }).pipe(catchError(this.handleError));
  }

  // Mettre à jour un schéma existant
  updateScheme(name: string, template: string): Observable<ApiResponse> {
    return this.http.put<ApiResponse>(
      `${this.apiUrl}/schemes/${encodeURIComponent(name.trim())}`,
      { pattern: template.trim() }
    ).pipe(catchError(this.handleError));
  }

  // Supprimer un schéma
  deleteScheme(name: string): Observable<ApiResponse> {
    return this.http.delete<ApiResponse>(
      `${this.apiUrl}/schemes/${encodeURIComponent(name.trim())}`
    ).pipe(catchError(this.handleError));
  }

  // ========== GÉNÉRATION (التوليد) ==========

  generate(data: { root: string; scheme: string }): Observable<GenerateResponse> {
    return this.http.post<GenerateResponse>(`${this.apiUrl}/generate`, {
      root: data.root.trim(),
      scheme: data.scheme.trim()
    }).pipe(
      timeout(30000), // 30s timeout for engine
      catchError(this.handleError)
    );
  }

  // ========== VALIDATION (التحقق) ==========

  validate(data: { word: string; root: string }): Observable<ValidateResponse> {
    return this.http.post<ValidateResponse>(`${this.apiUrl}/validate`, {
      word: data.word.trim(),
      root: data.root.trim()
    }).pipe(catchError(this.handleError));
  }

  // ========== JEU (اللعبة) ==========

  getGameQuestion(): Observable<GameQuestion> {
    return this.http.get<GameQuestion>(`${this.apiUrl}/game/question`)
      .pipe(
        timeout(10000), // 10s timeout
        catchError(this.handleError)
      );
  }

  // NOUVEAU: Vérifier la réponse du jeu
  checkGameAnswer(word: string, root: string): Observable<{ ok: boolean; correct: boolean }> {
    return this.http.post<{ ok: boolean; correct: boolean }>(
      `${this.apiUrl}/game/check`,
      { word, root }
    ).pipe(
      timeout(10000),
      catchError(this.handleError)
    );
  }
}

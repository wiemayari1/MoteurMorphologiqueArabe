import { Injectable } from '@angular/core';
import { HttpClient, HttpErrorResponse } from '@angular/common/http';
import { Observable, throwError } from 'rxjs';
import { catchError, retry, timeout } from 'rxjs/operators';
import { environment } from '../../environments/environment';

// Interfaces pour typer les réponses
export interface GameQuestion {
  id: number;
  word: string;
  correctAnswer: string;
  options: string[];
  type: 'conjugation' | 'derivation';
}

export interface Root {
  id?: number;
  root: string;
}

export interface Scheme {
  id?: number;
  name: string;
  pattern: string;
}

export interface RootsResponse {
  roots: Root[];
  count: number;
}

export interface SchemesResponse {
  schemes: Scheme[];
  count: number;
}

@Injectable({
  providedIn: 'root'
})
export class ApiService {
  private apiUrl = environment.apiUrl || 'http://localhost:3001/api';

  constructor(private http: HttpClient) {}

  private handleError(error: HttpErrorResponse): Observable<never> {
    let errorMessage = 'حدث خطأ غير متوقع';
    
    if (error.error instanceof ErrorEvent) {
      errorMessage = `خطأ: ${error.error.message}`;
    } else {
      switch (error.status) {
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

  // Game - CORRIGÉ: méthode au pluriel
  getGameQuestions(): Observable<GameQuestion[]> {
    return this.http.get<GameQuestion[]>(`${this.apiUrl}/game/questions`).pipe(
      timeout(5000),
      retry(1),
      catchError(this.handleError)
    );
  }

  // Garder aussi la version singulière si elle existe
  getGameQuestion(): Observable<GameQuestion> {
    return this.http.get<GameQuestion>(`${this.apiUrl}/game/question`).pipe(
      timeout(5000),
      catchError(this.handleError)
    );
  }

  // Roots - CORRIGÉ: typer la réponse
  getRoots(): Observable<RootsResponse> {
    return this.http.get<RootsResponse>(`${this.apiUrl}/roots`).pipe(
      timeout(5000),
      retry(1),
      catchError(this.handleError)
    );
  }

  addRoot(root: string): Observable<any> {
    return this.http.post(`${this.apiUrl}/roots`, { root }).pipe(
      timeout(5000),
      catchError(this.handleError)
    );
  }

  deleteRoot(id: number): Observable<any> {
    return this.http.delete(`${this.apiUrl}/roots/${id}`).pipe(
      timeout(5000),
      catchError(this.handleError)
    );
  }

  // Schemes - CORRIGÉ: typer la réponse
  getSchemes(): Observable<SchemesResponse> {
    return this.http.get<SchemesResponse>(`${this.apiUrl}/schemes`).pipe(
      timeout(5000),
      retry(1),
      catchError(this.handleError)
    );
  }

  addScheme(scheme: Scheme): Observable<any> {
    return this.http.post(`${this.apiUrl}/schemes`, scheme).pipe(
      timeout(5000),
      catchError(this.handleError)
    );
  }

  // Generate
  generateWords(root: string, scheme: string): Observable<string[]> {
    return this.http.post<string[]>(`${this.apiUrl}/generate`, { root, scheme }).pipe(
      timeout(10000),
      catchError(this.handleError)
    );
  }

  // Validate
  validateWord(word: string): Observable<any> {
    return this.http.post(`${this.apiUrl}/validate`, { word }).pipe(
      timeout(5000),
      catchError(this.handleError)
    );
  }
}

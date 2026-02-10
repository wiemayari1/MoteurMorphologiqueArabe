import { Injectable } from '@angular/core';
import { HttpClient } from '@angular/common/http';
import { environment } from '../../environments/environment';

export type GenerateRequest = { root: string; scheme: string };
export type GenerateResponse = { ok: boolean; word?: string; error?: string };

export type ValidateRequest = { word: string; root: string };
export type ValidateResponse = { ok: boolean; valid?: boolean; scheme?: string; error?: string };

@Injectable({ providedIn: 'root' })
export class ApiService {
  private base = environment.apiBaseUrl;

  constructor(private http: HttpClient) {}

  generate(req: GenerateRequest) {
    return this.http.post<GenerateResponse>(`${this.base}/api/generate`, req);
  }

  validate(req: ValidateRequest) {
    return this.http.post<ValidateResponse>(`${this.base}/api/validate`, req);
  }

  // optionnel si tu ajoutes ces endpoints côté API
  listRoots() {
    return this.http.get<{ ok: boolean; roots: string[] }>(`${this.base}/api/roots`);
  }
  addRoot(root: string) {
    return this.http.post<{ ok: boolean }>(`${this.base}/api/roots`, { root });
  }
  listSchemes() {
    return this.http.get<{ ok: boolean; schemes: { name: string; templ: string }[] }>(
      `${this.base}/api/schemes`
    );
  }
  upsertScheme(name: string, templ: string) {
    return this.http.post<{ ok: boolean }>(`${this.base}/api/schemes`, { name, templ });
  }
  deleteScheme(name: string) {
    return this.http.delete<{ ok: boolean }>(`${this.base}/api/schemes/${encodeURIComponent(name)}`);
  }
}

import { Injectable } from '@angular/core';
import { HttpClient } from '@angular/common/http';
import { environment } from '../../environments/environment';

@Injectable({ providedIn: 'root' })
export class ApiService {
  private base = environment.apiBaseUrl;

  constructor(private http: HttpClient) {}

  generate(root: string, scheme: string) {
    return this.http.post<any>(`${this.base}/api/generate`, { root, scheme });
  }

  validate(word: string, root: string) {
    return this.http.post<any>(`${this.base}/api/validate`, { word, root });
  }
}

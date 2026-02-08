import { Injectable } from '@angular/core';
import { Observable, of } from 'rxjs';

export interface GenerationRequest {
  racine: string;
  scheme: string;
}

export interface GenerationResponse {
  mot: string;
}

@Injectable({
  providedIn: 'root'
})
export class MorphoService {

  constructor() { }

  genererMot(req: GenerationRequest): Observable<GenerationResponse> {
    // Simulation simple
    let mot = 'مَكْتُوب';

    if (req.racine.trim() === 'كتب' && req.scheme.trim() === 'مفعول') {
      mot = 'مكتوب';
    }

    return of({ mot });
  }

  validerMot(mot: string, racine: string): Observable<{appartient: boolean, scheme?: string}> {
    if (mot.trim() === 'مكتوب' && racine.trim() === 'كتب') {
      return of({ appartient: true, scheme: 'مفعول' });
    }
    return of({ appartient: false });
  }

  getQuestionMinijeu(): Observable<{question: string, bonneReponse: string}> {
    return of({
      question: 'أعطِ جذر الكلمة مكتوب',
      bonneReponse: 'كتب'
    });
  }
}

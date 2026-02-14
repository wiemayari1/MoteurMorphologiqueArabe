import { Component, OnInit, OnDestroy } from '@angular/core';
import { CommonModule } from '@angular/common';
import { FormsModule } from '@angular/forms';
import { Subject, forkJoin, takeUntil, catchError, of, timeout } from 'rxjs';
import { debounceTime } from 'rxjs/operators';
import { MatCardModule } from '@angular/material/card';
import { MatButtonModule } from '@angular/material/button';
import { MatInputModule } from '@angular/material/input';
import { MatSelectModule } from '@angular/material/select';
import { MatProgressSpinnerModule } from '@angular/material/progress-spinner';
import { MatIconModule } from '@angular/material/icon';
import { MatChipsModule } from '@angular/material/chips';
import { MatTooltipModule } from '@angular/material/tooltip';

import { PageShellComponent } from '../../shared/page-shell/page-shell.component';
import { ApiService, SchemeItem, DerivativeItem, GenerateResponse } from '../../services/api.service';

// Interface pour les résultats enrichis
export interface GeneratedResult {
  word: string;
  root: string;
  schemeName: string;
  schemeTemplate: string;
}

@Component({
  selector: 'app-generate-page',
  standalone: true,
  imports: [
    CommonModule,
    FormsModule,
    MatCardModule,
    MatButtonModule,
    MatInputModule,
    MatSelectModule,
    MatProgressSpinnerModule,
    MatIconModule,
    MatChipsModule,
    MatTooltipModule,
    PageShellComponent
  ],
  templateUrl: './generate-page.component.html',
  styleUrls: ['./generate-page.component.scss']
})
export class GeneratePageComponent implements OnInit, OnDestroy {
  // Données du formulaire
  roots: string[] = [];
  schemes: SchemeItem[] = [];
  
  selectedRoot: string = '';
  selectedSchemes: SchemeItem[] = [];
  customRoot: string = '';
  customScheme: string = '';

  // Résultats
  generatedResults: GeneratedResult[] = [];
  derivatives: DerivativeItem[] = [];

  // États
  loading = false;
  loadingData = true;
  error: string | null = null;
  loadError: string | null = null;

  // RxJS
  private destroy$ = new Subject<void>();
  private inputSubject = new Subject<void>();

  constructor(private apiService: ApiService) {
    // Debounce pour la génération automatique
    this.inputSubject.pipe(
      debounceTime(600),
      takeUntil(this.destroy$)
    ).subscribe(() => {
      if (this.canGenerate && !this.loading) {
        this.generate();
      }
    });
  }

  ngOnInit(): void {
    this.loadInitialData();
    
    // Timeout de sécurité
    setTimeout(() => {
      if (this.loadingData) {
        this.loadingData = false;
        this.loadError = 'انتهت مهلة الاتصال. تأكد من تشغيل الخادم على المنفذ 3001';
      }
    }, 15000);
  }

  ngOnDestroy(): void {
    this.destroy$.next();
    this.destroy$.complete();
  }

  // ========== CHARGEMENT DES DONNÉES ==========

  loadInitialData(): void {
    console.log('Chargement des données...');
    this.loadingData = true;
    this.loadError = null;

    forkJoin({
      rootsResponse: this.apiService.getRoots().pipe(
        timeout(10000),
        catchError(err => {
          console.error('Erreur chargement racines:', err);
          return of({ ok: false, roots: [], error: err.message });
        })
      ),
      schemesResponse: this.apiService.getSchemes().pipe(
        timeout(10000),
        catchError(err => {
          console.error('Erreur chargement schèmes:', err);
          return of({ ok: false, schemes: [], error: err.message });
        })
      )
    }).pipe(
      takeUntil(this.destroy$)
    ).subscribe({
      next: ({ rootsResponse, schemesResponse }) => {
        console.log('Réponse racines:', rootsResponse);
        console.log('Réponse schèmes:', schemesResponse);

        // Traitement des racines
        if (rootsResponse.ok && rootsResponse.roots) {
          this.roots = this.extractRoots(rootsResponse.roots);
        } else {
          console.warn('Échec chargement racines:', rootsResponse.error);
        }

        // Traitement des schèmes
        if (schemesResponse.ok && schemesResponse.schemes) {
          this.schemes = schemesResponse.schemes;
        } else {
          console.warn('Échec chargement schèmes:', schemesResponse.error);
        }

        this.loadingData = false;

        // Vérifier si on a des données
        if (this.roots.length === 0 && this.schemes.length === 0) {
          this.loadError = 'لا يمكن تحميل البيانات. تأكد من تشغيل الخادم.';
        }
      },
      error: (err) => {
        console.error('Erreur forkJoin:', err);
        this.loadingData = false;
        this.loadError = 'فشل الاتصال بالخادم: ' + (err.message || 'خطأ غير معروف');
      }
    });
  }

  private extractRoots(rootsData: any[]): string[] {
    if (!Array.isArray(rootsData)) return [];
    
    return rootsData
      .map((item: any) => {
        if (typeof item === 'string') return item.trim();
        if (item && typeof item === 'object') {
          return (item.root || item.value || item.name || '').trim();
        }
        return '';
      })
      .filter((r: string) => r.length > 0)
      .sort((a: string, b: string) => a.localeCompare(b, 'ar'));
  }

  // ========== GESTION DES ÉVÉNEMENTS ==========

  onInputChange(): void {
    this.error = null;
    this.inputSubject.next();
  }

  onRootChange(): void {
    this.loadDerivatives();
    this.onInputChange();
  }

  // ========== CHARGEMENT DES DÉRIVÉS ==========

  loadDerivatives(): void {
    const root = (this.selectedRoot || this.customRoot).trim();
    if (!root) {
      this.derivatives = [];
      return;
    }

    this.apiService.getDerivatives(root).pipe(
      timeout(8000),
      takeUntil(this.destroy$),
      catchError(err => {
        console.error('Erreur chargement dérivés:', err);
        return of({ ok: false, derivatives: [] });
      })
    ).subscribe({
      next: (res) => {
        if (res.ok) {
          this.derivatives = res.derivatives || [];
        }
      }
    });
  }

  // ========== GÉNÉRATION ==========

  get canGenerate(): boolean {
    const hasRoot = !!(this.selectedRoot || this.customRoot)?.trim();
    const hasScheme = this.selectedSchemes.length > 0 || !!this.customScheme?.trim();
    return hasRoot && hasScheme && !this.loading;
  }

  generate(): void {
    const root = (this.selectedRoot || this.customRoot).trim();
    
    // Préparer les schèmes à traiter
    const schemesToProcess: { name: string; template: string }[] = [
      ...this.selectedSchemes.map(s => ({ name: s.name, template: s.template })),
      ...(this.customScheme?.trim() ? [{ name: 'مخصص', template: this.customScheme.trim() }] : [])
    ];

    // Validations
    if (!root) {
      this.error = 'الرجاء اختيار الجذر';
      return;
    }
    if (schemesToProcess.length === 0) {
      this.error = 'الرجاء اختيار وزن واحد على الأقل';
      return;
    }
    if (this.customRoot && !/^[\u0600-\u06FF]{3,4}$/.test(this.customRoot.trim())) {
      this.error = 'الجذر يجب أن يكون 3 أو 4 أحرف عربية';
      return;
    }

    this.loading = true;
    this.error = null;
    this.generatedResults = [];

    // Appels API parallèles
    const requests = schemesToProcess.map(scheme => 
      this.apiService.generate({ root, scheme: scheme.template }).pipe(
        timeout(20000),
        catchError(err => {
          console.error(`Erreur génération ${scheme.name}:`, err);
          return of({ ok: false, error: err.message } as GenerateResponse);
        })
      )
    );

    forkJoin(requests).pipe(
      takeUntil(this.destroy$)
    ).subscribe({
      next: (responses) => {
        this.loading = false;
        const results: GeneratedResult[] = [];

        responses.forEach((res, index) => {
          if (res.ok && res.word) {
            results.push({
              word: res.word,
              root: root,
              schemeName: schemesToProcess[index].name,
              schemeTemplate: schemesToProcess[index].template
            });
          }
        });

        this.generatedResults = results;

        if (results.length === 0) {
          this.error = 'فشل توليد الكلمات. تحقق من صحة الجذر والوزن.';
        } else if (results.length < schemesToProcess.length) {
          console.warn(`Partiel: ${results.length}/${schemesToProcess.length} générés`);
        }

        // Recharger la famille morphologique
        this.loadDerivatives();
      },
      error: (err) => {
        this.loading = false;
        this.error = 'خطأ في الاتصال: ' + (err.message || 'فشل التوليد');
        console.error('Erreur génération:', err);
      }
    });
  }

  // ========== UTILITAIRES ==========

  reset(): void {
    this.selectedRoot = '';
    this.selectedSchemes = [];
    this.customRoot = '';
    this.customScheme = '';
    this.generatedResults = [];
    this.derivatives = [];
    this.error = null;
  }

  copyWord(word: string): void {
    navigator.clipboard.writeText(word).then(
      () => console.log('Mot copié:', word),
      (err) => console.error('Erreur copie:', err)
    );
  }

  copyAllWords(): void {
    const words = this.generatedResults.map(r => r.word).join('\n');
    navigator.clipboard.writeText(words);
  }

  // Vérifier si un mot existe déjà dans les dérivés
  isExistingWord(word: string): boolean {
    return this.derivatives.some(d => d.word === word);
  }
}

import { Component, OnInit } from '@angular/core';
import { CommonModule } from '@angular/common';
import { FormsModule } from '@angular/forms';
import { Subject, debounceTime, forkJoin, timeout, finalize, takeUntil, of } from 'rxjs';
import { MatCardModule } from '@angular/material/card';
import { MatButtonModule } from '@angular/material/button';
import { MatInputModule } from '@angular/material/input';
import { MatSelectModule } from '@angular/material/select';
import { MatProgressSpinnerModule } from '@angular/material/progress-spinner';
import { MatIconModule } from '@angular/material/icon';
import { MatChipsModule } from '@angular/material/chips';
import { MatTooltipModule } from '@angular/material/tooltip';

import { PageShellComponent } from '../../shared/page-shell/page-shell.component';
import { ApiService, SchemeItem, DerivativeItem } from '../../services/api.service';
import { MatSnackBar, MatSnackBarModule } from '@angular/material/snack-bar';

// Interface pour les résultats de génération enrichis
export interface GeneratedWordResult {
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
    MatSnackBarModule,
    MatChipsModule,
    MatTooltipModule,
    PageShellComponent
  ],
  templateUrl: './generate-page.component.html',
  styleUrls: ['./generate-page.component.scss']
})
export class GeneratePageComponent implements OnInit {
  roots: string[] = [];
  schemes: SchemeItem[] = [];

  selectedRoot: string = '';
  // Multi-select for schemes
  selectedSchemes: SchemeItem[] = [];

  customRoot: string = '';
  customScheme: string = '';

  // Résultats enrichis avec racine, schème et mot
  generatedResults: GeneratedWordResult[] = [];
  
  // Historique des dérivés (famille morphologique complète)
  derivatives: DerivativeItem[] = [];

  loading = false;
  loadingData = true;
  error: string | null = null;
  loadError: string | null = null;

  // Pour le nettoyage des subscriptions
  private destroy$ = new Subject<void>();
  
  // Subject pour le debounce de la génération
  private generateSubject = new Subject<{ root: string; schemes: SchemeItem[] }>();

  constructor(private apiService: ApiService, private snackBar: MatSnackBar) {
    // Configuration du debounce pour la génération automatique
    this.generateSubject.pipe(
      debounceTime(400),
      takeUntil(this.destroy$)
    ).subscribe(({ root, schemes }) => {
      if (root && schemes.length > 0) {
        this.executeGeneration(root, schemes);
      }
    });
  }

  ngOnInit() {
    this.loadData();
    
    // Safety timeout
    setTimeout(() => {
      if (this.loadingData) {
        this.loadingData = false;
        this.loadError = 'يبدو أن الخادم لا يستجيب. يرجى التأكد من تشغيله.';
      }
    }, 5000);
  }

  ngOnDestroy() {
    this.destroy$.next();
    this.destroy$.complete();
  }

  // Déclenche la génération avec debounce
  onInputChange() {
    this.error = null;
    this.triggerGeneration();
  }

  onRootChange() {
    this.loadDerivatives();
    this.onInputChange();
  }

  // Déclencheur de génération (manuel ou automatique)
  private triggerGeneration() {
    const root = (this.selectedRoot || this.customRoot).trim();
    const schemes = [...this.selectedSchemes];
    
    if (this.customScheme?.trim()) {
      schemes.push({ 
        name: 'مخصص', 
        template: this.customScheme.trim(),
        pattern: this.customScheme.trim()
      });
    }

    if (root && schemes.length > 0) {
      this.generateSubject.next({ root, schemes });
    }
  }

  // Vérification si on peut générer
  get canGenerate(): boolean {
    const hasRoot = (this.selectedRoot || this.customRoot)?.trim().length > 0;
    const hasScheme = this.selectedSchemes.length > 0 || this.customScheme?.trim().length > 0;
    return hasRoot && hasScheme && !this.loading;
  }

  // Génération manuelle (bouton)
  generate() {
    const root = (this.selectedRoot || this.customRoot).trim();
    
    let schemesToProcess: SchemeItem[] = [...this.selectedSchemes];
    if (this.customScheme?.trim()) {
      schemesToProcess.push({ 
        name: 'مخصص', 
        template: this.customScheme.trim(),
        pattern: this.customScheme.trim()
      });
    }

    if (!root) {
      this.error = 'الرجاء اختيار الجذر';
      return;
    }

    if (schemesToProcess.length === 0) {
      this.error = 'الرجاء اختيار وزن واحد على الأقل';
      return;
    }

    if (this.customRoot && !/^[\u0600-\u06FF\s]+$/.test(this.customRoot)) {
      this.error = 'الجذر يجب أن يحتوي على أحرف عربية فقط';
      return;
    }

    this.executeGeneration(root, schemesToProcess);
  }

  // Exécution réelle de la génération
  private executeGeneration(root: string, schemes: SchemeItem[]) {
    this.loading = true;
    this.error = null;
    this.generatedResults = [];

    const requests = schemes.map(s => 
      this.apiService.generate({ root, scheme: s.template }).pipe(
        timeout(5000) // Timeout par requête
      )
    );

    forkJoin(requests).pipe(
      finalize(() => {
        this.loading = false;
      })
    ).subscribe({
      next: (responses) => {
        const results: GeneratedWordResult[] = [];

        responses.forEach((res, index) => {
          const scheme = schemes[index];
          if (res.ok && res.word) {
            results.push({
              word: res.word,
              root: root,
              schemeName: scheme.name,
              schemeTemplate: scheme.template
            });
          }
        });

        this.generatedResults = results;

        if (results.length === 0) {
          this.error = 'فشل التوليد لجميع الأوزان المختارة';
        } else if (results.length < schemes.length) {
          // Certains ont échoué mais pas tous
          this.snackBar.open(
            `تم توليد ${results.length} من ${schemes.length} كلمات`, 
            'إغلاق', 
            { duration: 3000 }
          );
        }

        // Recharger l'historique
        this.loadDerivatives();
      },
      error: (err) => {
        this.loading = false;
        this.error = 'خطأ في الاتصال بالخادم';
        console.error('Generation error:', err);
      }
    });
  }

  loadData() {
    this.loadingData = true;
    this.loadError = null;

    forkJoin({
      roots: this.apiService.getRoots(),
      schemes: this.apiService.getSchemes()
    }).pipe(
      timeout(8000),
      finalize(() => {
        this.loadingData = false;
      })
    ).subscribe({
      next: (results) => {
        // Traitement des racines
        const rootsResponse = results.roots as any;
        let rootItems: any[] = [];
        
        if (Array.isArray(rootsResponse)) {
          rootItems = rootsResponse;
        } else if (rootsResponse?.roots && Array.isArray(rootsResponse.roots)) {
          rootItems = rootsResponse.roots;
        }

        this.roots = rootItems
          .map((item: any) => {
            if (typeof item === 'string') return item.trim();
            if (item && typeof item === 'object') return (item.root || '').trim();
            return '';
          })
          .filter((r: string) => r.length > 0)
          .sort((a: string, b: string) => a.localeCompare(b, 'ar'));

        // Traitement des schèmes
        const schemesResponse = results.schemes as any;
        let schemeItems: any[] = [];
        
        if (Array.isArray(schemesResponse)) {
          schemeItems = schemesResponse;
        } else if (schemesResponse?.schemes && Array.isArray(schemesResponse.schemes)) {
          schemeItems = schemesResponse.schemes;
        }

        this.schemes = schemeItems.map((item: any) => {
          const obj = item && typeof item === 'object' ? item : {};
          return {
            name: obj.name ?? obj.scheme_name ?? 'وزن غير معروف',
            template: obj.template ?? obj.pattern ?? obj.scheme ?? '؟؟؟',
            pattern: obj.pattern ?? obj.template ?? '؟؟؟'
          };
        });
      },
      error: (err) => {
        this.loadError = 'فشل تحميل البيانات. يرجى التأكد من تشغيل الخادم.';
        console.error('Load data error:', err);
      }
    });
  }

  loadDerivatives() {
    const root = (this.selectedRoot || this.customRoot).trim();
    if (!root) {
      this.derivatives = [];
      return;
    }

    this.apiService.getDerivatives(root).pipe(
      timeout(5000),
      takeUntil(this.destroy$)
    ).subscribe({
      next: (res) => {
        if (res.ok) {
          this.derivatives = res.derivatives || [];
        }
      },
      error: (err) => {
        console.warn('Error loading derivatives:', err);
        this.derivatives = [];
      }
    });
  }

  // Obtenir la famille morphologique (dérivés de même racine)
  get morphologicalFamily(): DerivativeItem[] {
    return this.derivatives.filter(d => 
      d.root === (this.selectedRoot || this.customRoot)
    );
  }

  reset() {
    this.selectedRoot = '';
    this.selectedSchemes = [];
    this.customRoot = '';
    this.customScheme = '';
    this.generatedResults = [];
    this.derivatives = [];
    this.error = null;
    this.loadError = null;
  }

  copyWord(word: string) {
    navigator.clipboard.writeText(word).then(
      () => this.snackBar.open('تم نسخ الكلمة: ' + word, 'إغلاق', { duration: 2000 }),
      () => this.snackBar.open('فشل النسخ', 'إغلاق', { duration: 2000 })
    );
  }

  // Copier tous les résultats
  copyAllWords() {
    const words = this.generatedResults.map(r => r.word).join('\n');
    navigator.clipboard.writeText(words).then(
      () => this.snackBar.open(`تم نسخ ${this.generatedResults.length} كلمات`, 'إغلاق', { duration: 2000 }),
      () => this.snackBar.open('فشل النسخ', 'إغلاق', { duration: 2000 })
    );
  }

  // Vérifier si un mot existe déjà dans les dérivés
  isExistingWord(word: string): boolean {
    return this.derivatives.some(d => d.word === word);
  }
}

import { Component, OnInit, OnDestroy } from '@angular/core';
import { CommonModule } from '@angular/common';
import { FormsModule } from '@angular/forms';
import { Subject, debounceTime, forkJoin, timeout, finalize, takeUntil, catchError, of } from 'rxjs';
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
  roots: string[] = [];
  schemes: SchemeItem[] = [];

  selectedRoot: string = '';
  selectedSchemes: SchemeItem[] = [];
  customRoot: string = '';
  customScheme: string = '';

  generatedResults: GeneratedResult[] = [];
  derivatives: DerivativeItem[] = [];

  loading = false;
  loadingData = true;
  error: string | null = null;
  loadError: string | null = null;

  private destroy$ = new Subject<void>();
  private inputSubject = new Subject<string>();

  constructor(private apiService: ApiService) {
    this.inputSubject.pipe(
      debounceTime(500),
      takeUntil(this.destroy$)
    ).subscribe(() => {
      if (this.canGenerate) {
        this.generate();
      }
    });
  }

  ngOnInit() {
    this.loadData();
    
    // Timeout de sécurité plus long (10s)
    setTimeout(() => {
      if (this.loadingData) {
        this.loadingData = false;
        this.loadError = 'انتهت مهلة الاتصال بالخادم. تأكد من تشغيل: node src/server.js على المنفذ 3001';
      }
    }, 10000);
  }

  ngOnDestroy() {
    this.destroy$.next();
    this.destroy$.complete();
  }

  onInputChange() {
    this.error = null;
    this.inputSubject.next(Date.now().toString());
  }

  onRootChange() {
    this.loadDerivatives();
    this.onInputChange();
  }

  loadData() {
    this.loadingData = true;
    this.loadError = null;
    this.error = null;

    console.log('Chargement des données depuis l\'API...');

    forkJoin({
      roots: this.apiService.getRoots().pipe(
        timeout(8000),
        catchError(err => {
          console.error('Erreur roots:', err);
          return of({ ok: false, roots: [] });
        })
      ),
      schemes: this.apiService.getSchemes().pipe(
        timeout(8000),
        catchError(err => {
          console.error('Erreur schemes:', err);
          return of({ ok: false, schemes: [] });
        })
      )
    }).pipe(
      finalize(() => {
        this.loadingData = false;
      })
    ).subscribe({
      next: (results) => {
        console.log('Réponse roots:', results.roots);
        console.log('Réponse schemes:', results.schemes);

        // Traitement des racines
        if (results.roots.ok && Array.isArray(results.roots.roots)) {
          this.roots = results.roots.roots
            .map((item: any) => {
              if (typeof item === 'string') return item.trim();
              if (item && typeof item === 'object') return (item.root || '').trim();
              return '';
            })
            .filter((r: string) => r.length > 0)
            .sort((a: string, b: string) => a.localeCompare(b, 'ar'));
        } else {
          this.loadError = 'فشل تحميل الجذور من الخادم';
        }

        // Traitement des schèmes
        if (results.schemes.ok && Array.isArray(results.schemes.schemes)) {
          this.schemes = results.schemes.schemes.map((item: any) => ({
            name: item.name || 'وزن غير معروف',
            template: item.template || item.pattern || '؟؟؟',
            pattern: item.pattern || item.template || '؟؟؟'
          }));
        } else {
          this.loadError = 'فشل تحميل الأوزان من الخادم';
        }

        // Si les deux sont vides, c'est probablement un problème de connexion
        if (this.roots.length === 0 && this.schemes.length === 0) {
          this.loadError = 'لا يمكن الاتصال بالخادم. تأكد من تشغيل: node src/server.js';
        }
      },
      error: (err) => {
        console.error('Erreur forkJoin:', err);
        this.loadingData = false;
        this.loadError = 'خطأ في الاتصال: ' + (err.message || 'الخادم غير متاح');
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
      takeUntil(this.destroy$),
      catchError(err => {
        console.error('Erreur derivatives:', err);
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

  get canGenerate(): boolean {
    const hasRoot = (this.selectedRoot || this.customRoot)?.trim().length > 0;
    const hasScheme = this.selectedSchemes.length > 0 || this.customScheme?.trim().length > 0;
    return hasRoot && hasScheme && !this.loading;
  }

  generate() {
    const root = (this.selectedRoot || this.customRoot).trim();

    let schemesToProcess: { template: string; name: string }[] = [];
    this.selectedSchemes.forEach(s => {
      schemesToProcess.push({ template: s.template, name: s.name });
    });

    if (this.customScheme?.trim()) {
      schemesToProcess.push({ 
        template: this.customScheme.trim(), 
        name: 'مخصص' 
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

    this.loading = true;
    this.error = null;
    this.generatedResults = [];

    const requests = schemesToProcess.map(s =>
      this.apiService.generate({ root, scheme: s.template }).pipe(
        timeout(15000), // Timeout augmenté pour l'engine C++
        catchError(err => {
          console.error(`Erreur génération pour ${s.name}:`, err);
          return of({ ok: false, error: err.message });
        })
      )
    );

    forkJoin(requests).pipe(
      finalize(() => {
        this.loading = false;
      })
    ).subscribe({
      next: (responses) => {
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
          this.error = 'فشل التوليد لجميع الأوزان المختارة';
        } else if (results.length < schemesToProcess.length) {
          this.error = `تم توليد ${results.length} من ${schemesToProcess.length} كلمات فقط`;
        }

        this.loadDerivatives();
      },
      error: (err) => {
        this.loading = false;
        this.error = 'خطأ في الاتصال بالخادم: ' + err.message;
        console.error('Generation error:', err);
      }
    });
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
    navigator.clipboard.writeText(word);
  }
}

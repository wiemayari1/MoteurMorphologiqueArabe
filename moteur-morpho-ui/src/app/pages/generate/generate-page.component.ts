import { Component, OnInit, OnDestroy } from '@angular/core';
import { CommonModule } from '@angular/common';
import { FormsModule } from '@angular/forms';
import { Subject, debounceTime, forkJoin, timeout, finalize, takeUntil } from 'rxjs';
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

// Interface pour présenter clairement les résultats (racine, schème, mot)
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

  // Résultats enrichis avec racine, schème et mot
  generatedResults: GeneratedResult[] = [];
  
  // Famille morphologique complète (tous les dérivés de la même racine)
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
            if (item && typeof item === 'object') {
              return (item.root || '').trim();
            }
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
            name: obj.name ?? 'وزن غير معروف',
            template: obj.template ?? obj.pattern ?? '؟؟؟',
            pattern: obj.pattern ?? obj.template ?? '؟؟؟'
          };
        });
      },
      error: (err) => {
        this.loadError = 'فشل تحميل البيانات. يرجى التأكد من تشغيل الخادم.';
        console.error('Error loading data:', err);
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
        } else {
          this.derivatives = [];
        }
      },
      error: (err) => {
        console.warn('Error loading derivatives:', err);
        this.derivatives = [];
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

    // Préparer les schèmes à traiter
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
        timeout(10000)
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
        }

        // Recharger la famille morphologique
        this.loadDerivatives();
      },
      error: (err) => {
        this.loading = false;
        this.error = 'خطأ في الاتصال بالخادم';
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

  // Copier un mot spécifique
  copyWord(word: string) {
    navigator.clipboard.writeText(word);
  }
}

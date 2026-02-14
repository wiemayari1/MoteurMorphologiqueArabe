import { Component, OnInit } from '@angular/core';
import { CommonModule } from '@angular/common';
import { FormsModule } from '@angular/forms';
import { Subject, debounceTime, distinctUntilChanged, forkJoin } from 'rxjs';
import { MatCardModule } from '@angular/material/card';
import { MatButtonModule } from '@angular/material/button';
import { MatInputModule } from '@angular/material/input';
import { MatSelectModule } from '@angular/material/select';
import { MatProgressSpinnerModule } from '@angular/material/progress-spinner';
import { MatIconModule } from '@angular/material/icon';

import { PageShellComponent } from '../../shared/page-shell/page-shell.component';
import { ApiService, RootsResponse, SchemesResponse, RootItem, SchemeItem, DerivativeItem } from '../../services/api.service';
import { MatSnackBar, MatSnackBarModule } from '@angular/material/snack-bar';

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

  generatedWords: string[] = [];
  derivatives: DerivativeItem[] = []; // History of valid derivatives

  loading = false;
  error: string | null = null;

  private searchSubject = new Subject<void>();

  constructor(private apiService: ApiService, private snackBar: MatSnackBar) {
    this.searchSubject.pipe(
      debounceTime(500),
      distinctUntilChanged()
    ).subscribe(() => {
      this.checkAndGenerate();
    });
  }

  ngOnInit() {
    this.loadData();
  }

  onInputChange() {
    this.error = null;
    this.searchSubject.next();
  }

  onRootChange() {
    this.loadDerivatives();
    this.onInputChange();
  }

  checkAndGenerate() {
    if (this.canGenerate) {
      this.generate();
    }
  }

  loadData() {
    // === Racines ===
    this.apiService.getRoots().subscribe({
      next: (response: RootsResponse | any) => {
        if (!response) return;
        let rootItems: any[] = [];
        if (Array.isArray(response)) rootItems = response;
        else if (response.roots && Array.isArray(response.roots)) rootItems = response.roots;

        if (rootItems.length > 0) {
          this.roots = rootItems
            .map(item => {
              if (typeof item === 'string') return item.trim();
              if (item && typeof item === 'object') return ((item as any).root || '').trim();
              return '';
            })
            .filter((r): r is string => r.length > 0);
        }
      },
      error: (err) => console.warn('Error loading roots', err)
    });

    // === Schémas ===
    this.apiService.getSchemes().subscribe({
      next: (response: SchemesResponse | any) => {
        if (!response) return;
        let schemeItems: any[] = [];
        if (Array.isArray(response)) schemeItems = response;
        else if (response.schemes && Array.isArray(response.schemes)) schemeItems = response.schemes;

        if (schemeItems.length > 0) {
          this.schemes = schemeItems.map(item => {
            const obj = item && typeof item === 'object' ? item : {};
            return {
              name: obj.name ?? 'وزن غير معروف',
              template: obj.template ?? obj.pattern ?? '؟؟؟'
            };
          });
        }
      },
      error: (err) => console.warn('Error loading schemes', err)
    });
  }

  loadDerivatives() {
    const root = (this.selectedRoot || this.customRoot).trim();
    if (!root) {
      this.derivatives = [];
      return;
    }

    this.apiService.getDerivatives(root).subscribe({
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

    // Prepare list of schemes to process
    let schemesToProcess: { template: string, name?: string }[] = [...this.selectedSchemes];

    if (this.customScheme && this.customScheme.trim()) {
      schemesToProcess.push({ template: this.customScheme.trim(), name: 'مخصص' });
    }

    if (!root || schemesToProcess.length === 0) {
      this.error = 'الرجاء اختيار الجذر وشيم واحد على الأقل';
      return;
    }

    if (this.customRoot && !/^[\u0600-\u06FF\s]+$/.test(this.customRoot)) {
      this.error = 'الجذر يجب أن يحتوي على أحرف عربية فقط';
      return;
    }

    this.loading = true;
    this.error = null;
    this.generatedWords = [];

    // Create observables for all schemes
    const requests = schemesToProcess.map(s =>
      this.apiService.generate({ root, scheme: s.template })
    );

    forkJoin(requests).subscribe({
      next: (responses) => {
        this.loading = false;
        const newWords: string[] = [];

        responses.forEach(res => {
          if (res.ok && res.word) {
            newWords.push(res.word);
          }
        });

        if (newWords.length > 0) {
          this.generatedWords = newWords;
          // Reload history to show newly persisted derivatives
          this.loadDerivatives();
        } else {
          this.error = 'فشل التوليد لجميع الأوزان المختارة';
        }
      },
      error: (err) => {
        this.loading = false;
        this.error = 'خطأ في الاتصال بالخادم';
        console.error(err);
      }
    });
  }

  reset() {
    this.selectedRoot = '';
    this.selectedSchemes = [];
    this.customRoot = '';
    this.customScheme = '';
    this.generatedWords = [];
    this.derivatives = [];
    this.error = null;
  }

  copyWord(word: string) {
    navigator.clipboard.writeText(word).then(
      () => this.snackBar.open('تم نسخ الكلمة: ' + word, 'إغلاق', { duration: 2000 }),
      () => this.snackBar.open('فشل النسخ', 'إغلاق', { duration: 2000 })
    );
  }
}

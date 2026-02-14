import { Component, OnInit } from '@angular/core';
import { CommonModule } from '@angular/common';
import { FormsModule } from '@angular/forms';
import { MatCardModule } from '@angular/material/card';
import { MatButtonModule } from '@angular/material/button';
import { MatInputModule } from '@angular/material/input';
import { MatSelectModule } from '@angular/material/select';
import { MatProgressSpinnerModule } from '@angular/material/progress-spinner';
import { MatIconModule } from '@angular/material/icon';

import { PageShellComponent } from '../../shared/page-shell/page-shell.component';
import { ApiService, RootsResponse, SchemesResponse, RootItem, SchemeItem } from '../../services/api.service';
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
  template: `
    <app-page-shell title="توليد الكلمات">
      <div class="generate-container">
        <mat-card class="generate-card">
          <div class="generate-form">
            <div class="form-row">
              <mat-form-field appearance="outline" class="full-width">
                <mat-label>اختر الجذر</mat-label>
                <mat-select [(ngModel)]="selectedRoot">
                  <mat-option *ngFor="let root of roots" [value]="root">
                    {{ root }}
                  </mat-option>
                </mat-select>
              </mat-form-field>
            </div>

            <div class="form-row">
              <mat-form-field appearance="outline" class="full-width">
                <mat-label>اختر الوزن</mat-label>
                <mat-select [(ngModel)]="selectedScheme">
                  <mat-option *ngFor="let scheme of schemes" [value]="scheme">
                    {{ scheme.name }} ({{ scheme.template }})
                  </mat-option>
                </mat-select>
              </mat-form-field>
            </div>

            <div class="form-row custom-inputs" *ngIf="!selectedRoot || !selectedScheme">
              <mat-form-field appearance="outline">
                <mat-label>أو أدخل الجذر يدوياً</mat-label>
                <input matInput [(ngModel)]="customRoot" placeholder="مثل: كتب">
              </mat-form-field>

              <mat-form-field appearance="outline">
                <mat-label>أو أدخل الوزن يدوياً</mat-label>
                <input matInput [(ngModel)]="customScheme" placeholder="مثل: فاعل">
              </mat-form-field>
            </div>

            <button 
              mat-raised-button 
              color="primary" 
              class="generate-btn"
              [disabled]="loading || !canGenerate"
              (click)="generate()">
              <mat-icon *ngIf="!loading">auto_fix_high</mat-icon>
              <span *ngIf="!loading">توليد</span>
              <mat-spinner *ngIf="loading" diameter="24"></mat-spinner>
            </button>

            <button 
              mat-button 
              color="warn" 
              (click)="reset()"
              [disabled]="loading">
              إعادة تعيين
            </button>
          </div>
        </mat-card>

        <!-- Results -->
        <mat-card class="results-card" *ngIf="generatedWords.length > 0">
          <h3>الكلمات المولدة:</h3>
          <div class="words-list">
            <div 
              *ngFor="let word of generatedWords; let i = index" 
              class="word-item"
              [style.animation-delay]="i * 100 + 'ms'">
              <span class="word-number">{{ i + 1 }}</span>
              <span class="word-text">{{ word }}</span>
              <button mat-icon-button (click)="copyWord(word)">
                <mat-icon>content_copy</mat-icon>
              </button>
            </div>
          </div>
        </mat-card>

        <!-- Error -->
        <div class="error-message" *ngIf="error">
          <mat-icon>error</mat-icon>
          <p>{{ error }}</p>
        </div>

        <!-- Examples -->
        <mat-card class="examples-card">
          <h3>أمثلة:</h3>
          <div class="examples-list">
            <div class="example-item" *ngFor="let ex of examples" (click)="loadExample(ex)">
              <span class="example-root">{{ ex.root }}</span>
              <mat-icon>arrow_forward</mat-icon>
              <span class="example-word">{{ ex.result }}</span>
            </div>
          </div>
        </mat-card>
      </div>
    </app-page-shell>
  `,
  styleUrls: ['./generate-page.component.scss']
})
export class GeneratePageComponent implements OnInit {
  roots: string[] = ['كتب', 'درس', 'علم', 'فتح', 'عمل'];

  schemes: { id?: number; name: string; template: string }[] = [
    { name: 'فاعل', template: 'فاعل' },
    { name: 'مفعول', template: 'مفعول' },
    { name: 'كتاب', template: 'فِعال' },
    { name: 'مفعّلة', template: 'مفعّلة' }
  ];

  selectedRoot: string = '';
  selectedScheme: { id?: number; name: string; template: string } | null = null;
  customRoot: string = '';
  customScheme: string = '';
  generatedWords: string[] = [];
  loading = false;
  error: string | null = null;

  examples = [
    { root: 'كتب', scheme: 'فاعل', result: 'كاتب' },
    { root: 'كتب', scheme: 'مفعول', result: 'مكتوب' },
    { root: 'درس', scheme: 'مفعّلة', result: 'مدرسة' }
  ];

  constructor(private apiService: ApiService, private snackBar: MatSnackBar) { }

  ngOnInit() {
    this.loadData();
  }

  loadData() {
    // === Racines ===
    this.apiService.getRoots().subscribe({
      next: (response: RootsResponse | any) => {
        if (!response) return;

        let rootItems: any[] = [];

        if (Array.isArray(response)) {
          rootItems = response;
        } else if (response.roots && Array.isArray(response.roots)) {
          rootItems = response.roots;
        }

        if (rootItems.length > 0) {
          this.roots = rootItems
            .map(item => {
              if (typeof item === 'string') {
                return item.trim();
              }
              if (item && typeof item === 'object') {
                // plusieurs noms possibles selon l'API réelle
                return (
                  (item as any).root ||
                  (item as any).text ||
                  (item as any).value ||
                  (item as any).name ||
                  ''
                ).trim();
              }
              return '';
            })
            .filter((r): r is string => r.length > 0);
        }
      },
      error: (err) => {
        console.warn('Erreur chargement des racines → valeurs par défaut utilisées', err);
      }
    });

    // === Schémas / أوزان ===
    this.apiService.getSchemes().subscribe({
      next: (response: SchemesResponse | any) => {
        if (!response) return;

        let schemeItems: any[] = [];

        if (Array.isArray(response)) {
          schemeItems = response;
        } else if (response.schemes && Array.isArray(response.schemes)) {
          schemeItems = response.schemes;
        }

        if (schemeItems.length > 0) {
          this.schemes = schemeItems.map(item => {
            const obj = item && typeof item === 'object' ? item : {};

            return {
              id: obj.id ?? undefined,
              name: obj.name ?? 'وزن غير معروف',
              template: obj.template ?? obj.pattern ?? '؟؟؟'
            };
          });
        }
      },
      error: (err) => {
        console.warn('Erreur chargement des أوزان → valeurs par défaut utilisées', err);
      }
    });
  }

  get canGenerate(): boolean {
    const hasRoot = (this.selectedRoot || this.customRoot)?.trim().length > 0;
    const hasScheme = (this.selectedScheme?.template || this.customScheme)?.trim().length > 0;
    return hasRoot && hasScheme && !this.loading;
  }

  generate() {
    const root = (this.selectedRoot || this.customRoot).trim();
    const schemePattern = this.selectedScheme?.template || this.customScheme.trim();

    if (!root || !schemePattern) {
      this.error = 'الرجاء اختيار الجذر والوزن أو إدخالهما يدوياً';
      return;
    }

    if (this.customRoot && !/^[\u0600-\u06FF\s]+$/.test(this.customRoot)) {
      this.error = 'الجذر يجب أن يحتوي على أحرف عربية فقط';
      return;
    }

    if (this.customScheme && !/^[\u0600-\u06FF\s0-9]+$/.test(this.customScheme)) {
      this.error = 'الوزن يجب أن يحتوي على أحرف عربية فقط';
      return;
    }

    this.loading = true;
    this.error = null;
    this.generatedWords = [];

    const safetyTimeout = setTimeout(() => {
      if (this.loading) {
        this.loading = false;
        this.error = 'تجاوزت مهلة الانتظار. الخادم لا يستجيب.';
      }
    }, 15000);

    this.apiService.generate({ root, scheme: schemePattern }).subscribe({
      next: (response) => {
        clearTimeout(safetyTimeout);
        this.loading = false;
        if (response.ok && response.word) {
          this.generatedWords = [response.word];
        } else {
          this.error = response.error || 'فشل التوليد';
          if (this.error === 'scheme_not_found') {
            this.error = 'الوزن المختار غير مدعوم حالياً في المحرّك.';
          }
        }
      },
      error: (err) => {
        clearTimeout(safetyTimeout);
        this.loading = false;
        this.error = err.message || 'خطأ في الاتصال بالخادم';
        this.snackBar.open(this.error || 'خطأ في الاتصال بالخادم', 'إغلاق', { duration: 5000 });
        console.error('Generation error:', err);
      }
    });
  }

  reset() {
    this.selectedRoot = '';
    this.selectedScheme = null;
    this.customRoot = '';
    this.customScheme = '';
    this.generatedWords = [];
    this.error = null;
  }

  loadExample(ex: { root: string; scheme: string; result: string }) {
    this.selectedRoot = ex.root;
    this.selectedScheme = this.schemes.find(s => s.template === ex.scheme) || null;
    this.customRoot = ex.root;
    this.customScheme = ex.scheme;
    this.generate();
  }

  copyWord(word: string) {
    navigator.clipboard.writeText(word).then(
      () => alert('تم نسخ الكلمة: ' + word),
      () => alert('فشل النسخ')
    );
  }
}

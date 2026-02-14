import { Component } from '@angular/core';
import { CommonModule } from '@angular/common';
import { FormsModule } from '@angular/forms';
import { ApiService, RootItem } from '../../services/api.service';
import { Subject, debounceTime, distinctUntilChanged } from 'rxjs';

import { MatCardModule } from '@angular/material/card';
import { MatButtonModule } from '@angular/material/button';
import { MatFormFieldModule } from '@angular/material/form-field';
import { MatInputModule } from '@angular/material/input';
import { MatIconModule } from '@angular/material/icon';
import { MatSnackBar, MatSnackBarModule } from '@angular/material/snack-bar';

import { PageShellComponent } from '../../shared/page-shell/page-shell.component';

@Component({
  standalone: true,
  selector: 'app-roots-page',
  imports: [
    CommonModule,
    FormsModule,
    MatCardModule,
    MatButtonModule,
    MatFormFieldModule,
    MatInputModule,
    MatIconModule,
    MatSnackBarModule,
    PageShellComponent,
  ],
  templateUrl: './roots-page.component.html',
  styleUrls: ['./roots-page.component.scss'],
})
export class RootsPageComponent {
  roots: RootItem[] = [];
  filtered: RootItem[] = [];

  newRoot = '';
  query = '';

  loading = false;
  error: string | null = null;
  info: string | null = null;
  success: string | null = null;
  private searchSubject = new Subject<string>();

  constructor(private api: ApiService, private snackBar: MatSnackBar) {
    this.refresh();
    this.searchSubject.pipe(
      debounceTime(300),
      distinctUntilChanged()
    ).subscribe(() => this.applyFilter());
  }

  refresh() {
    this.loading = true;
    this.error = null;
    this.info = null;
    this.success = null;
    this.query = ''; // Reset query to show full count correctly

    this.api.listRoots().subscribe({
      next: (r) => {
        // Gérer les deux formats: string[] ou RootItem[]
        const rawRoots = r?.roots ?? [];
        this.roots = rawRoots.map((item: any) =>
          typeof item === 'string' ? { root: item, meaning: '' } : item
        );
        this.applyFilter();
        if (this.roots.length === 0) this.info = 'لا توجد جذور بعد.';
      },
      error: (e) => {
        this.error = e?.message ?? 'خطأ في الاتصال بالخادم';
        this.loading = false;
      },
      complete: () => (this.loading = false),
    });
  }

  add() {
    const root = (this.newRoot ?? '').trim();
    this.error = null;
    this.info = null;
    this.success = null;

    if (!/^[\u0600-\u06FF]{3}$/.test(root)) {
      this.error = 'الرجاء إدخال جذر ثلاثي عربي صحيح (3 أحرف عربية فقط).';
      return;
    }

    // Vérifier doublon côté client (rapide)
    if (this.roots.some(r => r.root === root)) {
      this.error = 'هذا الجذر موجود مسبقاً!';
      return;
    }

    this.loading = true;
    this.api.addRoot(root).subscribe({
      next: (res) => {
        this.newRoot = '';
        this.snackBar.open('تمت الإضافة بنجاح!', 'إغلاق', { duration: 3000 });
        // Optimistic update to avoid race condition/blank flashes
        this.roots.push({ root, meaning: '' });
        this.applyFilter();
        // Still refresh to stay in sync with server
        setTimeout(() => this.refresh(), 500);
      },
      error: (e) => {
        this.loading = false;
        this.snackBar.open(e?.message ?? 'خطأ في الإضافة', 'إغلاق', {
          duration: 4000,
          panelClass: ['error-snackbar']
        });
      },
    });
  }

  // ✅ NOUVEAU: Supprimer une racine
  delete(rootToDelete: string) {
    if (!confirm(`هل تريد حذف الجذر "${rootToDelete}"؟`)) return;

    this.loading = true;
    this.error = null;
    this.success = null;

    this.api.deleteRoot(rootToDelete).subscribe({
      next: () => {
        this.success = 'تم الحذف بنجاح!';
        this.refresh();
      },
      error: (e) => {
        this.loading = false;
        this.error = e?.message ?? 'خطأ في الحذف';
      },
    });
  }

  onSearchChange() {
    this.error = null; // Clear error on search/input change per feedback
    this.searchSubject.next(this.query);
  }

  applyFilter() {
    const q = (this.query ?? '').trim();
    if (!q) {
      this.filtered = [...this.roots];
      return;
    }
    // ✅ Recherche sur root et meaning
    this.filtered = this.roots.filter((r) =>
      r.root.includes(q) || (r.meaning && r.meaning.includes(q))
    );
  }

  private isTriliteralArabic(s: string) {
    // Broadened Arabic range to include diacritics and letters
    return /^[\u0600-\u06FF]{3}$/.test(s);
  }
}

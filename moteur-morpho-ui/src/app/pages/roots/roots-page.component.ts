import { Component } from '@angular/core';
import { CommonModule } from '@angular/common';
import { FormsModule } from '@angular/forms';
import { ApiService, RootItem } from '../../services/api.service';

import { MatCardModule } from '@angular/material/card';
import { MatButtonModule } from '@angular/material/button';
import { MatFormFieldModule } from '@angular/material/form-field';
import { MatInputModule } from '@angular/material/input';
import { MatIconModule } from '@angular/material/icon';

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

  constructor(private api: ApiService) {
    this.refresh();
  }

  refresh() {
    this.loading = true;
    this.error = null;
    this.info = null;
    this.success = null;

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

    if (!this.isTriliteralArabic(root)) {
      this.error = 'الرجاء إدخال جذر ثلاثي عربي صحيح (مثال: كتب).';
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
        this.newRoot = '';  // ✅ Vider le champ
        this.success = 'تمت الإضافة بنجاح!';
        this.refresh();      // ✅ Recharger la liste
      },
      error: (e) => {
        this.loading = false;
        // Gérer l'erreur de doublon du serveur
        if (e?.error?.error === 'duplicate') {
          this.error = 'هذا الجذر موجود مسبقاً!';
        } else {
          this.error = e?.message ?? 'خطأ في الإضافة';
        }
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
    // Arabic letters range basic: \u0621-\u064A
    // Must be exactly 3 letters, no spaces
    return /^[\u0621-\u064A]{3}$/.test(s);
  }
}

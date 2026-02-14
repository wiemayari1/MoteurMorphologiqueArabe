import { Component } from '@angular/core';
import { FormsModule } from '@angular/forms';
import { ApiService, SchemeItem } from '../../services/api.service';
import { PageShellComponent } from '../../shared/page-shell/page-shell.component';
import { MatCardModule } from '@angular/material/card';
import { MatFormFieldModule } from '@angular/material/form-field';
import { MatInputModule } from '@angular/material/input';
import { MatButtonModule } from '@angular/material/button';
import { MatIconModule } from '@angular/material/icon';
import { CommonModule } from '@angular/common';

@Component({
  standalone: true,
  selector: 'app-schemes-page',
  imports: [
    CommonModule,
    FormsModule,
    PageShellComponent,
    MatCardModule,
    MatFormFieldModule,
    MatInputModule,
    MatButtonModule,
    MatIconModule,
  ],
  templateUrl: './schemes-page.component.html',
  styleUrls: ['./schemes-page.component.scss'],
})
export class SchemesPageComponent {
  // Formulaire
  name = '';
  templ = '';

  // Mode édition
  editingName: string | null = null;
  isEditing = false;

  // Données
  schemes: SchemeItem[] = [];

  // États
  loading = false;
  error = '';
  success = '';

  constructor(private api: ApiService) {
    this.load();
  }

  // ✅ CHARGER la liste
  load() {
    this.loading = true;
    this.error = '';
    this.success = '';

    // Safety timeout
    const timeout = setTimeout(() => {
      if (this.loading) {
        this.loading = false;
        this.error = 'تجاوزت مهلة الانتظار';
      }
    }, 10000);

    this.api.listSchemes().subscribe({
      next: (r) => {
        clearTimeout(timeout);
        this.schemes = r.schemes ?? [];
        this.loading = false;
      },
      error: (e: any) => {
        clearTimeout(timeout);
        this.error = e?.message ?? 'خطأ في تحميل الأوزان';
        this.loading = false;
      },
    });
  }

  // ✅ AJOUTER un nouveau schéma
  addNew() {
    this.error = '';
    this.success = '';

    const cleanName = this.name.trim();
    const cleanTempl = this.templ.trim();

    if (!cleanName || !cleanTempl) {
      this.error = 'الرجاء إدخال الاسم والقالب';
      return;
    }

    // Vérifier doublon
    if (this.schemes.some(s => s.name === cleanName)) {
      this.error = 'هذا الوزن موجود مسبقاً! استخدم تحديث للتعديل.';
      return;
    }

    this.loading = true;
    this.api.addScheme(cleanName, cleanTempl).subscribe({
      next: () => {
        this.success = 'تمت الإضافة بنجاح!';
        this.name = '';
        this.templ = '';
        this.load();
      },
      error: (e: any) => {
        this.loading = false;
        this.error = e?.message ?? 'خطأ في الإضافة';
      },
    });
  }

  // ✅ METTRE À JOUR un schéma existant
  update() {
    if (!this.editingName) return;

    this.error = '';
    this.success = '';

    const cleanTempl = this.templ.trim();

    if (!cleanTempl) {
      this.error = 'الرجاء إدخال القالب';
      return;
    }

    this.loading = true;
    this.api.updateScheme(this.editingName, cleanTempl).subscribe({
      next: () => {
        this.success = 'تم التحديث بنجاح!';
        this.cancelEdit();
        this.load();
      },
      error: (e: any) => {
        this.loading = false;
        this.error = e?.message ?? 'خطأ في التحديث';
      },
    });
  }

  // ✅ PRÉPARER l'édition
  editScheme(s: SchemeItem) {
    this.editingName = s.name;
    this.name = s.name;  // Nom en lecture seule
    this.templ = s.template;
    this.isEditing = true;
    this.error = '';
    this.success = '';
  }

  // ✅ ANNULER l'édition
  cancelEdit() {
    this.editingName = null;
    this.isEditing = false;
    this.name = '';
    this.templ = '';
    this.error = '';
    this.success = '';
  }

  // ✅ SUPPRIMER un schéma
  deleteScheme(s: SchemeItem) {
    if (!confirm(`هل تريد حذف الوزن "${s.name}"؟`)) return;

    this.loading = true;
    this.error = '';

    this.api.deleteScheme(s.name).subscribe({
      next: () => {
        // Si on supprime celui en cours d'édition, annuler l'édition
        if (this.editingName === s.name) {
          this.cancelEdit();
        }
        this.success = 'تم الحذف بنجاح!';
        this.load();
      },
      error: (e: any) => {
        this.loading = false;
        this.error = e?.message ?? 'خطأ في الحذف';
      },
    });
  }

  // ✅ SAUVEGARDER (ajout ou mise à jour selon le mode)
  save() {
    if (this.isEditing) {
      this.update();
    } else {
      this.addNew();
    }
  }
}

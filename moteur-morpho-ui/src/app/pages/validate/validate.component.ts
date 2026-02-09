import { Component } from '@angular/core';
import { CommonModule } from '@angular/common';
import { FormsModule } from '@angular/forms';

import { MatCardModule } from '@angular/material/card';
import { MatFormFieldModule } from '@angular/material/form-field';
import { MatInputModule } from '@angular/material/input';
import { MatButtonModule } from '@angular/material/button';
import { MatIconModule } from '@angular/material/icon';
import { MatProgressBarModule } from '@angular/material/progress-bar';

import { ApiService } from '../../services/api.service';

@Component({
  selector: 'app-validate',
  standalone: true,
  imports: [
    CommonModule,
    FormsModule,
    MatCardModule,
    MatFormFieldModule,
    MatInputModule,
    MatButtonModule,
    MatIconModule,
    MatProgressBarModule,
  ],
  templateUrl: './validate.component.html',
  styleUrls: ['./validate.component.scss'],
})
export class ValidateComponent {
  word = 'مكتوب';
  root = 'كتب';
  loading = false;
  belongs: boolean | null = null;
  schemes: string[] = [];
  error = '';

  constructor(private api: ApiService) {}

  submit() {
    this.loading = true;
    this.belongs = null;
    this.schemes = [];
    this.error = '';

    this.api.validate(this.word.trim(), this.root.trim()).subscribe({
      next: (res) => {
        this.loading = false;
        if (!res?.ok) {
          this.error = 'حدث خطأ أثناء التحقق';
          return;
        }
        this.belongs = !!res.belongs;
        this.schemes = res.schemes ?? [];
      },
      error: (e) => {
        this.loading = false;
        this.error = e?.error?.message ?? 'تعذر الاتصال بالـ API';
      },
    });
  }
}

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
  selector: 'app-generate',
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
  templateUrl: './generate.component.html',
  styleUrls: ['./generate.component.scss'],
})
export class GenerateComponent {
  root = 'كتب';
  scheme = 'مفعول';
  loading = false;
  resultWord = '';
  error = '';

  constructor(private api: ApiService) {}

  submit() {
    this.loading = true;
    this.resultWord = '';
    this.error = '';

    this.api.generate(this.root.trim(), this.scheme.trim()).subscribe({
      next: (res) => {
        this.loading = false;
        if (!res?.ok) {
          this.error = 'حدث خطأ أثناء التوليد';
          return;
        }
        this.resultWord = res.word ?? '';
      },
      error: (e) => {
        this.loading = false;
        this.error = e?.error?.message ?? 'تعذر الاتصال بالـ API';
      },
    });
  }
}

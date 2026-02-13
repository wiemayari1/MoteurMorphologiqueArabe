import { Component } from '@angular/core';
import { FormsModule } from '@angular/forms';
import { ApiService, ValidateResponse } from '../../services/api.service';
import { MatCardModule } from '@angular/material/card';
import { MatFormFieldModule } from '@angular/material/form-field';
import { MatInputModule } from '@angular/material/input';
import { MatButtonModule } from '@angular/material/button';
import { MatSnackBar, MatSnackBarModule } from '@angular/material/snack-bar';
import { PageShellComponent } from '../../shared/page-shell/page-shell.component';

@Component({
  standalone: true,
  selector: 'app-validate-page',
  imports: [
    FormsModule,
    MatCardModule,
    MatFormFieldModule,
    MatInputModule,
    MatButtonModule,
    MatSnackBarModule,
    PageShellComponent,
  ],
  templateUrl: './validate-page.component.html',
  styleUrls: ['./validate-page.component.scss'],
})
export class ValidatePageComponent {
  word = '';
  root = '';
  res: ValidateResponse | null = null;
  loading = false;

  constructor(private api: ApiService, private snackBar: MatSnackBar) { }

  submit() {
    this.res = null;
    this.loading = true;
    this.api.validate({ word: this.word, root: this.root }).subscribe({
      next: (r) => {
        this.res = r;
        if (r.ok) {
          const msg = r.belongs ? 'الكلمة تنتمي إلى الجذر!' : 'الكلمة لا تنتمي إلى هذا الجذر.';
          this.snackBar.open(msg, 'إغلاق', { duration: 3000 });
        }
      },
      error: (e) => {
        this.res = { ok: false, error: e?.message ?? 'network_error' };
        this.snackBar.open('خطأ في الاتصال بالخادم', 'إغلاق', { duration: 4000 });
      },
      complete: () => (this.loading = false),
    });
  }
}

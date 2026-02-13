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
  errorMessage: string | null = null;
  loading = false;

  get canSubmit(): boolean {
    return this.word.trim().length > 0 && this.root.trim().length > 0 && !this.loading;
  }

  constructor(private api: ApiService, private snackBar: MatSnackBar) { }

  submit() {
    this.res = null;
    this.loading = true;
    this.api.validate({ word: this.word, root: this.root })
      .subscribe({
        next: (r) => {
          this.res = r;
          this.errorMessage = null;
          this.loading = false;
        },
        error: (e) => {
          this.res = null;
          this.errorMessage = e?.message ?? 'خطأ في الاتصال بالخادم';
          this.snackBar.open(this.errorMessage!, 'إغلاق', { duration: 4000 });
          this.loading = false;
        }
      });
  }

  onInputChange() {
    this.errorMessage = null;
    this.res = null;
  }
}

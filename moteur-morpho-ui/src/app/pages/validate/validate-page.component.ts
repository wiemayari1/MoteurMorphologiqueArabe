import { Component } from '@angular/core';
import { FormsModule } from '@angular/forms';
import { ApiService, ValidateResponse } from '../../services/api.service';
import { MatCardModule } from '@angular/material/card';
import { MatFormFieldModule } from '@angular/material/form-field';
import { MatInputModule } from '@angular/material/input';
import { MatButtonModule } from '@angular/material/button';

@Component({
  standalone: true,
  selector: 'app-validate-page',
  imports: [FormsModule, MatCardModule, MatFormFieldModule, MatInputModule, MatButtonModule],
  templateUrl: './validate-page.component.html',
  styleUrls: ['./validate-page.component.scss']
})
export class ValidatePageComponent {
  word = '';
  root = '';
  res: ValidateResponse | null = null;
  loading = false;

  constructor(private api: ApiService) {}

  submit() {
    this.res = null;
    this.loading = true;
    this.api.validate({ word: this.word, root: this.root }).subscribe({
      next: (r) => (this.res = r),
      error: (e) => (this.res = { ok: false, error: e?.message ?? 'network_error' }),
      complete: () => (this.loading = false)
    });
  }
}

import { Component } from '@angular/core';
import { FormsModule } from '@angular/forms';
import { ApiService, GenerateResponse } from '../../services/api.service';
import { MatCardModule } from '@angular/material/card';
import { MatFormFieldModule } from '@angular/material/form-field';
import { MatInputModule } from '@angular/material/input';
import { MatButtonModule } from '@angular/material/button';

@Component({
  standalone: true,
  selector: 'app-generate-page',
  imports: [FormsModule, MatCardModule, MatFormFieldModule, MatInputModule, MatButtonModule],
  templateUrl: './generate-page.component.html',
  styleUrls: ['./generate-page.component.scss']
})
export class GeneratePageComponent {
  root = '';
  scheme = '';
  res: GenerateResponse | null = null;
  loading = false;

  constructor(private api: ApiService) {}

  submit() {
    this.res = null;
    this.loading = true;
    this.api.generate({ root: this.root, scheme: this.scheme }).subscribe({
      next: (r) => (this.res = r),
      error: (e) => (this.res = { ok: false, error: e?.message ?? 'network_error' }),
      complete: () => (this.loading = false)
    });
  }
}

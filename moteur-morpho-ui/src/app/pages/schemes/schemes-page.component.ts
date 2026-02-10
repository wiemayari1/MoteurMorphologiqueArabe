import { Component } from '@angular/core';
import { FormsModule } from '@angular/forms';
import { ApiService } from '../../services/api.service';
import { MatCardModule } from '@angular/material/card';
import { MatButtonModule } from '@angular/material/button';
import { MatFormFieldModule } from '@angular/material/form-field';
import { MatInputModule } from '@angular/material/input';

type Scheme = { name: string; templ: string };

@Component({
  standalone: true,
  selector: 'app-schemes-page',
  imports: [FormsModule, MatCardModule, MatButtonModule, MatFormFieldModule, MatInputModule],
  templateUrl: './schemes-page.component.html',
  styleUrls: ['./schemes-page.component.scss']
})
export class SchemesPageComponent {
  schemes: Scheme[] = [];
  name = '';
  templ = '';
  error: string | null = null;

  constructor(private api: ApiService) {
    this.refresh();
  }

  refresh() {
    this.error = null;
    this.api.listSchemes().subscribe({
      next: (r) => (this.schemes = r.schemes ?? []),
      error: (e) => (this.error = e?.message ?? 'network_error')
    });
  }

  save() {
    if (!this.name.trim() || !this.templ.trim()) return;
    this.api.upsertScheme(this.name.trim(), this.templ.trim()).subscribe({
      next: () => {
        this.name = '';
        this.templ = '';
        this.refresh();
      },
      error: (e) => (this.error = e?.message ?? 'network_error')
    });
  }

  del(s: Scheme) {
    this.api.deleteScheme(s.name).subscribe({
      next: () => this.refresh(),
      error: (e) => (this.error = e?.message ?? 'network_error')
    });
  }
}

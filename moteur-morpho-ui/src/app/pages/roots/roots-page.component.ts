import { Component } from '@angular/core';
import { FormsModule } from '@angular/forms';
import { ApiService } from '../../services/api.service';
import { MatCardModule } from '@angular/material/card';
import { MatButtonModule } from '@angular/material/button';
import { MatFormFieldModule } from '@angular/material/form-field';
import { MatInputModule } from '@angular/material/input';

@Component({
  standalone: true,
  selector: 'app-roots-page',
  imports: [FormsModule, MatCardModule, MatButtonModule, MatFormFieldModule, MatInputModule],
  templateUrl: './roots-page.component.html',
  styleUrls: ['./roots-page.component.scss']
})
export class RootsPageComponent {
  roots: string[] = [];
  newRoot = '';
  loading = false;
  error: string | null = null;

  constructor(private api: ApiService) {
    this.refresh();
  }

  refresh() {
    this.loading = true;
    this.error = null;
    this.api.listRoots().subscribe({
      next: (r) => (this.roots = r.roots ?? []),
      error: (e) => (this.error = e?.message ?? 'network_error'),
      complete: () => (this.loading = false)
    });
  }

  add() {
    if (!this.newRoot.trim()) return;
    this.api.addRoot(this.newRoot.trim()).subscribe({
      next: () => {
        this.newRoot = '';
        this.refresh();
      },
      error: (e) => (this.error = e?.message ?? 'network_error')
    });
  }
}

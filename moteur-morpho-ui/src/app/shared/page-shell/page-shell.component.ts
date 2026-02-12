import { Component, Input } from '@angular/core';
import { CommonModule } from '@angular/common';
import { RouterModule, Router } from '@angular/router';
import { MatIconModule } from '@angular/material/icon';
import { MatButtonModule } from '@angular/material/button';

@Component({
  selector: 'app-page-shell',
  standalone: true,
  imports: [CommonModule, RouterModule, MatIconModule, MatButtonModule],
  template: `
    <div class="page-container">
      <!-- Header avec titre et bouton home -->
      <header class="page-header">
        <div class="header-content">
          <button mat-icon-button class="home-btn" (click)="goHome()" aria-label="Retour à l'accueil">
            <mat-icon>home</mat-icon>
          </button>
          <h1 class="page-title">{{ title }}</h1>
          <div class="header-spacer"></div>
        </div>
      </header>
      
      <!-- Contenu de la page -->
      <main class="page-content">
        <ng-content></ng-content>
      </main>

      <!-- Illustration Book au bas de la page -->
      <footer class="page-footer">
        <div class="footer-art">
          <img src="assets/illustrations/book.png" alt="Book Illustration" class="book-img">
        </div>
      </footer>
    </div>
  `,
  styleUrls: ['./page-shell.component.scss']
})
export class PageShellComponent {
  @Input() title: string = '';

  constructor(private router: Router) { }

  goHome() {
    this.router.navigate(['/']);
  }
}

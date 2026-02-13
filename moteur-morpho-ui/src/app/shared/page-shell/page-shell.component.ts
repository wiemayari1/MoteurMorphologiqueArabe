import { Component, Input } from '@angular/core';
import { CommonModule } from '@angular/common';
import { RouterModule, Router } from '@angular/router';
import { MatIconModule } from '@angular/material/icon';
import { MatButtonModule } from '@angular/material/button';

@Component({
  selector: 'app-page-shell',
  standalone: true,
  imports: [CommonModule, RouterModule, MatIconModule, MatButtonModule],
  templateUrl: './page-shell.component.html',
  styleUrls: ['./page-shell.component.scss']
})
export class PageShellComponent {
  @Input() title: string = '';
  @Input() cardTitle: string = '';
  @Input() showFooterImage: boolean = false;
  @Input() footerImageSrc: string = 'assets/illustrations/book.png';
  @Input() footerImageAlt: string = 'Illustration';

  constructor(public router: Router) { }

  goHome() {
    this.router.navigate(['/']);
  }

  getPageTitle(): string {
    if (this.title) return this.title;

    const url = this.router.url;
    if (url.includes('roots')) return 'إدارة الجذور';
    if (url.includes('schemes')) return 'إدارة الأوزان';
    if (url.includes('validate')) return 'التحقق من الكلمات';
    if (url.includes('generate')) return 'توليد الكلمات';
    if (url.includes('game')) return 'اللعبة';

    return 'المحرّك الصرفي';
  }
}

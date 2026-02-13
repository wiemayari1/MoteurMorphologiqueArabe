import { Component } from '@angular/core';
import { CommonModule } from '@angular/common';
import { PageShellComponent } from '../../shared/page-shell/page-shell.component';
import { MatCardModule } from '@angular/material/card';
import { MatIconModule } from '@angular/material/icon';
import { MatButtonModule } from '@angular/material/button';

@Component({
    selector: 'app-settings-page',
    standalone: true,
    imports: [CommonModule, PageShellComponent, MatCardModule, MatIconModule, MatButtonModule],
    template: `
    <app-page-shell title="الإعدادات" [cardTitle]="'تعديل المحرّك'">
      <div class="settings-container">
        <mat-card class="settings-card">
          <mat-card-header>
            <mat-icon mat-card-avatar>settings</mat-icon>
            <mat-card-title>تعديل الإعدادات</mat-card-title>
          </mat-card-header>
          
          <mat-card-content>
            <div class="placeholder-content">
              <mat-icon class="huge-icon">construction</mat-icon>
              <p>هذه الصفحة قيد التطوير. ستتمكن قريباً من تعديل إعدادات المحرّك الصرفي من هنا.</p>
            </div>
          </mat-card-content>
          
          <mat-card-actions align="end">
            <button mat-button color="primary" routerLink="/">العودة للرئيسية</button>
          </mat-card-actions>
        </mat-card>
      </div>
    </app-page-shell>
  `,
    styles: [`
    .settings-container {
      padding: var(--spacing-lg);
      display: flex;
      justify-content: center;
    }
    .settings-card {
      width: 100%;
      max-width: 600px;
      border-radius: var(--radius-xl);
      box-shadow: var(--shadow-lg);
    }
    .placeholder-content {
      display: flex;
      flex-direction: column;
      align-items: center;
      padding: var(--spacing-2xl);
      color: var(--color-text-secondary);
      text-align: center;
      
      .huge-icon {
        font-size: 64px;
        width: 64px;
        height: 64px;
        margin-bottom: var(--spacing-lg);
        opacity: 0.3;
      }
    }
  `]
})
export class SettingsPageComponent { }

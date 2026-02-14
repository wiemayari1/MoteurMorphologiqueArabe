import { Component, OnInit, OnDestroy } from '@angular/core';
import { CommonModule } from '@angular/common';
import { RouterModule } from '@angular/router';
import { MatCardModule } from '@angular/material/card';
import { MatButtonModule } from '@angular/material/button';
import { MatProgressBarModule } from '@angular/material/progress-bar';
import { MatIconModule } from '@angular/material/icon';
import { PageShellComponent } from '../../shared/page-shell/page-shell.component';
import { ApiService, GameQuestion } from '../../services/api.service';
import { Subject, takeUntil, timeout } from 'rxjs';

@Component({
  selector: 'app-game-page',
  standalone: true,
  imports: [
    CommonModule,
    RouterModule,
    MatCardModule,
    MatButtonModule,
    MatProgressBarModule,
    MatIconModule,
    PageShellComponent
  ],
  templateUrl: './game-page.component.html',
  styleUrls: ['./game-page.component.scss']
})
export class GamePageComponent implements OnInit, OnDestroy {
  loading = true;
  error: string | null = null;

  // Game State
  score = 0;
  totalQuestions = 6;
  currentIndex = 0;
  gameFinished = false;

  // Current Question Data based on HTML template requirements
  question: any = null; // { root, scheme, options, correct_index }

  // User Interaction
  showResult = false;
  isCorrect = false;
  selectedIndex: number | null = null;

  private destroy$ = new Subject<void>();

  constructor(private apiService: ApiService) { }

  ngOnInit() {
    this.startNewGame();
  }

  ngOnDestroy() {
    this.destroy$.next();
    this.destroy$.complete();
  }

  startNewGame() {
    this.score = 0;
    this.currentIndex = 0;
    this.gameFinished = false;
    this.loadQuestion();
  }

  loadQuestion() {
    this.loading = true;
    this.error = null;
    this.showResult = false;
    this.selectedIndex = null;
    this.question = null;

    // Timeout de sécurité côté composant (au cas où l'API est très lente)
    const safetyTimeout = setTimeout(() => {
      if (this.loading) {
        this.loading = false;
        this.error = 'تجاوزت مهلة الانتظار. الخادم لا يستجيب.';
      }
    }, 12000);

    this.apiService.getGameQuestion().pipe(
      timeout(10000),
      takeUntil(this.destroy$)
    ).subscribe({
      next: (res: GameQuestion) => {
        clearTimeout(safetyTimeout);
        this.loading = false;
        if (res.ok && res.options) {
          this.question = res;
        } else {
          this.error = 'فشل تحميل السؤال. الرجاء المحاولة مرة أخرى.';
        }
      },
      error: (err) => {
        clearTimeout(safetyTimeout);
        console.error(err);
        this.error = 'فشل تحميل السؤال. تحقق من اتصالك بالخادم.';
        this.loading = false;
      }
    });
  }

  selectOption(index: number) {
    if (this.showResult) return;

    this.selectedIndex = index;
    this.showResult = true;

    if (this.question && index === this.question.correct_index) {
      this.isCorrect = true;
      this.score++;
    } else {
      this.isCorrect = false;
    }
  }

  nextQuestion() {
    if (this.currentIndex < this.totalQuestions - 1) {
      this.currentIndex++;
      this.loadQuestion();
    } else {
      this.gameFinished = true;
    }
  }

  getButtonClass(index: number): string {
    if (!this.showResult) return '';

    if (index === this.question.correct_index) {
      return 'correct-btn'; // Vert
    }

    if (index === this.selectedIndex && !this.isCorrect) {
      return 'wrong-btn'; // Rouge
    }

    return '';
  }

  getScoreMessage(): string {
    const percentage = (this.score / this.totalQuestions) * 100;
    if (percentage === 100) return 'ممتاز! أحسنت!';
    if (percentage >= 80) return 'جيد جداً!';
    if (percentage >= 60) return 'جيد!';
    if (percentage >= 40) return 'يمكنك التحسن!';
    return 'حاول مرة أخرى!';
  }
}

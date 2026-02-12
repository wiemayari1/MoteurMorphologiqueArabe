import { Component, OnInit, OnDestroy } from '@angular/core';
import { CommonModule } from '@angular/common';
import { RouterModule } from '@angular/router';
import { MatCardModule } from '@angular/material/card';
import { MatButtonModule } from '@angular/material/button';
import { MatProgressBarModule } from '@angular/material/progress-bar';
import { MatIconModule } from '@angular/material/icon';
import { PageShellComponent } from '../../shared/page-shell/page-shell.component';
import { ApiService, GameQuestion } from '../../services/api.service';
import { Subject, takeUntil } from 'rxjs';

// Interface pour le frontend
interface QuestionDisplay {
  id: number;
  word: string;
  correctAnswer: string;
  options: string[];
  correctIndex: number;
  type: 'conjugation' | 'derivation';
}

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
  template: `
    <app-page-shell title="اللعبة">
      <div class="game-container">
        <!-- Stats -->
        <div class="game-stats" *ngIf="!loading && !error">
          <div class="score-badge">
            <span class="score-label">النقاط:</span>
            <span class="score-value">{{ score }} / {{ totalQuestions }}</span>
          </div>
          <mat-progress-bar 
            mode="determinate" 
            [value]="(currentQuestionIndex / totalQuestions) * 100"
            class="game-progress">
          </mat-progress-bar>
          <div class="question-counter">
            السؤال {{ currentQuestionIndex + 1 }} من {{ totalQuestions }}
          </div>
        </div>

        <!-- Loading -->
        <div class="loading-state" *ngIf="loading">
          <div class="spinner"></div>
          <p>جاري تحميل اللعبة...</p>
        </div>

        <!-- Error -->
        <div class="error-state" *ngIf="error">
          <mat-icon class="error-icon">error</mat-icon>
          <p>{{ error }}</p>
          <button mat-raised-button color="primary" (click)="restartGame()">
            إعادة المحاولة
          </button>
        </div>

        <!-- Game Content -->
        <mat-card class="game-card" *ngIf="!loading && !error && !gameFinished">
          <div class="question-section" *ngIf="currentQuestion">
            <h3 class="question-text">ما هو وزن كلمة "{{ currentQuestion.word }}"؟</h3>
            
            <div class="options-grid">
              <button 
                *ngFor="let option of currentQuestion.options; let i = index"
                mat-raised-button
                class="option-btn"
                [class.correct]="showResult && i === currentQuestion.correctIndex"
                [class.wrong]="showResult && selectedIndex === i && i !== currentQuestion.correctIndex"
                [disabled]="showResult"
                (click)="selectAnswer(i)">
                {{ option }}
              </button>
            </div>

            <div class="result-section" *ngIf="showResult">
              <div class="result-message" [class.success]="isCorrect" [class.failure]="!isCorrect">
                <mat-icon>{{ isCorrect ? 'check_circle' : 'cancel' }}</mat-icon>
                <span>{{ isCorrect ? 'إجابة صحيحة!' : 'إجابة خاطئة!' }}</span>
              </div>
              <button mat-raised-button color="primary" (click)="nextQuestion()">
                {{ isLastQuestion ? 'إنهاء اللعبة' : 'السؤال التالي' }}
              </button>
            </div>
          </div>
        </mat-card>

        <!-- Game Finished -->
        <mat-card class="game-card results-card" *ngIf="gameFinished">
          <div class="final-results">
            <h2>انتهت اللعبة!</h2>
            <div class="final-score">
              <span class="score-number">{{ score }}</span>
              <span class="score-total">/{{ totalQuestions }}</span>
            </div>
            <p class="score-message">{{ getScoreMessage() }}</p>
            <button mat-raised-button color="primary" (click)="restartGame()">
              <mat-icon>replay</mat-icon>
              لعب مرة أخرى
            </button>
          </div>
        </mat-card>
      </div>
    </app-page-shell>
  `,
  styleUrls: ['./game-page.component.scss']
})
export class GamePageComponent implements OnInit, OnDestroy {
  loading = true;
  error: string | null = null;
  questions: QuestionDisplay[] = [];
  currentQuestionIndex = 0;
  currentQuestion: QuestionDisplay | null = null;
  selectedIndex: number | null = null;
  showResult = false;
  isCorrect = false;
  score = 0;
  totalQuestions = 5;
  gameFinished = false;
  
  private destroy$ = new Subject<void>();

  constructor(private apiService: ApiService) {}

  ngOnInit() {
    this.loadGame();
  }

  ngOnDestroy() {
    this.destroy$.next();
    this.destroy$.complete();
  }

  loadGame() {
    this.loading = true;
    this.error = null;
    
    // Données mock
    const mockQuestions: QuestionDisplay[] = [
      { id: 1, word: 'كتب', correctAnswer: 'فاعل', options: ['فاعل', 'مفعول', 'فِعال', 'مفعّل'], correctIndex: 0, type: 'conjugation' },
      { id: 2, word: 'مكتوب', correctAnswer: 'مفعول', options: ['فاعل', 'مفعول', 'فِعال', 'مفعّل'], correctIndex: 1, type: 'conjugation' },
      { id: 3, word: 'مدرسة', correctAnswer: 'مفعّلة', options: ['فاعلة', 'مفعولة', 'مفعّلة', 'فَعِيل'], correctIndex: 2, type: 'conjugation' },
      { id: 4, word: 'كتاب', correctAnswer: 'فِعال', options: ['فاعل', 'مفعول', 'فِعال', 'مفعّل'], correctIndex: 2, type: 'conjugation' },
      { id: 5, word: 'معلم', correctAnswer: 'مفَعّل', options: ['فاعل', 'مفعول', 'فِعال', 'مفَعّل'], correctIndex: 3, type: 'conjugation' }
    ];

    // Appel API avec mapping
    this.apiService.getGameQuestion().pipe(
      takeUntil(this.destroy$)
    ).subscribe({
      next: (apiResponse: GameQuestion) => {
        if (apiResponse.ok && apiResponse.options) {
          // Transformer la réponse API
          this.questions = [{
            id: 1,
            word: apiResponse.root,
            correctAnswer: apiResponse.scheme,
            options: apiResponse.options,
            correctIndex: apiResponse.correct_index,
            type: 'conjugation'
          }];
          this.totalQuestions = this.questions.length;
          this.currentQuestion = this.questions[0];
        } else {
          this.questions = mockQuestions;
          this.totalQuestions = mockQuestions.length;
          this.currentQuestion = mockQuestions[0];
        }
        this.loading = false;
      },
      error: (err: Error) => {
        console.warn('API Error, using mock data:', err);
        this.questions = mockQuestions;
        this.totalQuestions = mockQuestions.length;
        this.currentQuestion = mockQuestions[0];
        this.loading = false;
      }
    });
  }

  selectAnswer(index: number) {
    if (this.showResult || !this.currentQuestion) return;
    
    this.selectedIndex = index;
    this.isCorrect = index === this.currentQuestion.correctIndex;
    this.showResult = true;
    
    if (this.isCorrect) {
      this.score++;
    }
  }

  nextQuestion() {
    if (this.currentQuestionIndex < this.questions.length - 1) {
      this.currentQuestionIndex++;
      this.currentQuestion = this.questions[this.currentQuestionIndex];
      this.selectedIndex = null;
      this.showResult = false;
    } else {
      this.gameFinished = true;
    }
  }

  restartGame() {
    this.currentQuestionIndex = 0;
    this.score = 0;
    this.selectedIndex = null;
    this.showResult = false;
    this.gameFinished = false;
    this.loadGame();
  }

  get isLastQuestion(): boolean {
    return this.currentQuestionIndex === this.questions.length - 1;
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

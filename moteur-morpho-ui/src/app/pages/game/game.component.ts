import { Component } from '@angular/core';
import { CommonModule } from '@angular/common';
import { FormsModule } from '@angular/forms';

import { MatCardModule } from '@angular/material/card';
import { MatFormFieldModule } from '@angular/material/form-field';
import { MatInputModule } from '@angular/material/input';
import { MatButtonModule } from '@angular/material/button';
import { MatIconModule } from '@angular/material/icon';

type Question = {
  root: string;
  scheme: string;
  word: string;
  ask: 'root' | 'scheme';
};

@Component({
  selector: 'app-game',
  standalone: true,
  imports: [
    CommonModule,
    FormsModule,
    MatCardModule,
    MatFormFieldModule,
    MatInputModule,
    MatButtonModule,
    MatIconModule,
  ],
  templateUrl: './game.component.html',
  styleUrls: ['./game.component.scss'],
})
export class GameComponent {
  roots = ['كتب', 'درس', 'دخل', 'خرج', 'علم'];
  schemes = ['فاعل', 'مفعول'];

  qIndex = 0;
  score = 0;
  total = 6;

  current: Question | null = null;
  answer = '';
  done = false;
  msg = '';

  start() {
    this.qIndex = 0;
    this.score = 0;
    this.done = false;
    this.msg = '';
    this.nextQuestion();
  }

  private generateWord(root: string, scheme: string): string {
    if (root === 'كتب' && scheme === 'مفعول') return 'مكتوب';
    if (root === 'كتب' && scheme === 'فاعل') return 'كاتب';
    return `${scheme}-${root}`; // demo
  }

  private nextQuestion() {
    if (this.qIndex >= this.total) {
      this.done = true;
      this.current = null;
      return;
    }

    const root = this.roots[Math.floor(Math.random() * this.roots.length)];
    const scheme = this.schemes[Math.floor(Math.random() * this.schemes.length)];
    const word = this.generateWord(root, scheme);
    const ask: 'root' | 'scheme' = Math.random() < 0.5 ? 'root' : 'scheme';

    this.current = { root, scheme, word, ask };
    this.answer = '';
    this.msg = '';
  }

  submit() {
    if (!this.current) return;

    const correct = this.current.ask === 'root'
      ? this.answer.trim() === this.current.root
      : this.answer.trim() === this.current.scheme;

    if (correct) {
      this.score++;
      this.msg = 'إجابة صحيحة';
    } else {
      const right = this.current.ask === 'root' ? this.current.root : this.current.scheme;
      this.msg = `إجابة خاطئة. الصحيح: ${right}`;
    }

    this.qIndex++;
    setTimeout(() => this.nextQuestion(), 500);
  }
}
